/*MIT License

Copyright (c) 2022 Shahryar Ahmad 

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/
#include "compiler.h"
#include "ast.h"
#include "builtinfunc.h"
#include "byte_src.h"
#include "convo.h"
#include "lntable.h"
#include "opcode.h"
#include "refgraph.h"
#include "utility.h"
#include "vm.h"
#include "zobject.h"
#include "zuko-src.h"
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>
#include <algorithm>
#include <queue>

using namespace std;

#define INSTRUCTION(x) program.push_back(x);bytes_done++;

extern unordered_map<string,BuiltinFunc> funcs;
extern bool REPL_MODE;
extern zclass* TypeError;
extern zclass* ValueError;
extern zclass* MathError; 
extern zclass* NameError;
extern zclass* IndexError;
extern zclass* ArgumentError;
extern zclass* FileIOError;
extern zclass* KeyError;
extern zclass* OverflowError;
extern zclass* FileOpenError;
extern zclass* FileSeekError; 
extern zclass* ImportError;
extern zclass* ThrowError;
extern zclass* MaxRecursionError;
extern zclass* AccessError;

void REPL();
void initFunctions();
void initMethods();

void extractSymRef(unordered_map<string,bool>& ref,refgraph* graph)//gives the names of symbols that are referenced and not deadcode
{
  std::queue<string> q;
  q.push(".main");
  //there is no incoming edge on .main
  //ref.emplace(".main",true);
  string curr;
  //printf("--begin BFS--\n");
  while(!q.empty())
  {
    curr = q.front();
  //  printf("%s\n",curr.c_str());
    q.pop();
    str_vector* adj = refgraph_getref(graph,curr.c_str());
    if(!adj)
        puts("adj is NULL");
    for(size_t i = 0;i<adj->size;i++)
    {
        const char* e = adj->arr[i];
  //    printf("  neighbour: %s\n",e.c_str());
      if(ref.find(e) == ref.end()) //node not already visited or processed
      {
        q.push(e);
        ref.emplace(e,true);
      }
    }
  }
  //printf("--end BFS--\n");
}
inline void addBytes(vector<uint8_t>& vec,int32_t x)
{
  size_t sz = vec.size();
  vec.resize(vec.size()+sizeof(int32_t));
  memcpy(&vec[sz],&x,sizeof(int32_t));
}
void Compiler::set_source(zuko_src* p,size_t root_idx)
{
    if(root_idx >= p->files.size)
    {
        fprintf(stderr,"Compiler: set_source() failed. REASON: root_idx out of range!");
        exit(1);
    }
    symRef.clear();
    extractSymRef(symRef,&p->ref_graph);
    num_of_constants = (int32_t*)&(p->num_of_constants);
    files = &p->files;
    sources = &p->sources;
    fileTOP = root_idx; // we start from the root file obviously
    line_num_table = &p->line_num_table;
    filename = p->files.arr[root_idx];
    if(p->num_of_constants > 0 && !REPL_MODE)
    {
        if(vm.constants)
            delete[] vm.constants;
        vm.constants = new zobject[p->num_of_constants];
        vm.total_constants = 0;
    }
    //init builtins
    initFunctions();
    initMethods();
    //reset all state
    inConstructor = false;
    inGen = false;
    inclass = false;
    infunc = false;
    if(!REPL_MODE)
    {
        bytecode.clear();
        globals.clear();
        locals.clear();
        className = "";
        classMemb.clear();
        line_num = 1;
        andJMPS.clear();
        orJMPS.clear();
        prefixes = {""};
        bytes_done = 0;
    }
}
void Compiler::compileError(string type,string msg)
{
    fprintf(stderr,"\nFile %s\n",filename.c_str());
    fprintf(stderr,"%s at line %zu\n",type.c_str(),line_num);
    int idx = str_vector_search(files,filename.c_str());
    const char* source_code = sources->arr[idx];
    size_t l = 1;
    string line = "";
    size_t k = 0;
    while(source_code[k]!=0 && l<=line_num)
    {
        if(source_code[k]=='\n')
            l+=1;
        else if(l==line_num)
            line+=source_code[k];
        k+=1;
    }
    fprintf(stderr,"%s\n",lstrip(line).c_str());
    fprintf(stderr,"%s\n",msg.c_str());
    if(REPL_MODE)
        REPL();
    exit(1);
}
int32_t Compiler::is_duplicate_constant(zobject x)
{
    for (int32_t k = 0; k < vm.total_constants; k += 1)
    {
        if (zobject_equals(vm.constants[k],x))
            return k;
    }
    return -1;
}
int32_t Compiler::add_to_vm_strings(const string& n)
{
    size_t i = 0;
    for(auto e: vm.strings)
    {
        if(strcmp(e.val,n.c_str()) == 0)
            return i;
        i++;
    }
    char* arr = new char[n.length()+1]; //will be freed by VM's destructor
    strcpy(arr,n.c_str());
    // the GC does not know about this memory
    // the string table won't be deallocated until exit, so no point in checking if
    // strings in it are reachable or not(during collect phase of GC)
    zstr str;
    str.len = n.length();
    str.val = arr;
    vm.strings.push_back(str);
    return (int32_t)vm.strings.size()-1;
}
void Compiler::add_lntable_entry(size_t opcodeIdx)
{
    byte_src tmp = {fileTOP,line_num};
    lntable_emplace(line_num_table,opcodeIdx,tmp);
}
int32_t Compiler::add_builtin_to_vm(const string& name)
{
    size_t index;
    auto fnAddr = funcs[name];
    for(index = 0;index < vm.builtin.size();index+=1)
    {
        if(vm.builtin[index]==fnAddr)
        return (size_t)index;
    }
    vm.builtin.push_back(funcs[name]);
    return (int32_t)vm.builtin.size()-1;
}
vector<uint8_t> Compiler::expr_bytecode(Node* ast)
{
    zobject reg;
    vector<uint8_t> bytes;
    if (ast->childs.size() == 0)
    {
        if (ast->type == NodeType::NUM)
        {
            const string& n = ast->val;
            if (isnum(n))
            {
                bytes.push_back(LOAD_INT32);
                addBytes(bytes,Int(n));
                bytes_done += 1 + sizeof(int32_t);
            }
            else if (isInt64(n))
            {
                reg.type = 'l';
                reg.l = toInt64(n);
                int32_t e = is_duplicate_constant(reg);
                if (e != -1)
                    foo = e;
                else
                {
                    foo = vm.total_constants;
                    reg.type = 'l';
                    reg.l = toInt64(n);
                    vm.constants[vm.total_constants++] = reg;
                }
                bytes.push_back(LOAD_CONST);
                addBytes(bytes,foo);
                bytes_done += 1 + sizeof(int32_t);
            }
            else
            {
                compileError("OverflowError", "Error integer "+n+" causes overflow!");
            }
            return bytes;
        }
        else if (ast->type == NodeType::FLOAT)
        {
            string n = ast->val;
            bool neg = false;
            if (n[0] == '-')
            {
                neg = true;
                n = n.substr(1);
            }
            /* while (n.length() > 0 && n[n.length() - 1] == '0')
            {
                n = n.substr(0, n.length() - 1);
            }*/
            if (neg)
                n = "-" + n;
            reg.type = 'f';
            reg.f = Float(n);
            int32_t e = is_duplicate_constant(reg);
            if (e != -1)
                foo = e;
            else
            {
                foo = vm.total_constants;

                vm.constants[vm.total_constants++] = reg;
            }
            bytes.push_back(LOAD_CONST);
            addBytes(bytes,foo);
            bytes_done += 1 + sizeof(int32_t);
            return bytes;
        }
        else if (ast->type ==  NodeType::BOOL_NODE)
        {
            const string& n = ast->val;
            bytes.push_back((n == "true") ? LOAD_TRUE : LOAD_FALSE);
            bytes_done+=1;
            return bytes;
        }
        else if (ast->type == NodeType::STR_NODE)
        {
            bytes.push_back(LOAD_STR);
            const string& n = ast->val;
            foo = add_to_vm_strings(n);
            addBytes(bytes,foo);
            bytes_done += 5;
            return bytes;
        }
        else if (ast->type == NodeType::NIL)
        {
            bytes.push_back(LOAD_NIL);
            bytes_done += 1;
            return bytes;
        }
        else if (ast->type == NodeType::ID_NODE)
        {
            const string& name = ast->val;
            bool isGlobal = false;
            bool isSelf = false;
            foo = resolve_name(name,isGlobal,true,&isSelf);
            if(isSelf)
            {
            add_lntable_entry(bytes_done);
            bytes.push_back(SELFMEMB);
            foo = add_to_vm_strings(name);
            addBytes(bytes,foo);
            bytes_done+=5;
            return bytes;
            }
            else if(!isGlobal)
            {
            bytes.push_back(LOAD_LOCAL);
            addBytes(bytes,foo);
            bytes_done+=5;
            return bytes;
            }
            else
            {
            bytes.push_back(LOAD_GLOBAL);
            addBytes(bytes,foo);
            bytes_done += 5;
            return bytes;
            }
        }
        else if (ast->type == NodeType::BYTE_NODE)
        {
            bytes.push_back(LOAD_BYTE);
            bytes.push_back(tobyte(ast->val));
            bytes_done+=2;
            return bytes;
        }

    }
    if (ast->type == NodeType::list)
    {
        
        for (size_t k = 0; k < ast->childs.size(); k += 1)
        {
            vector<uint8_t> elem = expr_bytecode(ast->childs[k]);
            bytes.insert(bytes.end(), elem.begin(), elem.end());
        }
        bytes.push_back(LOAD);
        bytes.push_back('j');
        foo = ast->childs.size();
        addBytes(bytes,foo);
        bytes_done += 2 + 4;
        return bytes;
    }
    if (ast->type == NodeType::dict)
    {
        
        for (size_t k = 0; k < ast->childs.size(); k += 1)
        {
            vector<uint8_t> elem = expr_bytecode(ast->childs[k]);
            bytes.insert(bytes.end(), elem.begin(), elem.end());
        }
        add_lntable_entry(bytes_done);//runtime errors can occur
        bytes.push_back(LOAD);
        bytes.push_back('a');
        foo = ast->childs.size() / 2;//number of key value pairs in dictionary
        addBytes(bytes,foo);
        bytes_done += 2 + 4;
        return bytes;
    }
    if (ast->type == NodeType::add)
    {
        if(ast->childs[0]->type == NodeType::ID_NODE && ast->childs[1]->type == NodeType::NUM && isnum(ast->childs[1]->val))
        {
            bool self1 = false;
            bool is_global1;
            
            int i = resolve_name(ast->childs[0]->val,is_global1,true,&self1);
            if(!self1)
            {
                bytes.push_back(LOADVAR_ADDINT32);
                bytes.push_back((is_global1)? 1 : 0);
                addBytes(bytes, i);
                addBytes(bytes, (int32_t)atoi(ast->childs[1]->val));
                bytes_done += 10;
                return bytes;
            }
            
        }
        vector<uint8_t> a = expr_bytecode(ast->childs[0]);
        bytes.insert(bytes.end(), a.begin(), a.end());
        vector<uint8_t> b = expr_bytecode(ast->childs[1]);
        bytes.insert(bytes.end(), b.begin(), b.end());
        bytes.push_back(ADD);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return bytes;
    }
    if (ast->type == NodeType::sub)
    {
        if(ast->childs[0]->type == NodeType::ID_NODE && ast->childs[1]->type == NodeType::NUM && isnum(ast->childs[1]->val))
        {
            bool self1 = false;
            bool is_global1;
            
            int i = resolve_name(ast->childs[0]->val,is_global1,true,&self1);
            if(!self1)
            {
                bytes.push_back(LOADVAR_SUBINT32);
                bytes.push_back((is_global1)? 1 : 0);
                addBytes(bytes, i);
                addBytes(bytes, (int32_t)atoi(ast->childs[1]->val));
                bytes_done += 10;
                return bytes;
            }
            
        }
        vector<uint8_t> a = expr_bytecode(ast->childs[0]);
        bytes.insert(bytes.end(), a.begin(), a.end());
        vector<uint8_t> b = expr_bytecode(ast->childs[1]);
        bytes.insert(bytes.end(), b.begin(), b.end());
        bytes.push_back(SUB);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
    
        return bytes;
    }
    if (ast->type == NodeType::div_node)
    {
        
        vector<uint8_t> a = expr_bytecode(ast->childs[0]);
        bytes.insert(bytes.end(), a.begin(), a.end());
        vector<uint8_t> b = expr_bytecode(ast->childs[1]);
        bytes.insert(bytes.end(), b.begin(), b.end());
        bytes.push_back(DIV);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return bytes;
    }
    if (ast->type == NodeType::mul)
    {
        
        vector<uint8_t> a = expr_bytecode(ast->childs[0]);
        bytes.insert(bytes.end(), a.begin(), a.end());
        vector<uint8_t> b = expr_bytecode(ast->childs[1]);
        bytes.insert(bytes.end(), b.begin(), b.end());
        bytes.push_back(MUL);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return bytes;
    }
    if (ast->type == NodeType::XOR_node)
    {
        
        vector<uint8_t> a = expr_bytecode(ast->childs[0]);
        bytes.insert(bytes.end(), a.begin(), a.end());
        vector<uint8_t> b = expr_bytecode(ast->childs[1]);
        bytes.insert(bytes.end(), b.begin(), b.end());
        bytes.push_back(XOR);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return bytes;
    }
    if (ast->type == NodeType::mod)
    {
        
        vector<uint8_t> a = expr_bytecode(ast->childs[0]);
        bytes.insert(bytes.end(), a.begin(), a.end());
        vector<uint8_t> b = expr_bytecode(ast->childs[1]);
        bytes.insert(bytes.end(), b.begin(), b.end());
        bytes.push_back(MOD);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return bytes;
    }
    if (ast->type == NodeType::lshift)
    {
        
        vector<uint8_t> a = expr_bytecode(ast->childs[0]);
        bytes.insert(bytes.end(), a.begin(), a.end());
        vector<uint8_t> b = expr_bytecode(ast->childs[1]);
        bytes.insert(bytes.end(), b.begin(), b.end());
        bytes.push_back(LSHIFT);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return bytes;
    }
    if (ast->type == NodeType::rshift)
    {
        
        vector<uint8_t> a = expr_bytecode(ast->childs[0]);
        bytes.insert(bytes.end(), a.begin(), a.end());
        vector<uint8_t> b = expr_bytecode(ast->childs[1]);
        bytes.insert(bytes.end(), b.begin(), b.end());
        bytes.push_back(RSHIFT);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return bytes;
    }
    if (ast->type == NodeType::bitwiseand)
    {
        
        vector<uint8_t> a = expr_bytecode(ast->childs[0]);
        bytes.insert(bytes.end(), a.begin(), a.end());
        vector<uint8_t> b = expr_bytecode(ast->childs[1]);
        bytes.insert(bytes.end(), b.begin(), b.end());
        bytes.push_back(BITWISE_AND);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return bytes;
    }
    if (ast->type == NodeType::bitwiseor)
    {
        
        vector<uint8_t> a = expr_bytecode(ast->childs[0]);
        bytes.insert(bytes.end(), a.begin(), a.end());
        vector<uint8_t> b = expr_bytecode(ast->childs[1]);
        bytes.insert(bytes.end(), b.begin(), b.end());
        bytes.push_back(BITWISE_OR);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return bytes;
    }
    if (ast->type == NodeType::memb)
    {
        
        if (ast->childs[1]->type == NodeType::call)
        {
            Node* callnode = ast->childs[1];

            vector<uint8_t> a = expr_bytecode(ast->childs[0]);
            bytes.insert(bytes.end(), a.begin(), a.end());
            Node* args = callnode->childs[2];
            for (size_t f = 0; f < args->childs.size(); f += 1)
            {
                vector<uint8_t> arg = expr_bytecode(args->childs[f]);
                bytes.insert(bytes.end(), arg.begin(), arg.end());
            }
            bytes.push_back(CALLMETHOD);
            add_lntable_entry(bytes_done);
            const string& memberName = callnode->childs[1]->val;
            foo = add_to_vm_strings(memberName);
            addBytes(bytes,foo);
            bytes.push_back(args->childs.size());
            
            bytes_done +=6;

            return bytes;
        }
        else
        { 

        vector<uint8_t> a = expr_bytecode(ast->childs[0]);
        bytes.insert(bytes.end(), a.begin(), a.end());
        add_lntable_entry(bytes_done);
        bytes.push_back(MEMB);
        if (ast->childs[1]->type != NodeType::ID_NODE)
            compileError("SyntaxError", "Invalid Syntax");
        const string& name = ast->childs[1]->val;
        foo = add_to_vm_strings(name);
        addBytes(bytes,foo);
        bytes_done +=5;
        return bytes;
        }
    }
    if (ast->type == NodeType::AND)
    {
        vector<uint8_t> a = expr_bytecode(ast->childs[0]);
        bytes.insert(bytes.end(), a.begin(), a.end());

        bytes.push_back(JMPIFFALSENOPOP);
        andJMPS.push_back(bytes_done);
        int32_t I = bytes.size();
        bytes.push_back(0);
        bytes.push_back(0);
        bytes.push_back(0);
        bytes.push_back(0);
        bytes_done+=5;
        vector<uint8_t> b = expr_bytecode(ast->childs[1]);
        bytes.insert(bytes.end(), b.begin(), b.end());

        foo = b.size();
        memcpy(&bytes[I],&foo,sizeof(int32_t));

        return bytes;
    }
    if (ast->type == NodeType::IS_node)
    {
        
        vector<uint8_t> a = expr_bytecode(ast->childs[0]);
        bytes.insert(bytes.end(), a.begin(), a.end());
        vector<uint8_t> b = expr_bytecode(ast->childs[1]);
        bytes.insert(bytes.end(), b.begin(), b.end());
        bytes.push_back(IS);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return bytes;
    }
    if (ast->type == NodeType::OR)
    {
        
        vector<uint8_t> a = expr_bytecode(ast->childs[0]);
        bytes.insert(bytes.end(), a.begin(), a.end());
        bytes.push_back(NOPOPJMPIF);
        orJMPS.push_back(bytes_done);
        int32_t I = bytes.size();
        bytes.push_back(0);
        bytes.push_back(0);
        bytes.push_back(0);
        bytes.push_back(0);
        bytes_done+=5;
        vector<uint8_t> b = expr_bytecode(ast->childs[1]);
        bytes.insert(bytes.end(), b.begin(), b.end());

        foo = b.size();
        memcpy(&bytes[I],&foo,sizeof(int32_t));
        return bytes;
    }
    if (ast->type == NodeType::lt)
    {
        
        vector<uint8_t> a = expr_bytecode(ast->childs[0]);
        bytes.insert(bytes.end(), a.begin(), a.end());
        vector<uint8_t> b = expr_bytecode(ast->childs[1]);
        bytes.insert(bytes.end(), b.begin(), b.end());
        bytes.push_back(SMALLERTHAN);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return bytes;
    }
    if (ast->type == NodeType::gt)
    {
        
        vector<uint8_t> a = expr_bytecode(ast->childs[0]);
        bytes.insert(bytes.end(), a.begin(), a.end());
        vector<uint8_t> b = expr_bytecode(ast->childs[1]);
        bytes.insert(bytes.end(), b.begin(), b.end());
        bytes.push_back(GREATERTHAN);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return bytes;
    }
    if (ast->type == NodeType::equal)
    {
        
        vector<uint8_t> a = expr_bytecode(ast->childs[0]);
        bytes.insert(bytes.end(), a.begin(), a.end());
        vector<uint8_t> b = expr_bytecode(ast->childs[1]);
        bytes.insert(bytes.end(), b.begin(), b.end());
        bytes.push_back(EQ);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return bytes;
    }
    if (ast->type == NodeType::noteq)
    {
        
        vector<uint8_t> a = expr_bytecode(ast->childs[0]);
        bytes.insert(bytes.end(), a.begin(), a.end());
        vector<uint8_t> b = expr_bytecode(ast->childs[1]);
        bytes.insert(bytes.end(), b.begin(), b.end());
        bytes.push_back(NOTEQ);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return bytes;
    }
    if (ast->type == NodeType::gte)
    {
        
        vector<uint8_t> a = expr_bytecode(ast->childs[0]);
        bytes.insert(bytes.end(), a.begin(), a.end());
        vector<uint8_t> b = expr_bytecode(ast->childs[1]);
        bytes.insert(bytes.end(), b.begin(), b.end());
        bytes.push_back(GROREQ);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return bytes;
    }
    if (ast->type == NodeType::lte)
    {
        
        vector<uint8_t> a = expr_bytecode(ast->childs[0]);
        bytes.insert(bytes.end(), a.begin(), a.end());
        vector<uint8_t> b = expr_bytecode(ast->childs[1]);
        bytes.insert(bytes.end(), b.begin(), b.end());
        bytes.push_back(SMOREQ);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return bytes;
    }
    if (ast->type == NodeType::neg)
    {
        
        vector<uint8_t> val = expr_bytecode(ast->childs[0]);
        bytes.insert(bytes.end(), val.begin(), val.end());
        bytes.push_back(NEG);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return bytes;
    }
    if (ast->type == NodeType::complement)
    {
        
        vector<uint8_t> val = expr_bytecode(ast->childs[0]);
        bytes.insert(bytes.end(), val.begin(), val.end());
        bytes.push_back(COMPLEMENT);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return bytes;
    }
    if (ast->type == NodeType::NOT_node)
    {
        
        vector<uint8_t> val = expr_bytecode(ast->childs[0]);
        bytes.insert(bytes.end(), val.begin(), val.end());
        bytes.push_back(NOT);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return bytes;
    }
    if (ast->type == NodeType::index_node)
    {
        int32_t idx1;
        int32_t idx2;

        if(ast->childs[0]->type == NodeType::ID_NODE && ast->childs[1]->type == NodeType::ID_NODE)
        {
            bool self1 = false;
            bool self2 = false;
            bool is_global1;
            bool is_global2;
            
            int i = resolve_name(ast->childs[0]->val,is_global1,true,&self1);
            int j = resolve_name(ast->childs[1]->val,is_global2,true,&self2);
            if(!self1 && !self2)
            {
                bytes.push_back(INDEX_FAST);
                bytes.push_back((is_global1)? 1 : 0);
                bytes.push_back((is_global2)? 1 : 0);
                addBytes(bytes, i);
                addBytes(bytes, j);
                bytes_done += 11;
                return bytes;
            }
            
        }
        vector<uint8_t> a = expr_bytecode(ast->childs[0]);
        bytes.insert(bytes.end(), a.begin(), a.end());
        vector<uint8_t> b = expr_bytecode(ast->childs[1]);
        bytes.insert(bytes.end(), b.begin(), b.end());
        bytes.push_back(INDEX);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return bytes;
    }
    if (ast->type == NodeType::call)
    {
        const string& name = ast->childs[1]->val;
        bool udf = false;
        if (funcs.find(name) == funcs.end())//check if it is builtin function
            udf = true;
        if (udf)
        {
            bool isGlobal = false;
            bool isSelf = false;
            int foo = resolve_name(name,isGlobal,true,&isSelf);
            if(isGlobal && compiled_functions.find(name)!=compiled_functions.end()) // Make direct call
            {
                auto p = compiled_functions[name];
                Node* fn = p.first;
                int32_t stack_idx = p.second;
                int32_t N = ast->childs[2]->childs.size(); //arguments given
                int32_t args_required = fn->childs[2]->childs.size();
                int32_t opt_args = 0;
                int32_t def_params = 0;
                int32_t def_begin = -1;
                for(size_t i=0;i<fn->childs[2]->childs.size();i++)
                {
                    Node* arg = fn->childs[2]->childs[i];
                    if(arg->childs.size()!=0)
                    {
                        def_params++;
                        if(def_begin==-1)
                            def_begin = i;
                    }
                }
                if ( (size_t)N + def_params < args_required || (size_t)N > args_required)
                {
                    compileError("ArgumentError", "Error function " + (string)name + " takes " + to_string(args_required) + " arguments," + to_string(N) + " given!");
                }
                //load all args
                for(size_t i=0;i<ast->childs[2]->childs.size();i++)
                {
                    Node* arg = ast->childs[2]->childs[i];
                    vector<uint8_t> tmp = expr_bytecode(arg);
                    bytes.insert(bytes.end(),tmp.begin(),tmp.end());
                }
                //load optional args
                for (size_t i = def_params - (args_required - N); i < def_params; i++)
                {
                    //printf("i = %zu\n",i);
                    Node* arg = fn->childs[2]->childs[def_begin+i]->childs[0];
                    vector<uint8_t> tmp = expr_bytecode(arg);
                    bytes.insert(bytes.end(),tmp.begin(),tmp.end());
                }
                bytes.push_back(CALL_DIRECT);
                addBytes(bytes, stack_idx);
                bytes_done+=5;
                return bytes;
            }
            if(isSelf)
            {
                bytes.push_back(LOAD_LOCAL);
                addBytes(bytes,0);//load self to pass as first argument
                bytes_done+=5;
            }
            for (size_t k = 0; k < ast->childs[2]->childs.size(); k += 1)
            {
                vector<uint8_t> val = expr_bytecode(ast->childs[2]->childs[k]);
                bytes.insert(bytes.end(), val.begin(), val.end());
            }
            if(isSelf)
            {
                add_lntable_entry(bytes_done);
                bytes.push_back(SELFMEMB);
                foo = add_to_vm_strings(name);
                addBytes(bytes,foo);
                bytes_done+=5;
                add_lntable_entry(bytes_done);
                bytes.push_back(CALLUDF);
                bytes.push_back(ast->childs[2]->childs.size()+1);
                bytes_done+=2;
                return bytes;
            }
            else if(isGlobal)
                bytes.push_back(LOAD_GLOBAL);
            else
                bytes.push_back(LOAD_LOCAL);
            addBytes(bytes,foo);
            bytes_done+=5;
            add_lntable_entry(bytes_done);
            bytes.push_back(CALLUDF);
            bytes.push_back(ast->childs[2]->childs.size());
            bytes_done+=2;
            return bytes;
        }
        else
        {
            for (size_t k = 0; k < ast->childs[2]->childs.size(); k += 1)
            {
                vector<uint8_t> val = expr_bytecode(ast->childs[2]->childs[k]);
                bytes.insert(bytes.end(), val.begin(), val.end());
            }
            add_lntable_entry(bytes_done);
            bytes.push_back(CALLFORVAL);
            bool add = true;
            size_t index = 0;
            for(index = 0;index < vm.builtin.size();index+=1)
            {
            if(vm.builtin[index]==funcs[name])
            {
                add = false;
                break;
            }
            }
            if(add)
            {
                vm.builtin.push_back(funcs[name]);
                foo = vm.builtin.size()-1;
            }
            else
                foo = index;
            addBytes(bytes,foo);
            bytes.push_back((char)ast->childs[2]->childs.size());
            bytes_done += 6;
            return bytes;
        }
    }
    if (ast->type == NodeType::YIELD_node)
    {
    if(inConstructor)
        compileError("SyntaxError","Error class constructor can not be generators!");
    
    vector<uint8_t> val = expr_bytecode(ast->childs[1]);
    bytes.insert(bytes.end(), val.begin(), val.end());              
    add_lntable_entry(bytes_done);
    bytes.push_back(YIELD_AND_EXPECTVAL);
    bytes_done += 1;
    return bytes;
    }  
    compileError("SyntaxError", "Invalid syntax in expression");
    exit(1);
    return bytes;//to prevent compiler warning
}
vector<string> Compiler::scan_class(Node* ast)
{
    vector<string> names;
    while(ast->type!=NodeType::EOP)
    {
        if(ast->type==NodeType::declare)
        {
        string n = ast->val;
        if(n[0]=='@')
            n = n.substr(1);
        if(std::find(names.begin(),names.end(),n)!=names.end() || std::find(names.begin(),names.end(),"@"+n)!=names.end())
        {
            line_num = atoi(ast->childs[0]->val);
            compileError("NameError","Error redeclaration of "+n+".");
        }
        names.push_back(ast->val);
        }
        else if(ast->type == NodeType::FUNC)
        {

        string n = ast->childs[1]->val;
        if(n[0]=='@')
            n = n.substr(1);
        if(std::find(names.begin(),names.end(),n)!=names.end() || std::find(names.begin(),names.end(),"@"+n)!=names.end())
        {
            line_num = atoi(ast->childs[0]->val);
            compileError("NameError","Error redeclaration of "+n+".");
        }
        names.push_back(ast->childs[1]->val);
        }
        else if(ast->type == NodeType::CORO) //generator function or coroutine
        {
            line_num = atoi(ast->childs[0]->val);
            compileError("NameError","Error coroutine inside class not allowed.");
        }
        
        else if(ast->type==NodeType::CLASS)
        {
        line_num = atoi(ast->childs[0]->val);
        compileError("SyntaxError","Error nested classes not supported");     
        }
        ast = ast->childs.back();
    }
    return names;
}
int32_t Compiler::resolve_name(string name,bool& isGlobal,bool blowUp,bool* isFromSelf)
{
    isGlobal = false;
    for(int32_t i=locals.size()-1;i>=0;i-=1)
    {
        if(locals[i].find(name)!=locals[i].end())
        return locals[i][name];
    }

    if(isFromSelf)
    {
        if(inclass && infunc && (std::find(classMemb.begin(),classMemb.end(),name)!=classMemb.end() || std::find(classMemb.begin(),classMemb.end(),"@"+name)!=classMemb.end() ))
        {
            *isFromSelf = true;
            return -2;
        }
    }
    for(int32_t i=prefixes.size()-1;i>=0;i--)
        {
        string prefix = prefixes[i];
        if(globals.find(prefix+name) != globals.end())
        {
            isGlobal = true;
            return globals[prefix+name];
        }
        }
    if(blowUp)
        compileError("NameError","Error name "+name+" is not defined!");
    return -1;
}
vector<uint8_t> Compiler::compile(Node* ast)
{
    vector<uint8_t> program;
    
    bool isGen = false;
    bool dfor = false;
    while (ast->type != NodeType::EOP)
    {
        if(ast->childs.size() >= 1)
        {
        if (ast->childs[0]->type == NodeType::line)
            line_num = Int(ast->childs[0]->val);
        }
        if (ast->type == NodeType::declare)
        {
            vector<uint8_t> val = expr_bytecode(ast->childs[1]);
            program.insert(program.end(), val.begin(), val.end());
            const string& name = ast->val;
            if (locals.size() == 0)
            {
                if (globals.find(name) != globals.end())
                    compileError("NameError", "Error redeclaration of variable " + name);
                foo = STACK_SIZE;
                if(!inclass)
                    globals.emplace(name,foo);
                STACK_SIZE+=1;
            }
            else
            {
                if(locals.back().find(name)!=locals.back().end())
                    compileError("NameError", "Error redeclaration of variable " + name);
                foo = STACK_SIZE;
                if(inclass && !infunc);
                else
                    locals.back().emplace(name,foo);
                STACK_SIZE+=1;
            }

        }
        else if (ast->type == NodeType::import || ast->type==NodeType::importas)
        {
            string name = ast->childs[1]->val;
            string vname = (ast->type==NodeType::importas) ? ast->childs[2]->val : name;
            bool f;
            if(resolve_name(vname,f,false)!=-1)
                compileError("NameError","Error redeclaration of name "+vname);
            add_lntable_entry(bytes_done);
            program.push_back(IMPORT);
            foo = add_to_vm_strings(name);
            addBytes(program,foo);
            bytes_done += 5;
            if(locals.size() == 0)
                globals.emplace(vname,STACK_SIZE);
            else
                locals.back().emplace(vname,STACK_SIZE);
            STACK_SIZE+=1;
        }
        else if (ast->type == NodeType::assign)
        {
            
            string name = ast->childs[1]->val;
            bool doit = true;
            if (ast->childs[1]->type == NodeType::ID_NODE)
            {
                if (ast->childs[2]->childs.size() == 2)
                {
                    if (ast->childs[2]->type == NodeType::add && ast->childs[2]->childs[0]->val == ast->childs[1]->val && ast->childs[2]->childs[0]->type==NodeType::ID_NODE && ast->childs[2]->childs[1]->type==NodeType::NUM && strcmp(ast->childs[2]->childs[1]->val,"1") == 0)
                    {
                    bool isGlobal = false;
                    bool isSelf = false;
                    int32_t idx = resolve_name(name,isGlobal,false,&isSelf);
                    if(idx==-1)
                        compileError("NameError","Error name "+name+" is not defined!");
                    if(isSelf)
                    {
                        vector<uint8_t> val = expr_bytecode(ast->childs[2]);
                        program.insert(program.end(), val.begin(), val.end());
                    }
                    add_lntable_entry(bytes_done);
                    foo = idx;
                    if(isSelf)
                    {
                        foo = add_to_vm_strings(name);
                        program.push_back(ASSIGNSELFMEMB);
                    }
                    else if(!isGlobal)
                        program.push_back(INPLACE_INC);
                    else
                        program.push_back(INC_GLOBAL);
                    addBytes(program,foo);
                    bytes_done+=5;
                    doit = false;
                    }
                }
                if (doit)
                {
                    bool isGlobal = false;
                    bool isSelf = false;
                    int32_t idx = resolve_name(name,isGlobal,false,&isSelf);
                    if(idx==-1)
                        compileError("NameError","Error name "+name+" is not defined!");
                    vector<uint8_t> val = expr_bytecode(ast->childs[2]);
                    program.insert(program.end(), val.begin(), val.end());
                    add_lntable_entry(bytes_done);
                    if(isSelf)
                    {
                    idx = add_to_vm_strings(name);
                    program.push_back(ASSIGNSELFMEMB);
                    }
                    else if(!isGlobal)
                    program.push_back(ASSIGN);
                    else
                    program.push_back(ASSIGN_GLOBAL);
                    addBytes(program,idx);
                    bytes_done += 5;
                }
            }
            else if (ast->childs[1]->type == NodeType::index_node)
            {
                //reassign index
                vector<uint8_t> M = expr_bytecode(ast->childs[1]->childs[0]);
                program.insert(program.end(),M.begin(),M.end());
                M = expr_bytecode(ast->childs[2]);
                program.insert(program.end(),M.begin(),M.end());
                M = expr_bytecode(ast->childs[1]->childs[1]);
                program.insert(program.end(),M.begin(),M.end());
                program.push_back(ASSIGNINDEX);
                add_lntable_entry(bytes_done);
                bytes_done+=1;
            }
            else if (ast->childs[1]->type == NodeType::memb)
            {
                //reassign object member
                vector<uint8_t> lhs = expr_bytecode(ast->childs[1]->childs[0]);
                program.insert(program.end(),lhs.begin(),lhs.end());
                if(ast->childs[1]->childs[1]->type!=NodeType::ID_NODE)
                    compileError("SyntaxError","Invalid Syntax");
                string mname = ast->childs[1]->childs[1]->val;
                vector<uint8_t> val = expr_bytecode(ast->childs[2]);
                program.insert(program.end(), val.begin(), val.end());
                program.push_back(ASSIGNMEMB);
                foo = add_to_vm_strings(mname);
                addBytes(program,foo);
                add_lntable_entry(bytes_done);
                bytes_done+=5;

            }
        }
        else if (ast->type == NodeType::memb)
        {
            Node* bitch = new_node(NodeType::memb,".");
            bitch->childs.push_back(ast->childs[1]);
            bitch->childs.push_back(ast->childs[2]);
            vector<uint8_t> stmt = expr_bytecode(bitch);
            delete bitch;
            program.insert(program.end(),stmt.begin(),stmt.end());
            program.push_back(POP_STACK);
            bytes_done +=1;
        }
        else if (ast->type == NodeType::WHILE || ast->type == NodeType::DOWHILE)
        {
            if(ast->type==NodeType::DOWHILE)
              bytes_done+=5;//do while uses one extra jump before the condition
            //to skip the condition first time
            size_t L = bytes_done;
            vector<uint8_t> cond = expr_bytecode(ast->childs[1]);
            bytes_done += 1 + JUMPOFFSET_SIZE;//JMPFALSE <4 byte where>
            int32_t before = STACK_SIZE;
            SymbolTable m;
            locals.push_back(m);
            vector<int32_t> breakIdxCopy = breakIdx;
            vector<int32_t> contIdxCopy = contIdx;
            int32_t localsBeginCopy = localsBegin; //idx of vector locals, from where
            //the locals of this loop begin
            localsBegin = locals.size() - 1;
            vector<uint8_t> block = compile(ast->childs[2]);
            localsBegin = localsBeginCopy; //backtrack
            STACK_SIZE = before;


            int32_t whileLocals = locals.back().size();
            locals.pop_back();
            int32_t where = block.size() + 1 + JUMPOFFSET_SIZE;
            if(whileLocals!=0)
                where+=4;//4 for NPOP_STACK
            if(ast->type==NodeType::DOWHILE)
            {
                program.push_back(JMP);
                foo = cond.size()+ 5;
                addBytes(program,foo);
            }
            program.insert(program.end(), cond.begin(), cond.end());
            //Check while condition
            program.push_back(JMPIFFALSE);
            foo = where;
            addBytes(program,foo);

            //
            program.insert(program.end(), block.begin(), block.end());
            if(whileLocals!=0)
            {
                program.push_back(GOTONPSTACK);
                foo = whileLocals;
                addBytes(program,foo);
                bytes_done += 4;
            }
            else
                program.push_back(GOTO);
            foo = L;
            addBytes(program,foo);
            bytes_done += 1 + JUMPOFFSET_SIZE;
            // backpatch break and continue offsets
            int32_t a = (int)bytes_done;
            int32_t b = (int)L;
            
            for(auto e: breakIdx)
            {
                backpatches.push_back(Pair(e,a));  
            }
            for(auto e: contIdx)
            {
                backpatches.push_back(Pair(e,b));
            }
            //backtrack
            breakIdx = breakIdxCopy;
            contIdx  = contIdxCopy;
        }
        else if((ast->type == NodeType::FOR) || (dfor = (ast->type == NodeType::DFOR)))
        {
            bool decl_variable = (ast->childs[1]->type == NodeType::decl);
            size_t linenum_copy = line_num;
            const string& loop_var_name = ast->childs[2]->val;
            int32_t lcv_idx;
            int32_t end_idx;
            int32_t step_idx;
            bool lcv_is_global = false;
            int32_t stack_before = STACK_SIZE;
            // bytecode to initialize loop variable
            if(decl_variable)
            {
                vector<uint8_t> init_expr = expr_bytecode(ast->childs[3]);
                program.insert(program.end(),init_expr.begin(),init_expr.end()); //will load initial value onto stack
                lcv_idx = STACK_SIZE;
                STACK_SIZE++; //this initial value becomes a local variable
            } 
            else
            {
                bool is_self = false;
                int tmp = resolve_name(loop_var_name,lcv_is_global,true,&is_self);
                if(is_self)
                    compileError("SyntaxError","for loop control variable can't be from 'self'");
                lcv_idx = tmp;
                //assign initial value
                vector<uint8_t> init_val = expr_bytecode(ast->childs[3]);
                program.insert(program.end(),init_val.begin(),init_val.end());
                if(lcv_is_global)
                    program.push_back(ASSIGN_GLOBAL);
                else
                    program.push_back(ASSIGN);
                addBytes(program,lcv_idx);
                bytes_done+=5;
            }
            //load end value
            vector<uint8_t> end_value = expr_bytecode(ast->childs[4]);
            program.insert(program.end(),end_value.begin(),end_value.end());
            end_idx = STACK_SIZE++;
            //load step size
            vector<uint8_t> step_value = expr_bytecode(ast->childs[5]); // not step sister you dirty mind
            program.insert(program.end(),step_value.begin(),step_value.end()); 
            step_idx = STACK_SIZE++;
            
            // bytecode to skip loop body if condition is not met
            line_num = linenum_copy;
            add_lntable_entry(bytes_done);
            if(dfor)
                program.push_back(SETUP_DLOOP);
            else
                program.push_back(SETUP_LOOP);
            bytes_done+=1;
            if(lcv_is_global)
            {
                program.push_back(1);
                addBytes(program,lcv_idx);
                addBytes(program, end_idx);
                bytes_done+=9;
            }
            else
            {
                program.push_back(0);
                addBytes(program,lcv_idx);
                bytes_done+=5;
            }
            int32_t skip_loop = bytes_done;
            addBytes(program,0); // apply backpatch here
            bytes_done += 4;

            // add loop body
            SymbolTable m;
            if(decl_variable)
                m.emplace(loop_var_name,lcv_idx);
            localsBegin = locals.size();
            locals.push_back(m);
            size_t stacksize_before_body = STACK_SIZE;
            size_t loop_start = bytes_done;
            bool infor_copy = infor;
            vector<int32_t> breakIdxCopy = breakIdx;
            vector<int32_t> contIdxCopy = contIdx;
            breakIdx.clear();
            contIdx.clear();
            infor = true;
            vector<uint8_t> loop_body = compile(ast->childs[6]);
            infor = infor_copy;
            program.insert(program.end(),loop_body.begin(),loop_body.end());
            locals.pop_back();
            int body_locals = STACK_SIZE - stacksize_before_body;
            STACK_SIZE = stacksize_before_body;
            /*if(body_locals == 1)
            {
                program.push_back(POP_STACK);
                bytes_done++;
            }
            else if(body_locals > 1)
            {
                program.push_back(NPOP_STACK);
                addBytes(program,body_locals);
                bytes_done+=5;
            }*/
            int32_t cont_dest = (int32_t)bytes_done;
            // finally add LOOP instruction
            line_num = linenum_copy;
            add_lntable_entry(bytes_done);
            if(dfor)
                program.push_back(DLOOP);
            else
                program.push_back(LOOP);
            if(lcv_is_global)
                program.push_back(1);
            else
                program.push_back(0);
            bytes_done+=1;
            addBytes(program, lcv_idx); //can be global or local
            if(lcv_is_global)
            {
                addBytes(program, end_idx); // is a local ( see above we reserved stack space for it)
                bytes_done+=4;
            }
            addBytes(program, (int32_t)loop_start);
            bytes_done += 9;//10;//18;
            
            //cleanup the locals we created
            int32_t break_dest = (int32_t)bytes_done;
            program.push_back(NPOP_STACK);
            if(decl_variable)
                addBytes(program,3);
            else
                addBytes(program,2);
            bytes_done += 5;
            for(auto e: breakIdx)
            {
                backpatches.push_back(Pair(e,break_dest));
            }
            for(auto e: contIdx)
            {
                backpatches.push_back(Pair(e,cont_dest));
            }
            backpatches.push_back(Pair(skip_loop,(int32_t)break_dest));
            breakIdx = breakIdxCopy;
            contIdx = contIdxCopy;
            STACK_SIZE = stack_before;
        }
        else if (ast->type == NodeType::FOREACH)
        {
            const string& loop_var_name = ast->childs[1]->val;
            vector<uint8_t> startIdx = expr_bytecode(ast->childs[3]);
            program.insert(program.end(),startIdx.begin(),startIdx.end());
            int32_t E = STACK_SIZE;
            STACK_SIZE+=1;

            SymbolTable m;
            int32_t fnIdx = add_builtin_to_vm("len");

            //Bytecode to calculate length of list we are looping
            vector<uint8_t> LIST = expr_bytecode(ast->childs[2]);
            program.insert(program.end(),LIST.begin(),LIST.end());
            int32_t ListStackIdx = STACK_SIZE;
            STACK_SIZE+=1;
            foo = E;
            program.push_back(INPLACE_INC);
            addBytes(program,foo);
            int32_t cont = bytes_done;
            bytes_done+=5;

            int32_t before = STACK_SIZE;
            locals.push_back(m);
            locals.back().emplace(loop_var_name,STACK_SIZE);
            STACK_SIZE+=1;
            size_t L = bytes_done;
            add_lntable_entry(L+12);
            bytes_done+=20+6+1+JUMPOFFSET_SIZE+6;
            vector<int32_t> breakIdxCopy = breakIdx;
            vector<int32_t> contIdxCopy = contIdx;
            int32_t localsBeginCopy = localsBegin; //idx of vector locals, from where
            //the locals of this loop begin
            localsBegin = locals.size() - 1;
            vector<uint8_t> block = compile(ast->childs[4]);
            localsBegin = localsBeginCopy;
            program.push_back(LOAD);
            program.push_back('v');
            foo = E;
            addBytes(program,foo);

            //
            program.push_back(LOAD);
            program.push_back('v');
            foo = ListStackIdx;
            addBytes(program,foo);
            //

            program.push_back(CALLFORVAL);
            foo = fnIdx;
            addBytes(program,foo);
            program.push_back(1);
            program.push_back(SMALLERTHAN);
            program.push_back(JMPIFFALSE);
            int32_t where = 7+block.size()+1+JUMPOFFSET_SIZE+5+6;
            int32_t whileLocals = locals.back().size();
            if(whileLocals!=0)
                where+=4;
            foo = where;
            addBytes(program,foo);


            foo = ListStackIdx;
            program.push_back(LOAD);
            program.push_back('v');
            addBytes(program,foo);
            program.push_back(LOAD);
            program.push_back('v');
            foo = E;
            addBytes(program,foo);
            program.push_back(INDEX);
            program.insert(program.end(),block.begin(),block.end());
            program.push_back(INPLACE_INC);
            foo = E;
            addBytes(program,foo);
            bytes_done+=5;
            STACK_SIZE = before-2;

            locals.pop_back();
            if(whileLocals!=0)
            {
                program.push_back(GOTONPSTACK);
                foo = whileLocals;
                addBytes(program,foo);
                bytes_done += 4;
            }
            else
                program.push_back(GOTO);
            foo = L;
            addBytes(program,foo);

            program.push_back(NPOP_STACK);
            foo = 2;
            addBytes(program,foo);
            bytes_done += 1 + JUMPOFFSET_SIZE+JUMPOFFSET_SIZE+1;
                        
            //backpatching             
            int32_t brk = bytes_done-5; 
            for(auto e: breakIdx)
            {
            backpatches.push_back(Pair(e,brk));
            }
            for(auto e: contIdx)
            {
            backpatches.push_back(Pair(e,cont));
            }
            //backtrack
            breakIdx = breakIdxCopy;
            contIdx  = contIdxCopy;
        }
        else if(ast->type == NodeType::NAMESPACE)
        {
        string name = ast->childs[1]->val;
        string prefix;
        for(auto e: prefixes)
            prefix+=e;
        prefixes.push_back(prefix+name+"::");
        vector<uint8_t> block = compile(ast->childs[2]);
        prefixes.pop_back();
        program.insert(program.end(),block.begin(),block.end());
        //Hasta La Vista Baby
        }
        else if (ast->type == NodeType::IF)
        {
            int32_t jmp_offset;
            if(ast->childs[1]->childs[0]->type == NodeType::equal)
            {
                Node* cond = ast->childs[1]->childs[0];
                vector<uint8_t> lhs = expr_bytecode(cond->childs[0]);
                vector<uint8_t> rhs = expr_bytecode(cond->childs[1]);
                program.insert(program.end(),lhs.begin(),lhs.end());
                program.insert(program.end(),rhs.begin(),rhs.end());
                program.push_back(CMP_JMPIFFALSE);
                bytes_done+=1;
                program.push_back(0);
                jmp_offset = ++bytes_done;
                addBytes(program, 0);
                bytes_done += 4;
                
            }
            else
            {
                vector<uint8_t> cond = expr_bytecode(ast->childs[1]->childs[0]);
                program.insert(program.end(),cond.begin(),cond.end());
                program.push_back(JMPIFFALSE);
                jmp_offset = ++bytes_done;
                addBytes(program,0);
                bytes_done += 4;
            }

            int32_t before = STACK_SIZE;
            SymbolTable m;
            locals.push_back(m);
            vector<uint8_t> block = compile(ast->childs[2]);
            STACK_SIZE = before;
            bool hasLocals = (locals.back().size()!=0);

            
            foo = block.size();//
            if(hasLocals)
                foo+=5;
            //addBytes(program,foo);
            backpatches.push_back(Pair(jmp_offset,foo));

            program.insert(program.end(), block.begin(), block.end());
            if(hasLocals)
            {
                program.push_back(NPOP_STACK);
                foo = locals.back().size();
                addBytes(program,foo);
                bytes_done += 5;
            }
            locals.pop_back();
        }
        else if (ast->type == NodeType::IFELSE)
        {
            
            vector<uint8_t> ifcond = expr_bytecode(ast->childs[1]->childs[0]);
            bytes_done += 1 + JUMPOFFSET_SIZE;
            int32_t before = STACK_SIZE;
            SymbolTable m;
            locals.push_back(m);
            vector<uint8_t> ifblock = compile(ast->childs[2]);
            STACK_SIZE = before;
            //
            int32_t iflocals = locals.back().size();
            locals.pop_back();
            program.insert(program.end(), ifcond.begin(), ifcond.end());
            program.push_back(JMPIFFALSE);
            foo = ifblock.size() + 1 + JUMPOFFSET_SIZE;

            if(iflocals!=0)
            {
                foo+=4;
                bytes_done+=4;
            }
            addBytes(program,foo);
            program.insert(program.end(), ifblock.begin(), ifblock.end());


            bytes_done += 1 + JUMPOFFSET_SIZE;
            before = STACK_SIZE;
            locals.push_back(m);
            vector<uint8_t> elseblock = compile(ast->childs[3]);
            STACK_SIZE = before;
            int32_t elseLocals = locals.back().size();
            locals.pop_back();
            if(iflocals!=0)
            {
                program.push_back(JMPNPOPSTACK);
                foo = iflocals;
                addBytes(program,foo);
            }
            else
                program.push_back(JMP);
            foo = elseblock.size();
            if(elseLocals!=0)
                foo+=5;
            addBytes(program,foo);
            program.insert(program.end(), elseblock.begin(), elseblock.end());
            if(elseLocals!=0)
            {
            program.push_back(NPOP_STACK);
            foo = elseLocals;
            addBytes(program,foo);
            bytes_done += 5;
            }
        }
        else if (ast->type == NodeType::IFELIFELSE)
        {
            
            vector<uint8_t> ifcond = expr_bytecode(ast->childs[1]->childs[0]);
            bytes_done += 1 + JUMPOFFSET_SIZE;//JumpIfFalse after if condition
            SymbolTable m;
            locals.push_back(m);
            int32_t before = STACK_SIZE;
            vector<uint8_t> ifblock = compile(ast->childs[2]);
            STACK_SIZE = before;
            int32_t iflocals = locals.back().size();
            locals.pop_back();
            bytes_done += 1 + JUMPOFFSET_SIZE;//JMP of if block
            if(iflocals!=0)
                bytes_done+=4;//for NPOP_STACK+ 4 byte operand
            vector<vector<uint8_t>> elifConditions;
            int32_t elifBlocksSize = 0;//total size in bytes elif blocks take including their conditions
            int32_t elifBlockcounter = 3;//third node of ast
            vector<int32_t> elifLocalSizes ;
            vector<vector<uint8_t>> elifBlocks;
            for (size_t k = 1; k < ast->childs[1]->childs.size(); k += 1)
            {
                vector<uint8_t> elifCond = expr_bytecode(ast->childs[1]->childs[k]);
                elifConditions.push_back(elifCond);
                bytes_done += 1 + JUMPOFFSET_SIZE;//JMPIFFALSE of elif
                before = STACK_SIZE;
                locals.push_back(m);
                vector<uint8_t> elifBlock = compile(ast->childs[elifBlockcounter]);
                STACK_SIZE = before;
                int32_t eliflocals = locals.back().size();
                elifLocalSizes.push_back(eliflocals);
                locals.pop_back();
                bytes_done += 1 + JUMPOFFSET_SIZE;// JMP of elifBlock
                elifBlocks.push_back(elifBlock);
                elifBlocksSize += elifCond.size() + 1 + JUMPOFFSET_SIZE + elifBlock.size() + 1 + JUMPOFFSET_SIZE ;
                if(eliflocals!=0)
                {
                elifBlocksSize+=4;
                bytes_done+=4;
                }
                elifBlockcounter += 1;
            }
            locals.push_back(m);
            before = STACK_SIZE;
            vector<uint8_t> elseBlock = compile(ast->childs[ast->childs.size() - 2]);
            STACK_SIZE = before;
            int32_t elseLocals = locals.back().size();
            locals.pop_back();
            program.insert(program.end(), ifcond.begin(), ifcond.end());
            program.push_back(JMPIFFALSE);
            foo = ifblock.size() + 1 + JUMPOFFSET_SIZE ;
            if(iflocals!=0)
                foo+=4;
            addBytes(program,foo);
            program.insert(program.end(), ifblock.begin(), ifblock.end());
            if(iflocals!=0)
            {
                program.push_back(JMPNPOPSTACK);
                foo = iflocals;
                addBytes(program,foo);
            }
            else
                program.push_back(JMP);
            foo = elifBlocksSize + elseBlock.size() ;
            if(elseLocals!=0)
                foo+=5;
            addBytes(program,foo);  
            for (size_t k = 0; k < elifBlocks.size(); k += 1)
            {

                elifBlocksSize -= elifBlocks[k].size() + elifConditions[k].size() + 1 + (2 * JUMPOFFSET_SIZE) + 1;
                if(elifLocalSizes[k]!=0)
                    elifBlocksSize -= 4;
                program.insert(program.end(), elifConditions[k].begin(), elifConditions[k].end());
                program.push_back(JMPIFFALSE);
                foo = elifBlocks[k].size() + JUMPOFFSET_SIZE + 1;
                if(elifLocalSizes[k]!=0)
                    foo+=4;
                addBytes(program,foo);
                program.insert(program.end(), elifBlocks[k].begin(), elifBlocks[k].end());
                if(elifLocalSizes[k]!=0)
                {
                    program.push_back(JMPNPOPSTACK);
                    foo = elifLocalSizes[k];
                    addBytes(program,foo);
                }
                else
                    program.push_back(JMP);
                foo = elifBlocksSize + elseBlock.size() ;
                if(elseLocals!=0)
                    foo+=5;
                addBytes(program,foo);
            }

            program.insert(program.end(), elseBlock.begin(), elseBlock.end());
            if(elseLocals!=0)
            {
                program.push_back(NPOP_STACK);
                foo = elseLocals;
                addBytes(program,foo);
                bytes_done+=5;
            }
        }
        else if (ast->type == NodeType::IFELIF)
        {           
            vector<uint8_t> ifcond = expr_bytecode(ast->childs[1]->childs[0]);
            bytes_done += 1 + JUMPOFFSET_SIZE;//JumpIfFalse after if condition
            
            SymbolTable m;
            locals.push_back(m);
            int32_t before = STACK_SIZE;
            vector<uint8_t> ifblock = compile(ast->childs[2]);
            
            STACK_SIZE = before;
            int32_t iflocals = locals.back().size();
            locals.pop_back();
            bytes_done += 1 + JUMPOFFSET_SIZE;//JMP of if block
            if(iflocals!=0)
                bytes_done+=4;//for NPOP_STACK+ 4 byte operand
            vector<vector<uint8_t>> elifConditions;
            int32_t elifBlocksSize = 0;//total size in bytes elif blocks take including their conditions
            int32_t elifBlockcounter = 3;//third node of ast
            vector<int32_t> elifLocalSizes ;
            vector<vector<uint8_t>> elifBlocks;
            for (size_t k = 1; k < ast->childs[1]->childs.size(); k += 1)
            {
                vector<uint8_t> elifCond = expr_bytecode(ast->childs[1]->childs[k]);
                elifConditions.push_back(elifCond);
                bytes_done += 1 + JUMPOFFSET_SIZE;//JMPIFFALSE of elif
                
                before = STACK_SIZE;
                locals.push_back(m);
                vector<uint8_t> elifBlock = compile(ast->childs[elifBlockcounter]);
                
                STACK_SIZE = before;
                int32_t eliflocals = locals.back().size();
                elifLocalSizes.push_back(eliflocals);
                locals.pop_back();
                bytes_done += 1 + JUMPOFFSET_SIZE;// JMP of elifBlock
                elifBlocks.push_back(elifBlock);
                elifBlocksSize += elifCond.size() + 1 + JUMPOFFSET_SIZE + elifBlock.size() + 1 + JUMPOFFSET_SIZE ;
                if(eliflocals!=0)
                {
                    elifBlocksSize+=4;
                    bytes_done+=4;
                }
                elifBlockcounter += 1;
        }

        program.insert(program.end(), ifcond.begin(), ifcond.end());
        program.push_back(JMPIFFALSE);
        foo = ifblock.size() + 1 + JUMPOFFSET_SIZE ;
        if(iflocals!=0)
            foo+=4;
        addBytes(program,foo);
        program.insert(program.end(), ifblock.begin(), ifblock.end());
        if(iflocals!=0)
        {
            program.push_back(JMPNPOPSTACK);
            foo = iflocals;
            addBytes(program,foo);
        }
        else
            program.push_back(JMP);
        foo = elifBlocksSize;
        addBytes(program,foo);
        for (size_t k = 0; k < elifBlocks.size(); k += 1)
        {
            elifBlocksSize -= elifBlocks[k].size() + elifConditions[k].size() + 1 + (2 * JUMPOFFSET_SIZE) + 1;
            if(elifLocalSizes[k]!=0)
                elifBlocksSize -= 4;
            program.insert(program.end(), elifConditions[k].begin(), elifConditions[k].end());
            program.push_back(JMPIFFALSE);
            foo = elifBlocks[k].size() + JUMPOFFSET_SIZE + 1;
            if(elifLocalSizes[k]!=0)
                foo+=4;
            addBytes(program,foo);
            program.insert(program.end(), elifBlocks[k].begin(), elifBlocks[k].end());
            if(elifLocalSizes[k]!=0)
            {

                program.push_back(JMPNPOPSTACK);
                foo = elifLocalSizes[k];
                addBytes(program,foo);
            }
            else
                program.push_back(JMP);
            foo = elifBlocksSize ;
            addBytes(program,foo);
        }


        }         
        else if(ast->type==NodeType::TRYCATCH)
        {
        program.push_back(ONERR_GOTO);
        bytes_done+=5;
        
        int32_t before = STACK_SIZE;
        SymbolTable m;
        locals.push_back(m);
        vector<uint8_t> tryBlock = compile(ast->childs[2]);
        
        STACK_SIZE = before;
        int32_t trylocals = locals.back().size();
        locals.pop_back();

        foo = bytes_done+6 +(trylocals==0 ? 0:5);
        addBytes(program,foo);
        program.insert(program.end(),tryBlock.begin(),tryBlock.end());
        if(trylocals!=0)
        {
            foo = trylocals;
            program.push_back(NPOP_STACK);
            addBytes(program,foo);
            bytes_done+=5;

        }
        program.push_back(POP_EXCEP_TARG);
        bytes_done+=6;
        
        before = STACK_SIZE;
        m.emplace(ast->childs[1]->val,STACK_SIZE);
        STACK_SIZE+=1;
        locals.push_back(m);
        vector<uint8_t> catchBlock = compile(ast->childs[3]);
        int32_t catchlocals = locals.back().size();
        
        foo = catchBlock.size()+((catchlocals==1) ? 1 : 5);
        program.push_back(JMP);
        addBytes(program,foo);
        program.insert(program.end(),catchBlock.begin(),catchBlock.end());
        STACK_SIZE = before;

        if(catchlocals>1)
        {
            foo = catchlocals;
            program.push_back(NPOP_STACK);
            addBytes(program,foo);
            bytes_done+=5;
        }
        else
        {
            program.push_back(POP_STACK);
            bytes_done+=1;
        }
        locals.pop_back();

        }
        else if (ast->type==NodeType::FUNC || (isGen = ast->type == NodeType::CORO))
        {
            int32_t selfIdx= 0;
            if(infunc)
            {
                line_num = atoi(ast->childs[0]->val);
                compileError("SyntaxError","Error function within function not allowed!");
            }
            const string& name = ast->childs[1]->val;
            auto it = symRef.find(name);
            bool isRef = it!=symRef.end() && (*it).second;
            if(compileAllFuncs || isRef || inclass)
            {
                if(funcs.find(name)!=funcs.end() && !inclass)
                    compileError("NameError","Error a builtin function with same name already exists!");
                if(locals.size()==0 && globals.find(name)!=globals.end())
                    compileError("NameError","Error redeclaration of name "+name);
                else if(locals.size()!=0 && locals.back().find(name)!=locals.back().end())
                    compileError("NameError","Error redeclaration of name "+name);
                if(!inclass)
                {
                    if(locals.size()==0)
                    {
                        globals.emplace(name,STACK_SIZE);
                        if(!isGen)
                            compiled_functions.emplace(name,std::pair<Node*,int32_t>(ast,STACK_SIZE));
                    }
                    else
                        locals.back().emplace(name,STACK_SIZE);
                    STACK_SIZE+=1;
                }
                unordered_map<string,int32_t> C;
                uint8_t def_param =0;
                vector<uint8_t> expr;
                int32_t before = STACK_SIZE;
                STACK_SIZE = 0;
                if(inclass)
                {
                    locals.back().emplace("self",STACK_SIZE);
                    selfIdx = STACK_SIZE;
                    if(name=="__construct__")
                        inConstructor = true;
                    STACK_SIZE+=1;
                }
                for (size_t k =0; k<ast->childs[2]->childs.size(); k += 1)
                {
                    string n = ast->childs[2]->childs[k]->val;
                    if(ast->childs[2]->childs[k]->childs.size()!=0)
                    {
                        if(isGen)
                            compileError("SyntaxError","Error default parameters not supported for couroutines");
                        expr = expr_bytecode(ast->childs[2]->childs[k]->childs[0]);
                        program.insert(program.end(),expr.begin(),expr.end());
                        def_param +=1;
                    }
                    C.emplace(n,STACK_SIZE);
                    STACK_SIZE+=1;
                }
                if(isGen)
                    program.push_back(LOAD_CO);
                else
                    program.push_back(LOAD_FUNC);
                foo = bytes_done+2+JUMPOFFSET_SIZE+JUMPOFFSET_SIZE+JUMPOFFSET_SIZE+1 +((isGen) ? 0 : 1);
                addBytes(program,foo);
                foo = add_to_vm_strings(name);
                addBytes(program,foo);
                if(inclass)
                    program.push_back(ast->childs[2]->childs.size()+1);
                else
                    program.push_back(ast->childs[2]->childs.size());
                //Push number of optional parameters
                if(!isGen)
                {
                    program.push_back(def_param);
                    bytes_done++;
                }
                bytes_done+=10;

                /////////
                int32_t I = program.size();
                program.push_back(JMP);
                addBytes(program,0);
                bytes_done += 1 + JUMPOFFSET_SIZE ;
                
                locals.push_back(C);
                
                infunc = true;
                inGen = isGen;
                returnStmtAtFnScope = false;
                vector<uint8_t> funcBody = compile(ast->childs[3]);
                infunc = false;
                
                inConstructor = false;
                inGen = false;
                locals.pop_back();
                STACK_SIZE = before;
                if(name!="__construct__")
                {
                    if (funcBody.size() == 0 || last_stmt_type!=NodeType::RETURN_NODE)
                    {
                        funcBody.push_back(LOAD_NIL);
                        if(isGen)
                            funcBody.push_back(CO_STOP);
                        else
                            funcBody.push_back(OP_RETURN);
                        bytes_done +=2;
                    }
                }
                else
                {
                    funcBody.push_back(LOAD_LOCAL);
                    foo = selfIdx;
                    addBytes(funcBody,foo);
                    funcBody.push_back(OP_RETURN);
                    bytes_done+=6;
                }
                //program.push_back(JMP);
                foo =  funcBody.size();
                memcpy(&program[I+1],&foo,sizeof(int));
                //addBytes(program,foo);

                program.insert(program.end(), funcBody.begin(), funcBody.end());
            }
            else
            {
                //printf("not compiling function %s because it is not fnReferenced\n",name.c_str());
            }
            isGen = false;
        }
        else if(ast->type==NodeType::CLASS || ast->type==NodeType::EXTCLASS)
        {
        bool extendedClass = (ast->type == NodeType::EXTCLASS);
        string name = ast->childs[1]->val;
        if(!REPL_MODE && symRef.find(name)==symRef.end())
        {
            //class not referenced in source code
            //no need to compile
            ast = ast->childs.back();
            continue;
        }

        if(globals.find(name)!=globals.end())
            compileError("NameError","Redeclaration of name "+name);
        globals.emplace(name,STACK_SIZE);
        
        STACK_SIZE+=1;
        vector<string> names = scan_class(ast->childs[2]);
        if(extendedClass)
        {
            vector<uint8_t> baseClass = expr_bytecode(ast->childs[ast->childs.size()-2]);
            program.insert(program.end(),baseClass.begin(),baseClass.end());
        }
        for(auto e: names)
        {
            if(e=="super" || e=="self")
            {
                compileError("NameError","Error class not allowed to have members named \"super\" or \"self\"");
            }
            foo = add_to_vm_strings(e);
            program.push_back(LOAD_STR);
            addBytes(program,foo);
            bytes_done+=5;
        }
        SymbolTable M;
        locals.push_back(M);
        int32_t before = STACK_SIZE;
        
        inclass = true;
        className = name;
        classMemb = names;
        vector<uint8_t> block = compile(ast->childs[2]);
        inclass = false;
        

        locals.pop_back();
        int32_t totalMembers = names.size();
        STACK_SIZE = before;

        program.insert(program.end(),block.begin(),block.end());
        if(extendedClass)
        {
            program.push_back(BUILD_DERIVED_CLASS);
            add_lntable_entry(bytes_done);
        }
        else
            program.push_back(BUILD_CLASS);

        addBytes(program,totalMembers);
        foo = add_to_vm_strings(name);
        addBytes(program,foo);
        bytes_done+=9;


        }
        else if (ast->type == NodeType::RETURN_NODE)
        {
            if(inConstructor)
                compileError("SyntaxError","Error class constructors should not return anything!");
            //stmt in it's block
            if(ast->childs[1]->type == NodeType::NUM && isnum(ast->childs[1]->val))
            {
                program.push_back(RETURN_INT32);
                addBytes(program,(int32_t)atoi(ast->childs[1]->val));
                bytes_done += 5;
            }
            else
            {
                vector<uint8_t> val = expr_bytecode(ast->childs[1]);
                program.insert(program.end(), val.begin(), val.end());
                add_lntable_entry(bytes_done);
                if(inGen)
                    program.push_back(CO_STOP);
                else
                    program.push_back(OP_RETURN);
                bytes_done += 1;
            }
        }
        else if (ast->type == NodeType::THROW_node)
        {   
            vector<uint8_t> val = expr_bytecode(ast->childs[1]);
            program.insert(program.end(), val.begin(), val.end());
            add_lntable_entry(bytes_done);
            program.push_back(THROW);
            bytes_done += 1;
        }
        else if (ast->type == NodeType::YIELD_node)
        {
            if(inConstructor)
                compileError("SyntaxError","Error class constructor can not be generators!");
            
            vector<uint8_t> val = expr_bytecode(ast->childs[1]);
            program.insert(program.end(), val.begin(), val.end());              
            add_lntable_entry(bytes_done);
            program.push_back(YIELD);
            bytes_done += 1;
        }
        else if (ast->type == NodeType::BREAK_node)
        {
            program.push_back(GOTONPSTACK);
            breakIdx.push_back(bytes_done+5);
            foo = 0;
            for(size_t i=localsBegin;i<locals.size();i++)
                foo+=locals[i].size();
            if(infor)
                foo-=1; //don't pop loop control variable
            addBytes(program,foo);
            foo = 0;
            addBytes(program,foo);
            bytes_done += 9;
        }
        else if(ast->type==NodeType::gc)
        {
            program.push_back(GC);
            bytes_done+=1;
        }
        else if (ast->type == NodeType::CONTINUE_node)
        {
            if(infor)
            {
                program.push_back(GOTO);
                bytes_done+=1;
                contIdx.push_back(bytes_done);
                addBytes(program,0);
                bytes_done+=4;
            }
            else
            {
                program.push_back(GOTONPSTACK);
                contIdx.push_back(bytes_done+5);
                foo = 0;
                for(size_t i=localsBegin;i<locals.size();i++)
                    foo+=locals[i].size();
                if(infor)
                    foo -= 1;
                addBytes(program,foo);
                foo = 0;
                addBytes(program,foo);
                bytes_done += 9;
            }
        }
        else if (ast->type == NodeType::call)
        {
            const string& name = ast->childs[1]->val;
            bool udf = (funcs.find(name) == funcs.end());  
            if (udf)
            {
                vector<uint8_t> callExpr = expr_bytecode(ast);
                program.insert(program.end(),callExpr.begin(),callExpr.end());
                program.push_back(POP_STACK);
                bytes_done+=1;
            }
            else
            {
                for (size_t k = 0; k < ast->childs[2]->childs.size(); k += 1)
                {
                    vector<uint8_t> val = expr_bytecode(ast->childs[2]->childs[k]);
                    program.insert(program.end(), val.begin(), val.end());
                }
                add_lntable_entry(bytes_done);
                program.push_back(CALL);
                bytes_done += 1;
                size_t index = 0;
                bool add = true;
                for(index = 0;index < vm.builtin.size();index+=1)
                {
                    if(vm.builtin[index]==funcs[name])
                    {
                        add = false;
                        break;
                    }
                }
                if(add)
                {
                    vm.builtin.push_back(funcs[name]);
                    foo = vm.builtin.size()-1;
                }
                else
                    foo = index;
                addBytes(program,foo);
                bytes_done += 5;
                program.push_back((char)ast->childs[2]->childs.size());

            }
        }
        else if(ast->type == NodeType::file)
        {
        if(infunc)
            compileError("SyntaxError","Error file import inside function!");
        if(inclass)
            compileError("SyntaxError","Error file import inside class!");
            
        string C = filename;
        filename = ast->childs[1]->val;
        short K = fileTOP;
        fileTOP = str_vector_search(files,filename.c_str());
        bool a = infunc;
        bool b = inclass;
        infunc = false;
        inclass = false;
        vector<uint8_t> fByteCode = compile(ast->childs[2]);
        infunc = a;
        inclass = b;
        program.insert(program.end(),fByteCode.begin(),fByteCode.end());
        filename = C;
        fileTOP = K;
        }
        else
        {
            printf("SyntaxError in file %s\n%s\nThe line does not seem to be a meaningful statement!\n", filename.c_str(), NodeTypeNames[(int)ast->type]);
            exit(1);
        }
        last_stmt_type = ast->type;
        ast = ast->childs.back();
    }
    return program;

}
void Compiler::optimize_jmps(vector<uint8_t>& bytecode)
{
    for(auto e: andJMPS)
    {
        int offset;
        memcpy(&offset,&bytecode[e+1],4);
        size_t k = e+5+offset;
        int newoffset;
        while(k < bytes_done && bytecode[k] == JMPIFFALSENOPOP)
        {
        memcpy(&newoffset,&bytecode[k+1],4);
        offset+=newoffset+5;
        k = k+5+newoffset;
        }
        memcpy(&bytecode[e+1],&offset,4);
    }
    //optimize short circuit jumps for or
    for(auto e: orJMPS)
    {
        int offset;
        memcpy(&offset,&bytecode[e+1],4);
        size_t k = e+5+offset;
        int newoffset;
        while(k < bytes_done && bytecode[k] == NOPOPJMPIF)
        {
        memcpy(&newoffset,&bytecode[k+1],4);
        offset+=newoffset+5;
        k = k+5+newoffset;
        }
        memcpy(&bytecode[e+1],&offset,4);
    }
}
void Compiler::reduceStackTo(int size)//for REPL
{
    while((int)STACK_SIZE > size)
    {
        int lastidx = (int)STACK_SIZE-1;
        int firstidx = (int)size;
        for(auto it=globals.begin();it!=globals.end();)
        {
            if((*it).second>=firstidx  && (*it).second <= lastidx)
            {
                it = globals.erase(it);
                STACK_SIZE-=1;
            }
            else
                ++it;
        }
    }
}
zclass* Compiler::make_error_class(string name,zclass* error)
{
    zclass* k = vm_alloc_zclass();
    int32_t idx = add_to_vm_strings(name);
    k->name = vm.strings[idx].val;
    StrMap_assign(&(k->members),&(error->members));
    StrMap_assign(&(k->members),&(error->privateMembers));
    globals.emplace(name,STACK_SIZE++);
    zlist_push(&vm.STACK,zobj_from_class(k));
    return k;
}
void Compiler::add_builtin(const std::string& name,BuiltinFunc fn)
{
    funcs[name] = fn;   
}
zobject make_argv_list(int argc,const char* argv[])
{
    int32_t k = 2;
    zlist* l = vm_alloc_zlist();
    while (k < argc)
    {
        zstr* p = vm_alloc_zstr(strlen(argv[k]));
        memcpy(p->val,argv[k],strlen(argv[k]));
        zlist_push(l,zobj_from_str_ptr(p));
        k += 1;
    }
    return zobj_from_list(l);
}
zobject make_zfile(FILE* fp)
{
    zfile* f = vm_alloc_zfile();
    f->fp = fp;
    f->open = true;
    return zobj_from_file(f);
}
vector<uint8_t>& Compiler::compileProgram(Node* ast,int32_t argc,const char* argv[],int32_t options)//compiles as a complete program adds NPOP_STACK and OP_EXIT
{
    //If prev vector is empty then this program will be compiled as an independent new one
    //otherwise this program will be an addon to the previous one
    //new = prev +curr
    bytes_done = bytecode.size();
    compileAllFuncs = (options & OPT_COMPILE_DEADCODE);
    line_num = 1;
    andJMPS.clear();
    orJMPS.clear();
    backpatches.clear();
    if(bytecode.size() == 0)
    {
        globals.emplace("argv",0);
        globals.emplace("stdin",1);
        globals.emplace("stdout",2);
        globals.emplace("stderr",3);
        vm.STACK.size = 0;
        zlist_push(&vm.STACK,make_argv_list(argc,argv));
        zlist_push(&vm.STACK,make_zfile(stdin));
        zlist_push(&vm.STACK,make_zfile(stdout));
        zlist_push(&vm.STACK,make_zfile(stderr));
        STACK_SIZE = vm.STACK.size;
        
        add_to_vm_strings("msg");
        
        zobject nil;
        nil.type = Z_NIL;

        Error = vm_alloc_zclass();
        Error->name = "Error";
        StrMap_emplace(&(Error->members),"msg",nil);
        //Any class inheriting from Error will have 'msg'
        globals.emplace("Error",STACK_SIZE);
        add_to_vm_strings("__construct__");
        zfun* fun = vm_alloc_zfun();
        fun->name = "__construct__";
        fun->args = 2;
        fun->i = 5;
        fun->_klass = NULL;
        vm.important.push_back((void*)fun);

        bytecode.push_back(JMP);
        addBytes(bytecode,21);
        bytecode.push_back(LOAD_LOCAL);
        addBytes(bytecode,0);
        bytecode.push_back(LOAD_LOCAL);
        addBytes(bytecode,1);
        bytecode.push_back(ASSIGNMEMB);
        addBytes(bytecode,0);
        bytecode.push_back(LOAD_LOCAL);
        addBytes(bytecode,0);
        bytecode.push_back(OP_RETURN);

        bytes_done+=26;
        zobject construct;
        construct.type = Z_FUNC;
        construct.ptr = (void*)fun;
        StrMap_emplace(&(Error->members),"__construct__",construct);
        zlist_push(&vm.STACK,zobj_from_class(Error));
        STACK_SIZE+=1;
        TypeError = make_error_class("TypeError",Error);
        ValueError = make_error_class("ValueError",Error);
        MathError  = make_error_class("MathError",Error);
        NameError = make_error_class("NameError",Error);
        IndexError = make_error_class("IndexError",Error);
        ArgumentError = make_error_class("ArgumentError",Error);
        FileIOError = make_error_class("FileIOError",Error);
        KeyError = make_error_class("KeyError",Error);
        OverflowError = make_error_class("OverflowError",Error);
        FileOpenError = make_error_class("FileOpenError",Error);
        FileSeekError  = make_error_class("FileSeekError",Error);
        ImportError = make_error_class("ImportError",Error);
        ThrowError = make_error_class("ThrowError",Error);
        MaxRecursionError =make_error_class("MaxRecursionError",Error);
        AccessError = make_error_class("AccessError",Error);
    }

    auto bt = compile(ast);
    bytecode.insert(bytecode.end(),bt.begin(),bt.end());
    bool popGlobals = (options & OPT_POP_GLOBALS);
    if(globals.size()!=0 && popGlobals)
    {
        bytecode.push_back(NPOP_STACK);
        foo = globals.size();
        addBytes(bytecode,foo);
        bytes_done+=5;
    }

    bytecode.push_back(OP_EXIT);
    bytes_done+=1;
    if (bytes_done != bytecode.size())
    {
        printf("Zuko encountered an internal error.\nError Code: 10\n");
        exit(EXIT_FAILURE);
    }
    //final phase
    //apply backpatches
    for(const Pair& p: backpatches)
    {
        memcpy(&bytecode[p.x],&p.y,sizeof(int32_t));
    }
    //optimize short circuit jumps for 'and' and 'or' operators
    optimize_jmps(bytecode);
    return bytecode;
}
