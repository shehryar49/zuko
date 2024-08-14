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
#include "builtin-map.h"
#include "builtinfunc.h"
#include "byte_src.h"
#include "convo.h"
#include "lntable.h"
#include "opcode.h"
#include "pair-vector.h"
#include "parser.h"
#include "ptr-vector.h"
#include "refgraph.h"
#include "str-vec.h"
#include "symtable.h"
#include "misc.h"
#include "vm.h"
#include "zbytearray.h"
#include "zobject.h"
#include "zuko-src.h"
#include <stdint.h>
#include <stdio.h>
#include "sizet_vector.h"
#include "repl.h"


#define INSTRUCTION(x) program.push_back(x);bytes_done++;



void initFunctions();
void initMethods();
extern ptr_vector vm_strings;
extern ptr_vector vm_builtin;
extern ptr_vector vm_important;
void extractSymRef(str_vector* ref,refgraph* graph)//gives the names of symbols that are referenced and not deadcode
{
  str_vector q;
  str_vector_init(&q);
  str_vector_push(&q,".main");

  //there is no incoming edge on .main
  //ref.emplace(".main",true);
  const char* curr;
  //printf("--begin DFS--\n");
  while(q.size != 0)
  {
    curr = q.arr[--q.size];
    str_vector* adj = refgraph_getref(graph,curr);
    if(!adj)
        puts("adj is NULL");
    for(size_t i = 0;i<adj->size;i++)
    {
        const char* e = adj->arr[i];
        if(str_vector_search(ref,e) == -1) //node not already visited or processed
        {
            str_vector_push(&q,e);
            str_vector_push(ref,e);
        }
    }
  }
  //printf("--end DFS--\n");
}
static inline void addBytes(zbytearr* vec,int32_t x)
{
  size_t sz = vec->size;
  zbytearr_resize(vec,sz + sizeof(int32_t));
  memcpy(vec->arr+sz,&x,sizeof(int32_t));
}
void compiler_set_source(compiler* ctx,zuko_src* p,size_t root_idx)
{
    if(root_idx >= p->files.size)
    {
        fprintf(stderr,"Compiler: set_source() failed. REASON: root_idx out of range!");
        exit(1);
    }
    str_vector_init(&ctx->symRef);
    ctx->symRef.size = 0;
    extractSymRef(&ctx->symRef,&p->ref_graph);
    ctx->num_of_constants = (int32_t*)&(p->num_of_constants);
    ctx->files = &p->files;
    ctx->sources = &p->sources;
    ctx->fileTOP = root_idx; // we start from the root file obviously
    ctx->line_num_table = &p->line_num_table;
    ctx->filename = p->files.arr[root_idx];
    if(p->num_of_constants > 0 && !REPL_MODE)
    {
        if(vm_constants)
            free(vm_constants);
        vm_constants = malloc(sizeof(zobject)*p->num_of_constants);
        vm_total_constants = 0;
    }
    sizet_vector_init(&ctx->andJMPS);
    sizet_vector_init(&ctx->orJMPS);
    symtable_init(&ctx->globals);
    ptr_vector_init(&ctx->locals);
    str_vector_init(&ctx->classMemb);
    sizet_vector_init(&ctx->breakIdx);
    sizet_vector_init(&ctx->contIdx);
    zbytearr_init(&ctx->bytecode);
    pair_vector_init(&ctx->backpatches);
    //init builtins
    initFunctions();
    initMethods();
    //reset all state
    ctx->inConstructor = false;
    ctx->inGen = false;
    ctx->inclass = false;
    ctx->infunc = false;
    ctx->infor = false;
    if(!REPL_MODE)
    {
        ctx->bytecode.size = 0;
        symtable_clear(&ctx->globals);
        ctx->locals.size = 0;
        ctx->className = "";
        ctx->classMemb.size  = 0;
        ctx->line_num = 1;
        ctx->andJMPS.size = 0;
        ctx->orJMPS.size = 0;
        str_vector_init(&ctx->prefixes);
        str_vector_push(&ctx->prefixes,"");
        ctx->bytes_done = 0;
    }
}
void compileError(compiler* ctx,const char* type,const char* msg)
{
    fprintf(stderr,"\nFile %s\n",ctx->filename);
    fprintf(stderr,"%s at line %zu\n",type,ctx->line_num);
    int idx = str_vector_search(ctx->files,ctx->filename);
    const char* source_code = ctx->sources->arr[idx];
    size_t l = 1;
    size_t k = 0;
    while(source_code[k]!=0 && l<=ctx->line_num)
    {
        if(source_code[k]=='\n')
            l+=1;
        else if(l==ctx->line_num)
            fputc(source_code[k],stderr);
        k+=1;
    }
    fprintf(stderr,"\n%s\n",msg);
    if(REPL_MODE)
        REPL();
    exit(1);
}
static int32_t is_duplicate_constant(zobject x)
{
    for (int32_t k = 0; k < vm_total_constants; k += 1)
    {
        if (zobject_equals(vm_constants[k],x))
            return k;
    }
    return -1;
}
int32_t add_to_vm_strings(const char* n)
{
    size_t i = 0;
    for(size_t i=0; i< vm_strings.size; i++)
    {
        zstr* e = (zstr*)(vm_strings.arr[i]);
        if(strcmp(e->val,n) == 0)
            return i;
        i++;
    }
    size_t len = strlen(n);
    char* arr = malloc(sizeof(char)*(len+1)); //will be freed by VM's destructor
    strcpy(arr,n);
    // the GC does not know about this memory
    // the string table won't be deallocated until exit, so no point in checking if
    // strings in it are reachable or not(during collect phase of GC)
    zstr* str = malloc(sizeof(zstr));
    str->len = len;
    str->val = arr;
    ptr_vector_push(&vm_strings,str);
    return (int32_t)vm_strings.size-1;
}
void add_lntable_entry(compiler* ctx,size_t opcodeIdx)
{
    byte_src tmp = {ctx->fileTOP,ctx->line_num};
    lntable_emplace(ctx->line_num_table,opcodeIdx,tmp);
}
int32_t add_builtin_to_vm(const char* name)
{
    size_t index;
    BuiltinFunc fnAddr;
    bmap_get(&funcs,name,&fnAddr);
    for(index = 0;index < vm_builtin.size;index+=1)
    {
        if(vm_builtin.arr[index]==fnAddr)
        return (size_t)index;
    }
    ptr_vector_push(&vm_builtin,fnAddr);
    return (int32_t)vm_builtin.size-1;
}
int32_t foo;
void expr_bytecode(compiler* ctx,Node* ast)
{
    zobject reg;
    if (ast->childs.size == 0)
    {
        if (ast->type == NUM)
        {
            const char* n = ast->val;
            if (isnum(n))
            {
                zbytearr_push(&ctx->bytecode,LOAD_INT32);
                addBytes(&ctx->bytecode,str_to_int32(n));
                ctx->bytes_done += 1 + sizeof(int32_t);
            }
            else if (is_int64(n))
            {
                reg.type = 'l';
                reg.l = str_to_int64(n);
                int32_t e = is_duplicate_constant(reg);
                if (e != -1)
                    foo = e;
                else
                {
                    foo = vm_total_constants;
                    reg.type = 'l';
                    reg.l = str_to_int64(n);
                    vm_constants[vm_total_constants++] = reg;
                }
                zbytearr_push(&ctx->bytecode,LOAD_CONST);
                addBytes(&ctx->bytecode,foo);
                ctx->bytes_done += 1 + sizeof(int32_t);
            }
            else
            {
                char buffer[50];
                snprintf(buffer,50,"Error integer %s causes overflow!",n);
                compileError(ctx,"OverflowError", buffer);
            }
            return;
        }
        else if (ast->type == FLOAT)
        {
            const char* n = ast->val;
            bool neg = false;
            if (n[0] == '-')
            {
                neg = true;
                //n++;
            }
            reg.type = 'f';
            reg.f = str_to_double(n);
            int32_t e = is_duplicate_constant(reg);
            if (e != -1)
                foo = e;
            else
            {
                foo = vm_total_constants;
                vm_constants[vm_total_constants++] = reg;
            }
            zbytearr_push(&ctx->bytecode,LOAD_CONST);
            addBytes(&ctx->bytecode,foo);
            ctx->bytes_done += 1 + sizeof(int32_t);
            return;
        }
        else if (ast->type ==  BOOL_NODE)
        {
            const char* n = ast->val;
            zbytearr_push(&ctx->bytecode,(strcmp(n,"true") == 0) ? LOAD_TRUE : LOAD_FALSE);
            ctx->bytes_done+=1;
            return;
        }
        else if (ast->type == STR_NODE)
        {
            zbytearr_push(&ctx->bytecode,LOAD_STR);
            foo = add_to_vm_strings(ast->val);
            addBytes(&ctx->bytecode,foo);
            ctx->bytes_done += 5;
            return;
        }
        else if (ast->type == NIL)
        {
            zbytearr_push(&ctx->bytecode,LOAD_NIL);
            ctx->bytes_done += 1;
            return;
        }
        else if (ast->type == ID_NODE)
        {
            const char* name = ast->val;
            bool isGlobal = false;
            bool isSelf = false;
            foo = resolve_name(name,&isGlobal,true,&isSelf);
            if(isSelf)
            {
                add_lntable_entry(ctx,ctx->bytes_done);
                zbytearr_push(&ctx->bytecode,SELFMEMB);
                foo = add_to_vm_strings(ast->val);
                addBytes(&ctx->bytecode,foo);
                ctx->bytes_done+=5;
                return;
            }
            else if(!isGlobal)
            {
                zbytearr_push(&ctx->bytecode,LOAD_LOCAL);
                addBytes(&ctx->bytecode,foo);
                ctx->bytes_done+=5;
                return;
            }
            else
            {
                zbytearr_push(&ctx->bytecode,LOAD_GLOBAL);
                addBytes(&ctx->bytecode,foo);
                ctx->bytes_done += 5;
                return;
            }
        }
        else if (ast->type == BYTE_NODE)
        {
            zbytearr_push(&ctx->bytecode,LOAD_BYTE);
            zbytearr_push(&ctx->bytecode,tobyte(ast->val));
            ctx->bytes_done+=2;
            return;
        }

    }
    if (ast->type == list)
    {
        
        for (size_t k = 0; k < ast->childs.size; k += 1)
        {
            expr_bytecode(ctx,ast->childs.arr[k]);
        }
        zbytearr_push(&ctx->bytecode,LOAD);
        zbytearr_push(&ctx->bytecode,'j');
        foo = ast->childs.size;
        addBytes(&ctx->bytecode,foo);
        ctx->bytes_done += 2 + 4;
        return;
    }
    if (ast->type == dict)
    {
        
        for (size_t k = 0; k < ast->childs.size; k += 1)
        {
            expr_bytecode(ctx,ast->childs.arr[k]);
        }
        add_lntable_entry(ctx,ctx->bytes_done);//runtime errors can occur
        zbytearr_push(&ctx->bytecode,LOAD);
        zbytearr_push(&ctx->bytecode,'a');
        foo = ast->childs.size / 2;//number of key value pairs in dictionary
        addBytes(&ctx->bytecode,foo);
        ctx->bytes_done += 2 + 4;
        return;
    }
    if (ast->type == add)
    {
        if(ast->childs.arr[0]->type == ID_NODE && ast->childs.arr[1]->type == NUM && isnum(ast->childs.arr[1]->val))
        {
            bool self1 = false;
            bool is_global1;
            
            int i = resolve_name(ast->childs.arr[0]->val,&is_global1,true,&self1);
            if(!self1)
            {
                zbytearr_push(&ctx->bytecode,LOADVAR_ADDINT32);
                zbytearr_push(&ctx->bytecode,(is_global1)? 1 : 0);
                addBytes(&ctx->bytecode, i);
                addBytes(&ctx->bytecode, (int32_t)atoi(ast->childs.arr[1]->val));
                ctx->bytes_done += 10;
                return;
            }
            
        }
        expr_bytecode(ast->childs.arr[0]);
        expr_bytecode(ast->childs.arr[1]);
        zbytearr_push(&bytecode,ADD);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return;
    }
    if (ast->type == sub)
    {
        if(ast->childs.arr[0]->type == NodeType::ID_NODE && ast->childs.arr[1]->type == NodeType::NUM && isnum(ast->childs.arr[1]->val))
        {
            bool self1 = false;
            bool is_global1;
            
            int i = resolve_name(ast->childs.arr[0]->val,&is_global1,true,&self1);
            if(!self1)
            {
                zbytearr_push(&bytecode,LOADVAR_SUBINT32);
                zbytearr_push(&bytecode,(is_global1)? 1 : 0);
                addBytes(&bytecode, i);
                addBytes(&bytecode, (int32_t)atoi(ast->childs.arr[1]->val));
                bytes_done += 10;
                return;
            }
            
        }
        expr_bytecode(ast->childs.arr[0]);
        expr_bytecode(ast->childs.arr[1]);
        zbytearr_push(&bytecode,SUB);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
    
        return;
    }
    if (ast->type == div_node)
    {
        
        expr_bytecode(ast->childs.arr[0]);
        expr_bytecode(ast->childs.arr[1]);
        zbytearr_push(&bytecode,DIV);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return;
    }
    if (ast->type == mul)
    {
        expr_bytecode(ast->childs.arr[0]);
        expr_bytecode(ast->childs.arr[1]);
        zbytearr_push(&bytecode,MUL);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return;
    }
    if (ast->type == XOR_node)
    {
        expr_bytecode(ast->childs.arr[0]);
        expr_bytecode(ast->childs.arr[1]);
        zbytearr_push(&bytecode,XOR);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return;
    }
    if (ast->type == mod)
    {
        expr_bytecode(ast->childs.arr[0]);
        expr_bytecode(ast->childs.arr[1]);
        zbytearr_push(&bytecode,MOD);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return;
    }
    if (ast->type == lshift)
    {
        expr_bytecode(ast->childs.arr[0]);
        expr_bytecode(ast->childs.arr[1]);
        zbytearr_push(&bytecode,LSHIFT);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return;
    }
    if (ast->type == rshift)
    {
        expr_bytecode(ast->childs.arr[0]);
        expr_bytecode(ast->childs.arr[1]);
        zbytearr_push(&bytecode,RSHIFT);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return;
    }
    if (ast->type == bitwiseand)
    {
        expr_bytecode(ast->childs.arr[0]);
        expr_bytecode(ast->childs.arr[1]);
        zbytearr_push(&bytecode,BITWISE_AND);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return;
    }
    if (ast->type == bitwiseor)
    {
        expr_bytecode(ast->childs.arr[0]);
        expr_bytecode(ast->childs.arr[1]);
        zbytearr_push(&bytecode,BITWISE_OR);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return;
    }
    if (ast->type == memb)
    {
        
        if (ast->childs.arr[1]->type == NodeType::call)
        {
            Node* callnode = ast->childs.arr[1];

            expr_bytecode(ast->childs.arr[0]);
            Node* args = callnode->childs.arr[2];
            for (size_t f = 0; f < args->childs.size; f += 1)
            {
                expr_bytecode(args->childs.arr[f]);
            }
            zbytearr_push(&bytecode,CALLMETHOD);
            add_lntable_entry(bytes_done);
            byte_src tmp;
            const char* memberName = callnode->childs.arr[1]->val;
            foo = add_to_vm_strings(memberName);
            addBytes(&bytecode,foo);
            zbytearr_push(&bytecode,args->childs.size);
            
            bytes_done +=6;

            return;
        }
        else
        { 
            expr_bytecode(ast->childs.arr[0]);
            add_lntable_entry(bytes_done);
            zbytearr_push(&bytecode,MEMB);
            if (ast->childs.arr[1]->type != NodeType::ID_NODE)
                compileError("SyntaxError", "Invalid Syntax");
            const char* name = ast->childs.arr[1]->val;
            foo = add_to_vm_strings(name);
            addBytes(&bytecode,foo);
            bytes_done +=5;
            return;
        }
    }
    if (ast->type == AND)
    {
        expr_bytecode(ast->childs.arr[0]);
        zbytearr_push(&bytecode,JMPIFFALSENOPOP);
        sizet_vector_push(&andJMPS,bytes_done);
        int32_t I = bytecode.size;
        zbytearr_push(&bytecode,0);
        zbytearr_push(&bytecode,0);
        zbytearr_push(&bytecode,0);
        zbytearr_push(&bytecode,0);
        bytes_done+=5;
        size_t before = bytes_done;
        expr_bytecode(ast->childs.arr[1]);
        size_t after = bytes_done;
        foo = after-before;
        memcpy(bytecode.arr+I,&foo,sizeof(int32_t));
        return;
    }
    if (ast->type == IS_node)
    {
        expr_bytecode(ast->childs.arr[0]);
        expr_bytecode(ast->childs.arr[1]);
        zbytearr_push(&bytecode,IS);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return;
    }
    if (ast->type == OR)
    {
        expr_bytecode(ast->childs.arr[0]);
        zbytearr_push(&bytecode,NOPOPJMPIF);
        sizet_vector_push(&orJMPS,bytes_done);
        int32_t I = bytecode.size;
        zbytearr_push(&bytecode,0);
        zbytearr_push(&bytecode,0);
        zbytearr_push(&bytecode,0);
        zbytearr_push(&bytecode,0);
        bytes_done+=5;
        size_t before = bytes_done;
        expr_bytecode(ast->childs.arr[1]);
        size_t after = bytes_done;
        foo = after - before;
        memcpy(bytecode.arr + I,&foo,sizeof(int32_t));
        return;
    }
    if (ast->type == lt)
    {
        expr_bytecode(ast->childs.arr[0]);
        expr_bytecode(ast->childs.arr[1]);
        zbytearr_push(&bytecode,SMALLERTHAN);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return;
    }
    if (ast->type == gt)
    {
        expr_bytecode(ast->childs.arr[0]);
        expr_bytecode(ast->childs.arr[1]);
        zbytearr_push(&bytecode,GREATERTHAN);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return;
    }
    if (ast->type == equal)
    {
        expr_bytecode(ast->childs.arr[0]);
        expr_bytecode(ast->childs.arr[1]);
        zbytearr_push(&bytecode,EQ);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return;
    }
    if (ast->type == noteq)
    {
        expr_bytecode(ast->childs.arr[0]);
        expr_bytecode(ast->childs.arr[1]);
        zbytearr_push(&bytecode,NOTEQ);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return;
    }
    if (ast->type == gte)
    {
        expr_bytecode(ast->childs.arr[0]);
        expr_bytecode(ast->childs.arr[1]);
        zbytearr_push(&bytecode,GROREQ);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return;
    }
    if (ast->type == lte)
    {
        expr_bytecode(ast->childs.arr[0]);
        expr_bytecode(ast->childs.arr[1]);
        zbytearr_push(&bytecode,SMOREQ);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return;
    }
    if (ast->type == neg)
    {
        expr_bytecode(ast->childs.arr[0]);
        zbytearr_push(&bytecode,NEG);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return;
    }
    if (ast->type == complement)
    {
        expr_bytecode(ast->childs.arr[0]);
        zbytearr_push(&bytecode,COMPLEMENT);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return;
    }
    if (ast->type == NOT_node)
    {
        expr_bytecode(ast->childs.arr[0]);
        zbytearr_push(&bytecode,NOT);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return;
    }
    if (ast->type == index_node)
    {
        int32_t idx1;
        int32_t idx2;

        if(ast->childs.arr[0]->type == NodeType::ID_NODE && ast->childs.arr[1]->type == NodeType::ID_NODE)
        {
            bool self1 = false;
            bool self2 = false;
            bool is_global1;
            bool is_global2;
            
            int i = resolve_name(ast->childs.arr[0]->val,&is_global1,true,&self1);
            int j = resolve_name(ast->childs.arr[1]->val,&is_global2,true,&self2);
            if(!self1 && !self2)
            {
                zbytearr_push(&bytecode,INDEX_FAST);
                zbytearr_push(&bytecode,(is_global1)? 1 : 0);
                zbytearr_push(&bytecode,(is_global2)? 1 : 0);
                addBytes(&bytecode, i);
                addBytes(&bytecode, j);
                bytes_done += 11;
                return;
            }
            
        }
        expr_bytecode(ast->childs.arr[0]);
        expr_bytecode(ast->childs.arr[1]);
        zbytearr_push(&bytecode,INDEX);
        add_lntable_entry(bytes_done);
        bytes_done += 1;
        return;
    }
    if (ast->type == call)
    {
        const char* name = ast->childs.arr[1]->val;
        bool udf = false;
        if (!function_exists(name))//check if it is builtin function
            udf = true;
        if (udf)
        {
            bool isGlobal = false;
            bool isSelf = false;
            int foo = resolve_name(name,&isGlobal,true,&isSelf);
            /*if(isGlobal && compiled_functions.find(name)!=compiled_functions.end()) // Make direct call
            {
                auto p = compiled_functions[name];
                Node* fn = p.first;
                int32_t stack_idx = p.second;
                int32_t N = ast->childs.arr[2]->childs.size; //arguments given
                int32_t args_required = fn->childs.arr[2]->childs.size;
                int32_t opt_args = 0;
                int32_t def_params = 0;
                int32_t def_begin = -1;
                for(size_t i=0;i<fn->childs.arr[2]->childs.size;i++)
                {
                    Node* arg = fn->childs.arr[2]->childs.arr[i];
                    if(arg->childs.size!=0)
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
                for(size_t i=0;i<ast->childs.arr[2]->childs.size;i++)
                {
                    Node* arg = ast->childs.arr[2]->childs.arr[i];
                    expr_bytecode(arg);
                }
                //load optional args
                for (size_t i = def_params - (args_required - N); i < def_params; i++)
                {
                    //printf("i = %zu\n",i);
                    Node* arg = fn->childs.arr[2]->childs.arr[def_begin+i]->childs.arr[0];
                    expr_bytecode(arg);
                }
                zbytearr_push(&bytecode,CALL_DIRECT);
                addBytes(&bytecode, stack_idx);
                bytes_done+=5;
                return;
            }*/
            if(isSelf)
            {
                zbytearr_push(&bytecode,LOAD_LOCAL);
                addBytes(&bytecode,0);//load self to pass as first argument
                bytes_done+=5;
            }
            for (size_t k = 0; k < ast->childs.arr[2]->childs.size; k += 1)
            {
                expr_bytecode(ast->childs.arr[2]->childs.arr[k]);
            }
            if(isSelf)
            {
                add_lntable_entry(bytes_done);
                zbytearr_push(&bytecode,SELFMEMB);
                foo = add_to_vm_strings(name);
                addBytes(&bytecode,foo);
                bytes_done+=5;
                add_lntable_entry(bytes_done);
                zbytearr_push(&bytecode,CALLUDF);
                zbytearr_push(&bytecode,ast->childs.arr[2]->childs.size+1);
                bytes_done+=2;
                return;
            }
            else if(isGlobal)
                zbytearr_push(&bytecode,LOAD_GLOBAL);
            else
                zbytearr_push(&bytecode,LOAD_LOCAL);
            addBytes(&bytecode,foo);
            bytes_done+=5;
            add_lntable_entry(bytes_done);
            zbytearr_push(&bytecode,CALLUDF);
            zbytearr_push(&bytecode,ast->childs.arr[2]->childs.size);
            bytes_done+=2;
            return;
        }
        else
        {
            for (size_t k = 0; k < ast->childs.arr[2]->childs.size; k += 1)
            {
                expr_bytecode(ast->childs.arr[2]->childs.arr[k]);
            }
            add_lntable_entry(bytes_done);
            zbytearr_push(&bytecode,CALLFORVAL);
            bool add = true;
            size_t index = 0;
            BuiltinFunc fnAddr;
            bmap_get(&funcs,name,&fnAddr);
            for(index = 0;index < vm_builtin.size();index+=1)
            {

                if(vm_builtin[index]==fnAddr)
                {
                    add = false;
                    break;
                }
            }
            if(add)
            {
                vm_builtin.push_back(fnAddr);
                foo = vm_builtin.size()-1;
            }
            else
                foo = index;
            addBytes(&bytecode,foo);
            zbytearr_push(&bytecode,(char)ast->childs.arr[2]->childs.size);
            bytes_done += 6;
            return;
        }
    }
    if (ast->type == YIELD_node)
    {
        if(inConstructor)
            compileError("SyntaxError","Error class constructor can not be generators!");
        
        expr_bytecode(ast->childs.arr[1]);         
        add_lntable_entry(bytes_done);
        zbytearr_push(&bytecode,YIELD_AND_EXPECTVAL);
        bytes_done += 1;
        return;
    }  
    
    compileError("SyntaxError", "Invalid syntax in expression");
    return;//to prevent compiler warning
}
void scan_class(Node* ast)
{
    classMemb.size = 0;
    while(ast->type!=NodeType::EOP)
    {
        if(ast->type==NodeType::declare)
        {
            const char* n = ast->val;
            if(n[0]=='@')
                n++;
            if(str_vector_search(&classMemb,n)!=-1 || str_vector_search(&classMemb,(ast->val))!=-1)
            {
                line_num = atoi(ast->childs.arr[0]->val);
                snprintf(error_buffer,100,"Error redeclaration of %s",n);
                compileError("NameError",error_buffer);
            }
            str_vector_push(&classMemb,ast->val);
        }
        else if(ast->type == NodeType::FUNC)
        {
            const char* n = ast->childs.arr[1]->val;
            if(n[0]=='@')
                n++;
            if(str_vector_search(&classMemb,n)!=-1 || str_vector_search(&classMemb,ast->val)!=-1)
            {
                line_num = atoi(ast->childs.arr[0]->val);
                snprintf(error_buffer,100,"Error redeclaration of %s",n);
                compileError("NameError",error_buffer);
            }
            str_vector_push(&classMemb,ast->childs.arr[1]->val);
        }
        else if(ast->type == NodeType::CORO) //generator function or coroutine
        {
            line_num = atoi(ast->childs.arr[0]->val);
            compileError("NameError","Error coroutine inside class not allowed.");
        }
        
        else if(ast->type==NodeType::CLASS)
        {
            line_num = atoi(ast->childs.arr[0]->val);
            compileError("SyntaxError","Error nested classes not supported");     
        }
        ast = ast->childs.arr[ast->childs.size-1];
    }
}
int32_t Compiler::resolve_name(const char* name,bool* isGlobal,bool blowUp,bool* isFromSelf)
{
    *isGlobal = false;
    for(int32_t i=locals.size-1;i>=0;i-=1)
    {
        size_t tmp;
        if(symtable_get((symtable*)locals.arr[i],name,&tmp))
            return tmp;
    }

    if(isFromSelf)
    {
        char buffer[strlen(name)+2];
        snprintf(buffer,50,"@%s",name);
        if(inclass && infunc && str_vector_search(&classMemb,name)!=-1 || str_vector_search(&classMemb,buffer)!=-1 )
        {
            *isFromSelf = true;
            return -2;
        }
    }
    for(int32_t i=prefixes.size-1;i>=0;i--)
    {
        const char* prefix = prefixes.arr[i];
        size_t val;
        char* new_name = merge_str(prefix,name);
        if(symtable_get(&globals,new_name,&val))
        {
            *isGlobal = true;
            free((void*)new_name);
            return val;
        }
        free(new_name);
    }
    if(blowUp)
    {
        char buffer[60];
        snprintf(buffer,60,"Error name %s is not defined!", name);
        compileError("NameError",buffer);
    }
    return -1;
}
size_t Compiler::compile(Node* ast)
{
    size_t bytes_done_before = bytes_done; 
    bool isGen = false;
    bool dfor = false;
    while (ast->type != NodeType::EOP)
    {
        if(ast->childs.size >= 1)
        {
        if (ast->childs.arr[0]->type == NodeType::line_node)
            line_num = str_to_int32(ast->childs.arr[0]->val);
        }
        if (ast->type == NodeType::declare)
        {
            expr_bytecode(ast->childs.arr[1]);
            const char* name = ast->val;
            size_t tmp;
            if (locals.size == 0)
            {
                if (symtable_get(&globals, name, &tmp))
                {
                    snprintf(error_buffer,100,"Error redeclaration of variable %s",name);
                    compileError("NameError", error_buffer);
                }
                foo = STACK_SIZE;
                if(!inclass)
                    symtable_emplace(&globals,ast->val,foo);
                STACK_SIZE+=1;
            }
            else
            {
                symtable* last = (symtable*)locals.arr[locals.size-1];

                if(symtable_get(last,name,&tmp))
                {
                    snprintf(error_buffer,100,"Error redeclaration of variable %s",name);
                    compileError("NameError", error_buffer);
                }
                foo = STACK_SIZE;
                if(inclass && !infunc);
                else
                    symtable_emplace(last,name,foo);
                STACK_SIZE+=1;
            }

        }
        else if (ast->type == NodeType::import || ast->type==NodeType::importas)
        {
            const char* name = ast->childs.arr[1]->val;
            const char* vname = (ast->type==NodeType::importas) ? ast->childs.arr[2]->val : name;
            bool f;
            if(resolve_name(vname,&f,false)!=-1)
            {
                char buffer[50];
                snprintf(buffer,50,"Error redeclaration of name %s",vname);
                compileError("NameError",buffer);
            }
            add_lntable_entry(bytes_done);
            zbytearr_push(&bytecode,IMPORT);
            foo = add_to_vm_strings(name);
            addBytes(&bytecode,foo);
            bytes_done += 5;
            if(locals.size == 0)
                symtable_emplace(&globals,vname,STACK_SIZE);
            else
            {
                symtable* last = (symtable*)locals.arr[locals.size-1];
                symtable_emplace(last,vname,STACK_SIZE);
            }
            STACK_SIZE+=1;
        }
        else if (ast->type == NodeType::assign)
        {        
            const char* name = ast->childs.arr[1]->val;
            bool doit = true;
            if (ast->childs.arr[1]->type == NodeType::ID_NODE)
            {
                if (ast->childs.arr[2]->childs.size == 2)
                {
                    if (ast->childs.arr[2]->type == NodeType::add && ast->childs.arr[2]->childs.arr[0]->val == ast->childs.arr[1]->val && ast->childs.arr[2]->childs.arr[0]->type==NodeType::ID_NODE && ast->childs.arr[2]->childs.arr[1]->type==NodeType::NUM && strcmp(ast->childs.arr[2]->childs.arr[1]->val,"1") == 0)
                    {
                    bool isGlobal = false;
                    bool isSelf = false;
                    int32_t idx = resolve_name(name,&isGlobal,false,&isSelf);
                    if(idx==-1)
                    {
                        char buffer[50];
                        snprintf(buffer,50,"Error name %s is not defined!",name);
                        compileError("NameError",buffer);
                    }
                    if(isSelf)
                    {
                        expr_bytecode(ast->childs.arr[2]);
                    }
                    add_lntable_entry(bytes_done);
                    foo = idx;
                    if(isSelf)
                    {
                        foo = add_to_vm_strings(name);
                        zbytearr_push(&bytecode,ASSIGNSELFMEMB);
                    }
                    else if(!isGlobal)
                        zbytearr_push(&bytecode,INPLACE_INC);
                    else
                        zbytearr_push(&bytecode,INC_GLOBAL);
                    addBytes(&bytecode,foo);
                    bytes_done+=5;
                    doit = false;
                    }
                }
                if (doit)
                {
                    bool isGlobal = false;
                    bool isSelf = false;
                    int32_t idx = resolve_name(name,&isGlobal,false,&isSelf);
                    if(idx==-1)
                    {
                        char buffer[50];
                        snprintf(buffer,50,"Error name %s is not defined!",name);
                        compileError("NameError",buffer);
                    }
                    expr_bytecode(ast->childs.arr[2]);
                    add_lntable_entry(bytes_done);
                    if(isSelf)
                    {
                    idx = add_to_vm_strings(name);
                    zbytearr_push(&bytecode,ASSIGNSELFMEMB);
                    }
                    else if(!isGlobal)
                    zbytearr_push(&bytecode,ASSIGN);
                    else
                    zbytearr_push(&bytecode,ASSIGN_GLOBAL);
                    addBytes(&bytecode,idx);
                    bytes_done += 5;
                }
            }
            else if (ast->childs.arr[1]->type == NodeType::index_node)
            {
                //reassign index
                expr_bytecode(ast->childs.arr[1]->childs.arr[0]);
                expr_bytecode(ast->childs.arr[2]);
                expr_bytecode(ast->childs.arr[1]->childs.arr[1]);
                zbytearr_push(&bytecode,ASSIGNINDEX);
                add_lntable_entry(bytes_done);
                bytes_done+=1;
            }
            else if (ast->childs.arr[1]->type == NodeType::memb)
            {
                //reassign object member
                expr_bytecode(ast->childs.arr[1]->childs.arr[0]);
                if(ast->childs.arr[1]->childs.arr[1]->type!=NodeType::ID_NODE)
                    compileError("SyntaxError","Invalid Syntax");
                const char* mname = ast->childs.arr[1]->childs.arr[1]->val;
                expr_bytecode(ast->childs.arr[2]);
                zbytearr_push(&bytecode,ASSIGNMEMB);
                foo = add_to_vm_strings(mname);
                addBytes(&bytecode,foo);
                add_lntable_entry(bytes_done);
                bytes_done+=5;

            }
        }
        else if (ast->type == NodeType::memb)
        {
            Node* bitch = new_node(NodeType::memb,".");
            nodeptr_vector_push(&(bitch->childs),ast->childs.arr[1]);
            nodeptr_vector_push(&(bitch->childs),ast->childs.arr[2]);
            expr_bytecode(bitch);
            free(bitch);
            zbytearr_push(&bytecode,POP_STACK);
            bytes_done +=1;
        }
        else if (ast->type == NodeType::WHILE || ast->type == NodeType::DOWHILE)
        {
            size_t offset1_idx; 
            size_t offset2_idx;
            if(ast->type==NodeType::DOWHILE)
            {
                //to skip the condition first time
                zbytearr_push(&bytecode,GOTO);
                addBytes(&bytecode,0);
                offset1_idx = bytes_done+1;
                bytes_done += 5;
            }
            
            size_t L = bytes_done;
            expr_bytecode(ast->childs.arr[1]);
            zbytearr_push(&bytecode,GOTOIFFALSE);
            offset2_idx = bytes_done + 1;
            addBytes(&bytecode,0);
            bytes_done += 5;

            int32_t before = STACK_SIZE;
            symtable m;
            symtable_init(&m);
            ptr_vector_push(&locals,&m);

            size_t breakIdxSizeCopy = breakIdx.size;
            size_t contIdxSizeCopy = contIdx.size;

            int32_t localsBeginCopy = localsBegin; //idx of vector locals, from where
            //the locals of this loop begin
            localsBegin = locals.size - 1;
            size_t loop_body_begin = bytes_done;
            size_t loop_body_size = compile(ast->childs.arr[2]);
            localsBegin = localsBeginCopy; //backtrack
            STACK_SIZE = before;

            
            int32_t whileLocals = m.size;
            locals.size--;
            //
            if(whileLocals!=0)
            {
                zbytearr_push(&bytecode,GOTONPSTACK);
                foo = whileLocals;
                addBytes(&bytecode,foo);
                bytes_done += 4;
            }
            else
                zbytearr_push(&bytecode,GOTO);
            foo = L; // goto the start of loop
            addBytes(&bytecode,foo);
            bytes_done += 1 + JUMPOFFSET_SIZE;
            // backpatch break and continue offsets
            int32_t a = (int)bytes_done;
            int32_t b = (int)L;
            
            for(size_t i=breakIdxSizeCopy;i<breakIdx.size;i++)
            {
                pair_vector_push(&backpatches,breakIdx.arr[i],a);  
            }
            for(size_t i=contIdxSizeCopy;i<contIdx.size;i++)
            {
                pair_vector_push(&backpatches,contIdx.arr[i],b);
            }
            if(ast->type == NodeType::DOWHILE)
                pair_vector_push(&backpatches,offset1_idx,loop_body_begin);
            pair_vector_push(&backpatches,offset2_idx,bytes_done);
            //backtrack
            breakIdx.size = breakIdxSizeCopy;
            contIdx.size  = contIdxSizeCopy;
            symtable_destroy(&m);
        }
        else if((ast->type == NodeType::FOR) || (dfor = (ast->type == NodeType::DFOR)))
        {
            bool decl_variable = (ast->childs.arr[1]->type == NodeType::decl_node);
            size_t linenum_copy = line_num;
            const char* loop_var_name = ast->childs.arr[2]->val;
            int32_t lcv_idx;
            int32_t end_idx;
            int32_t step_idx;
            bool lcv_is_global = false;
            int32_t stack_before = STACK_SIZE;
            // bytecode to initialize loop variable
            if(decl_variable)
            {
                expr_bytecode(ast->childs.arr[3]);//will load initial value onto stack
                lcv_idx = STACK_SIZE;
                STACK_SIZE++; //this initial value becomes a local variable
            } 
            else
            {
                bool is_self = false;
                int tmp = resolve_name(loop_var_name,&lcv_is_global,true,&is_self);
                if(is_self)
                    compileError("SyntaxError","for loop control variable can't be from 'self'");
                lcv_idx = tmp;
                //assign initial value
                expr_bytecode(ast->childs.arr[3]);
                if(lcv_is_global)
                    zbytearr_push(&bytecode,ASSIGN_GLOBAL);
                else
                    zbytearr_push(&bytecode,ASSIGN);
                addBytes(&bytecode,lcv_idx);
                bytes_done+=5;
            }
            //load end value
            expr_bytecode(ast->childs.arr[4]);
            end_idx = STACK_SIZE++;
            //load step size
            expr_bytecode(ast->childs.arr[5]); // step value
            step_idx = STACK_SIZE++;
            
            // bytecode to skip loop body if condition is not met
            line_num = linenum_copy;
            add_lntable_entry(bytes_done);
            if(dfor)
                zbytearr_push(&bytecode,SETUP_DLOOP);
            else
                zbytearr_push(&bytecode,SETUP_LOOP);
            bytes_done+=1;
            if(lcv_is_global)
            {
                zbytearr_push(&bytecode,1);
                addBytes(&bytecode,lcv_idx);
                addBytes(&bytecode, end_idx);
                bytes_done+=9;
            }
            else
            {
                zbytearr_push(&bytecode,0);
                addBytes(&bytecode,lcv_idx);
                bytes_done+=5;
            }
            int32_t skip_loop = bytes_done;
            addBytes(&bytecode,0); // apply backpatch here
            bytes_done += 4;

            // add loop body
            symtable m;
            symtable_init(&m);
            if(decl_variable)
                symtable_emplace(&m,loop_var_name,lcv_idx);
            localsBegin = locals.size;
            ptr_vector_push(&locals, &m);
            size_t stacksize_before_body = STACK_SIZE;
            size_t loop_start = bytes_done;
            bool infor_copy = infor;
            size_t breakIdxSizeCopy = breakIdx.size;
            size_t contIdxSizeCopy= contIdx.size;

            infor = true;
            size_t a = bytes_done;
            compile(ast->childs.arr[6]);
            size_t b = bytes_done;
            infor = infor_copy;
            void* t;
            ptr_vector_pop(&locals,&t);
            int body_locals = STACK_SIZE - stacksize_before_body;
            STACK_SIZE = stacksize_before_body;
            /*if(body_locals == 1)
            {
                zbytearr_push(&bytecode,POP_STACK);
                bytes_done++;
            }
            else if(body_locals > 1)
            {
                zbytearr_push(&bytecode,NPOP_STACK);
                addBytes(&bytecode,body_locals);
                bytes_done+=5;
            }*/
            int32_t cont_dest = (int32_t)bytes_done;
            // finally add LOOP instruction
            line_num = linenum_copy;
            add_lntable_entry(bytes_done);
            if(dfor)
                zbytearr_push(&bytecode,DLOOP);
            else
                zbytearr_push(&bytecode,LOOP);
            if(lcv_is_global)
                zbytearr_push(&bytecode,1);
            else
                zbytearr_push(&bytecode,0);
            bytes_done+=1;
            addBytes(&bytecode, lcv_idx); //can be global or local
            if(lcv_is_global)
            {
                addBytes(&bytecode, end_idx); // is a local ( see above we reserved stack space for it)
                bytes_done+=4;
            }
            addBytes(&bytecode, (int32_t)loop_start);
            bytes_done += 9;//10;//18;
            
            //cleanup the locals we created
            int32_t break_dest = (int32_t)bytes_done;
            zbytearr_push(&bytecode,NPOP_STACK);
            if(decl_variable)
                addBytes(&bytecode,3);
            else
                addBytes(&bytecode,2);
            bytes_done += 5;
            for(size_t i=breakIdxSizeCopy;i<breakIdx.size;i++)
            {
                pair_vector_push(&backpatches,breakIdx.arr[i],break_dest);
            }
            for(size_t i=contIdxSizeCopy;i<contIdx.size;i++)
            {
                pair_vector_push(&backpatches,contIdx.arr[i],cont_dest);
            }
            pair_vector_push(&backpatches,skip_loop,(int32_t)break_dest);
            breakIdx.size = breakIdxSizeCopy;
            contIdx.size = contIdxSizeCopy;
            STACK_SIZE = stack_before;
            symtable_destroy(&m);
        }
        else if (ast->type == NodeType::FOREACH)
        {
            const char* loop_var_name = ast->childs.arr[1]->val;
            int32_t fnIdx = add_builtin_to_vm("len"); //builtin len()
            int32_t len_idx;
            int32_t lcv_idx;
            int32_t list_idx;
            size_t goto1_offset_idx;

            //load integer iterator
            zbytearr_push(&bytecode,LOAD_INT32);
            addBytes(&bytecode,0);
            bytes_done += 5;
            lcv_idx = STACK_SIZE;
            STACK_SIZE++;
            //load the list and calculate length
            expr_bytecode(ast->childs.arr[2]); //load the list
            zbytearr_push(&bytecode,CALLFORVAL);
            addBytes(&bytecode,fnIdx);
            zbytearr_push(&bytecode,1);
            bytes_done += 6;
            len_idx = STACK_SIZE;
            STACK_SIZE++;

            expr_bytecode(ast->childs.arr[2]); //load the list
            list_idx = STACK_SIZE;
            STACK_SIZE++;

            // Generate loop condition
            int32_t loop_start = bytes_done;
            zbytearr_push(&bytecode,LOAD_LOCAL);
            addBytes(&bytecode,lcv_idx);
            zbytearr_push(&bytecode,LOAD_LOCAL);
            addBytes(&bytecode,len_idx);
            bytes_done += 10;
            zbytearr_push(&bytecode,SMALLERTHAN);
            add_lntable_entry(bytes_done);
            zbytearr_push(&bytecode,GOTOIFFALSE);
            goto1_offset_idx = bytes_done + 2;
            addBytes(&bytecode,0);
            bytes_done += 6;



            //load the indexed element
            symtable m;
            symtable_init(&m);
            zbytearr_push(&bytecode,LOAD_LOCAL);
            addBytes(&bytecode,list_idx);
            zbytearr_push(&bytecode,LOAD_LOCAL);
            addBytes(&bytecode,lcv_idx);
            bytes_done += 10;
            zbytearr_push(&bytecode,INDEX);
            add_lntable_entry(bytes_done++);
            size_t stack_size_before = STACK_SIZE;
            symtable_emplace(&m,loop_var_name,STACK_SIZE++);
            ptr_vector_push(&locals,&m);

            size_t breakIdxSizeCopy = breakIdx.size;
            size_t contIdxSizeCopy = contIdx.size;
            int32_t localsBeginCopy = localsBegin; //idx of vector locals, from where
            //the locals of this loop begin
            localsBegin = locals.size - 1;
            bool a = infor;
            infor = false; // we are compiling foreach not for loop
            compile(ast->childs.arr[3]);
            infor = a;
            localsBegin = localsBeginCopy;
            locals.size--;
            STACK_SIZE = stack_size_before;

            size_t cont_target = bytes_done;
            zbytearr_push(&bytecode,INPLACE_INC);
            addBytes(&bytecode,lcv_idx);
            bytes_done += 5;
            int32_t whileLocals = m.size;
            if(whileLocals != 0)
            {
                zbytearr_push(&bytecode,GOTONPSTACK);
                addBytes(&bytecode,whileLocals);
                addBytes(&bytecode,loop_start);
                bytes_done += 9;
            }
            else
            {
                zbytearr_push(&bytecode,GOTO);;
                addBytes(&bytecode,loop_start);
                bytes_done += 5;
            }
        
            int32_t brk_target = bytes_done;
            zbytearr_push(&bytecode,NPOP_STACK);
            addBytes(&bytecode,3);
            bytes_done += 5;
                        
            //backpatching             
            for(size_t i=breakIdxSizeCopy;i < breakIdx.size;i++)
            {
                pair_vector_push(&backpatches,breakIdx.arr[i],brk_target);
            }
            for(size_t i = contIdxSizeCopy; i< contIdx.size; i++)
            {
                size_t k = contIdx.arr[i];
                int32_t x = m.size - 1;
                memcpy(bytecode.arr+k-4,&x,sizeof(int32_t));
                pair_vector_push(&backpatches,contIdx.arr[i],cont_target);
            }
            pair_vector_push(&backpatches,goto1_offset_idx,brk_target);
            //backtrack
            breakIdx.size = breakIdxSizeCopy;
            contIdx.size  = contIdxSizeCopy;
            STACK_SIZE -= 3;
            symtable_destroy(&m);
        }
        else if(ast->type == NodeType::NAMESPACE)
        {
            const char* name = ast->childs.arr[1]->val;
            char* prefix;
            for(size_t i = 0;i<prefixes.size;i++)
            {
                if(i == 0)
                    prefix = merge_str("",prefixes.arr[i]);
                else
                {
                    char* old = prefix;
                    prefix = merge_str(prefix,prefixes.arr[i]);
                    free(old);
                }
            }
            char* old = prefix;
            prefix = merge_str(prefix,name);
            char* tmp = merge_str(prefix,"::");
            free(old);
            free(prefix);
            str_vector_push(&prefixes,tmp);
            compile(ast->childs.arr[2]);
            char* t;
            str_vector_pop(&prefixes,&t);
            //Hasta La Vista Baby
        }
        else if (ast->type == NodeType::IF)
        {
            int32_t offset1_idx;

            /*if(ast->childs.arr[1]->childs.arr[0]->type == NodeType::equal)
            {
                Node* cond = ast->childs.arr[1]->childs.arr[0];
                expr_bytecode(cond->childs.arr[0]);
                expr_bytecode(cond->childs.arr[1]);
                zbytearr_push(&bytecode,CMP_JMPIFFALSE);
                bytes_done+=1;
                zbytearr_push(&bytecode,0);
                jmp_offset = ++bytes_done;
                addBytes(&bytecode, 0);
                bytes_done += 4;
                
            }
            else
            {*/
                expr_bytecode(ast->childs.arr[1]->childs.arr[0]); // load condition
                zbytearr_push(&bytecode,GOTOIFFALSE);
                offset1_idx = ++bytes_done;
                addBytes(&bytecode,0);
                bytes_done += 4;
            //}

            int32_t before = STACK_SIZE;
            symtable m;
            symtable_init(&m);
            ptr_vector_push(&locals,&m);
            size_t block_size = compile(ast->childs.arr[2]); // block
            STACK_SIZE = before;
            bool hasLocals = (m.size!=0);
            if(hasLocals)
            {
                zbytearr_push(&bytecode,NPOP_STACK);
                foo = m.size;
                addBytes(&bytecode,foo);
                bytes_done += 5;
            }
            locals.size--;
            pair_vector_push(&backpatches,offset1_idx,bytes_done);
            symtable_destroy(&m);
        }
        else if (ast->type == NodeType::IFELSE)
        {
            size_t offset1_idx;
            size_t offset2_idx;
            size_t stack_size_before;
            size_t else_offset;
            symtable m;

            expr_bytecode(ast->childs.arr[1]->childs.arr[0]); //bytecode to load if condition
            offset1_idx = bytes_done+1;
            zbytearr_push(&bytecode,GOTOIFFALSE);
            addBytes(&bytecode,0);
            bytes_done += 1 + JUMPOFFSET_SIZE;
            int32_t before = STACK_SIZE;
            symtable_init(&m);
            ptr_vector_push(&locals,&m);
            size_t ifblock_size = compile(ast->childs.arr[2]);
            STACK_SIZE = before;
            //
            int32_t iflocals = m.size;
            locals.size--;
            if(iflocals != 0)
            {
                zbytearr_push(&bytecode,GOTONPSTACK);      
                addBytes(&bytecode,iflocals);
                offset2_idx = bytes_done+5;
                addBytes(&bytecode,0);
                bytes_done += 9;
            }          
            else
            {
                zbytearr_push(&bytecode,GOTO);
                offset2_idx = bytes_done + 1;
                addBytes(&bytecode,0);
                bytes_done += 5;
            }

            else_offset = bytes_done;
            before = STACK_SIZE;
            symtable_clear(&m);
            ptr_vector_push(&locals,&m);
            size_t elseblock_size = compile(ast->childs.arr[3]);
            STACK_SIZE = before;
            int32_t elseLocals = m.size;
            locals.size--;
            if(elseLocals!=0)
            {
                zbytearr_push(&bytecode,NPOP_STACK);
                addBytes(&bytecode,elseLocals);
                bytes_done += 5;
            }
            pair_vector_push(&backpatches,offset1_idx,else_offset);
            pair_vector_push(&backpatches,offset2_idx,bytes_done);
            symtable_destroy(&m);
        }
        else if (ast->type == NodeType::IFELIFELSE)
        {  
            size_t offset1_idx;
            size_t offset2_idx;
            size_t after_else_offset;
            expr_bytecode(ast->childs.arr[1]->childs.arr[0]); //load if condition
            zbytearr_push(&bytecode,GOTOIFFALSE);
            offset1_idx = bytes_done + 1;
            addBytes(&bytecode,0);
            bytes_done += 1 + JUMPOFFSET_SIZE;
            
            symtable m;
            symtable_init(&m);
            ptr_vector_push(&locals,&m);
            int32_t before = STACK_SIZE;
            compile(ast->childs.arr[2]); // compile if block
            STACK_SIZE = before;
            int32_t iflocals = m.size;
            locals.size--;
            
            if(iflocals != 0)
            {
                zbytearr_push(&bytecode,GOTONPSTACK);
                addBytes(&bytecode,iflocals);
                offset2_idx = bytes_done + 5;
                addBytes(&bytecode,0);
                bytes_done += 9;
            }
            else
            {
                zbytearr_push(&bytecode,GOTO);
                offset2_idx = bytes_done+1;
                addBytes(&bytecode,0);
                bytes_done += 5;
            }

            pair_vector_push(&backpatches,offset1_idx,bytes_done);
            int32_t elifBlockcounter = 3;//third node of ast
            sizet_vector the_end;
            sizet_vector_init(&the_end);
            for (size_t k = 1; k < ast->childs.arr[1]->childs.size; k += 1)
            {
                size_t goto1_offset_idx;
                size_t goto2_offset_idx;

                expr_bytecode(ast->childs.arr[1]->childs.arr[k]); // load elif condition
                zbytearr_push(&bytecode,GOTOIFFALSE);
                goto1_offset_idx = bytes_done + 1;
                addBytes(&bytecode,0);
                bytes_done += 1 + JUMPOFFSET_SIZE;
                before = STACK_SIZE;
                symtable_clear(&m);
                ptr_vector_push(&locals,&m);
                compile(ast->childs.arr[elifBlockcounter]); // compile elif block
                STACK_SIZE = before;
                int32_t eliflocals = m.size;
                locals.size--;
                
                if(eliflocals != 0)
                {
                    zbytearr_push(&bytecode,GOTONPSTACK);
                    addBytes(&bytecode,eliflocals);
                    goto2_offset_idx = bytes_done + 5;
                    addBytes(&bytecode,0);
                    bytes_done += 9;
                }
                else
                {
                    zbytearr_push(&bytecode,GOTO);
                    goto2_offset_idx = bytes_done+1;
                    addBytes(&bytecode, 0);
                    bytes_done += 5;
                }
                pair_vector_push(&backpatches,goto1_offset_idx,bytes_done);
                sizet_vector_push(&the_end,goto2_offset_idx);
                elifBlockcounter += 1;
            }

            symtable_clear(&m);
            ptr_vector_push(&locals,&m);
            before = STACK_SIZE;
            compile(ast->childs.arr[ast->childs.size - 2]); // compile else block
            STACK_SIZE = before;
            size_t elseLocals = m.size;
            locals.size--;
            if(elseLocals!=0)
            {
                zbytearr_push(&bytecode,NPOP_STACK);
                addBytes(&bytecode,elseLocals);
                bytes_done+=5;
            }
            for(size_t i=0;i<the_end.size;i++)
                pair_vector_push(&backpatches,the_end.arr[i],bytes_done);
            pair_vector_push(&backpatches,offset2_idx,bytes_done);
            symtable_destroy(&m);
        }
        else if (ast->type == NodeType::IFELIF)
        {  
            size_t offset1_idx;
            size_t offset2_idx;
            size_t after_else_offset;
            expr_bytecode(ast->childs.arr[1]->childs.arr[0]); //load if condition
            zbytearr_push(&bytecode,GOTOIFFALSE);
            offset1_idx = bytes_done + 1;
            addBytes(&bytecode,0);
            bytes_done += 1 + JUMPOFFSET_SIZE;
            
            symtable m;
            symtable_init(&m);
            ptr_vector_push(&locals,&m);
            int32_t before = STACK_SIZE;
            compile(ast->childs.arr[2]); // compile if block
            STACK_SIZE = before;
            int32_t iflocals = m.size;
            locals.size--;
            
            if(iflocals != 0)
            {
                zbytearr_push(&bytecode,GOTONPSTACK);
                addBytes(&bytecode,iflocals);
                offset2_idx = bytes_done + 5;
                addBytes(&bytecode,0);
                bytes_done += 9;
            }
            else
            {
                zbytearr_push(&bytecode,GOTO);
                offset2_idx = bytes_done+1;
                addBytes(&bytecode,0);
                bytes_done += 5;
            }

            pair_vector_push(&backpatches,offset1_idx,bytes_done);
            int32_t elifBlockcounter = 3;//third node of ast
            sizet_vector the_end;
            sizet_vector_init(&the_end);
            for (size_t k = 1; k < ast->childs.arr[1]->childs.size; k += 1)
            {
                size_t goto1_offset_idx;
                size_t goto2_offset_idx;

                expr_bytecode(ast->childs.arr[1]->childs.arr[k]); // load elif condition
                zbytearr_push(&bytecode,GOTOIFFALSE);
                goto1_offset_idx = bytes_done + 1;
                addBytes(&bytecode,0);
                bytes_done += 1 + JUMPOFFSET_SIZE;
                before = STACK_SIZE;
                symtable_clear(&m);
                ptr_vector_push(&locals,&m);
                compile(ast->childs.arr[elifBlockcounter]); // compile elif block
                STACK_SIZE = before;
                int32_t eliflocals = m.size;
                locals.size--;
                
                if(eliflocals != 0)
                {
                    zbytearr_push(&bytecode,GOTONPSTACK);
                    addBytes(&bytecode,eliflocals);
                    goto2_offset_idx = bytes_done + 5;
                    addBytes(&bytecode,0);
                    bytes_done += 9;
                }
                else if(k != ast->childs.arr[1]->childs.size - 1) // not last elif
                {
                    zbytearr_push(&bytecode,GOTO);
                    goto2_offset_idx = bytes_done+1;
                    addBytes(&bytecode, 0);
                    bytes_done += 5;
                }
                pair_vector_push(&backpatches,goto1_offset_idx,bytes_done);
                if(k != ast->childs.arr[1]->childs.size - 1) // not last elif
                    sizet_vector_push(&the_end,goto2_offset_idx);
                elifBlockcounter += 1;
            }

            for(size_t i=0;i<the_end.size;i++)
                pair_vector_push(&backpatches,the_end.arr[i],bytes_done);
            pair_vector_push(&backpatches,offset2_idx,bytes_done);
            symtable_destroy(&m);
        }
        else if(ast->type==NodeType::TRYCATCH)
        {
            size_t offset1_idx;
            size_t offset2_idx;
            zbytearr_push(&bytecode,ONERR_GOTO);
            addBytes(&bytecode,0);
            offset1_idx = bytes_done+1;
            bytes_done+=5;
            
            int32_t before = STACK_SIZE;
            symtable m;
            symtable_init(&m);
            ptr_vector_push(&locals,&m);
            compile(ast->childs.arr[2]);
            
            STACK_SIZE = before;
            int32_t trylocals = m.size;
            locals.size--;

            zbytearr_push(&bytecode,POP_EXCEP_TARG);
            bytes_done+=1;
            
            if(trylocals!=0)
            {
                zbytearr_push(&bytecode,GOTONPSTACK);
                addBytes(&bytecode,trylocals);
                offset2_idx = bytes_done + 5;
                addBytes(&bytecode,0);
                bytes_done += 9;
            }
            else
            {
                zbytearr_push(&bytecode,GOTO);
                addBytes(&bytecode,0);
                offset2_idx = bytes_done+1;
                bytes_done += 5;
            }

            pair_vector_push(&backpatches,offset1_idx,bytes_done);
            
            before = STACK_SIZE;
            symtable_clear(&m);
            symtable_emplace(&m,ast->childs.arr[1]->val,STACK_SIZE);
            STACK_SIZE+=1;
            ptr_vector_push(&locals,&m);
            compile(ast->childs.arr[3]);
            int32_t catchlocals = m.size;
            STACK_SIZE = before;
            locals.size--;
            if(catchlocals > 1)
            {
                zbytearr_push(&bytecode,NPOP_STACK);
                addBytes(&bytecode,catchlocals);
                bytes_done+=5;
            }
            else
            {
                zbytearr_push(&bytecode,POP_STACK);
                bytes_done+=1;
            }
            pair_vector_push(&backpatches,offset2_idx,bytes_done);
            symtable_destroy(&m);
        }
        else if (ast->type==NodeType::FUNC || (isGen = ast->type == NodeType::CORO))
        {
            int32_t selfIdx= 0;
            if(infunc)
            {
                line_num = atoi(ast->childs.arr[0]->val);
                compileError("SyntaxError","Error function within function not allowed!");
            }
            const char* name = ast->childs.arr[1]->val;
            bool isRef = (str_vector_search(&symRef,name) != -1);
            size_t tmp;
            if(compileAllFuncs || isRef || inclass)
            {
                symtable* last = (locals.size == 0) ? NULL : (symtable*)locals.arr[locals.size-1];
                size_t tmp;
                if(function_exists(name) && !inclass)
                {
                    snprintf(error_buffer,100,"Error redeclaration of builtin function %s",name);
                    compileError("NameError", error_buffer);
                }
                if(locals.size==0 && symtable_get(&globals,name,&tmp))
                {
                    snprintf(error_buffer,100,"Error redeclaration of variable %s",name);
                    compileError("NameError", error_buffer);
                }
                else if(locals.size!=0 && symtable_get(last,name,&tmp))
                {
                    snprintf(error_buffer,100,"Error redeclaration of variable %s",name);
                    compileError("NameError", error_buffer);
                }
                if(!inclass)
                {
                    if(locals.size==0)
                    {
                        symtable_emplace(&globals,name,STACK_SIZE);
                        //if(!isGen)
                        //    compiled_functions.emplace(name,std::pair<Node*,int32_t>(ast,STACK_SIZE));
                    }
                    else
                        symtable_emplace(last,name,STACK_SIZE);
                    STACK_SIZE+=1;
                }
                symtable C;
                symtable_init(&C);
                uint8_t def_param =0;
                vector<uint8_t> expr;
                int32_t before = STACK_SIZE;
                STACK_SIZE = 0;
                if(inclass)
                {
                    symtable_emplace(last,"self",STACK_SIZE);
                    selfIdx = STACK_SIZE;
                    if(strcmp(name,"__construct__")==0)
                        inConstructor = true;
                    STACK_SIZE+=1;
                }
                for (size_t k =0; k<ast->childs.arr[2]->childs.size; k += 1)
                {
                    const char* n = ast->childs.arr[2]->childs.arr[k]->val;
                    if(ast->childs.arr[2]->childs.arr[k]->childs.size!=0)
                    {
                        if(isGen)
                            compileError("SyntaxError","Error default parameters not supported for couroutines");
                        expr_bytecode(ast->childs.arr[2]->childs.arr[k]->childs.arr[0]);
                        def_param +=1;
                    }
                    symtable_emplace(&C,n,STACK_SIZE);
                    STACK_SIZE+=1;
                }
                if(isGen)
                    zbytearr_push(&bytecode,LOAD_CO);
                else
                    zbytearr_push(&bytecode,LOAD_FUNC);
                foo = bytes_done+2+JUMPOFFSET_SIZE+JUMPOFFSET_SIZE+JUMPOFFSET_SIZE+1 +((isGen) ? 0 : 1);
                addBytes(&bytecode,foo);
                foo = add_to_vm_strings(name);
                addBytes(&bytecode,foo);
                if(inclass)
                    zbytearr_push(&bytecode,ast->childs.arr[2]->childs.size+1);
                else
                    zbytearr_push(&bytecode,ast->childs.arr[2]->childs.size);
                //Push number of optional parameters
                if(!isGen)
                {
                    zbytearr_push(&bytecode,def_param);
                    bytes_done++;
                }
                bytes_done+=10;

                /////////
                zbytearr_push(&bytecode,GOTO);
                size_t offset1_idx = bytes_done+1;
                addBytes(&bytecode,0);
                bytes_done += 1 + JUMPOFFSET_SIZE ;
                
                ptr_vector_push(&locals,&C);
                
                infunc = true;
                inGen = isGen;
                returnStmtAtFnScope = false;
                size_t funcBody_size = compile(ast->childs.arr[3]);
                infunc = false;
                
                inConstructor = false;
                inGen = false;
                locals.size--;
                STACK_SIZE = before;
                if(strcmp(name,"__construct__")!=0)
                {
                    if (funcBody_size == 0 || last_stmt_type!=NodeType::RETURN_NODE)
                    {
                        zbytearr_push(&bytecode,LOAD_NIL);
                        if(isGen)
                            zbytearr_push(&bytecode,CO_STOP);
                        else
                            zbytearr_push(&bytecode,OP_RETURN);
                        bytes_done +=2;
                    }
                }
                else
                {
                    zbytearr_push(&bytecode,LOAD_LOCAL);
                    foo = selfIdx;
                    addBytes(&bytecode,foo);
                    zbytearr_push(&bytecode,OP_RETURN);
                    bytes_done+=6;
                }
                pair_vector_push(&backpatches,offset1_idx,bytes_done);
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
            const char* name = ast->childs.arr[1]->val;
            bool isRef = (str_vector_search(&symRef,name) != -1);
            if(!REPL_MODE && !isRef)
            {
                //class not referenced in source code
                //no need to compile
                ast = ast->childs.arr[ast->childs.size-1];
                continue;
            }
            size_t tmp;
            if(symtable_get(&globals,name,&tmp))
            {
                snprintf(error_buffer,100,"Error redeclaration of variable %s",name);
                compileError("NameError", error_buffer);
            }
            symtable_emplace(&globals,name,STACK_SIZE);
            STACK_SIZE+=1;
            scan_class(ast->childs.arr[(extendedClass) ? 3: 2]); // sets classMemb
            if(extendedClass)
            {
                expr_bytecode(ast->childs.arr[2]);
            }
            for(size_t i=0;i<classMemb.size;i++)
            {
                const char* e = classMemb.arr[i];
                if(strcmp(e,"super")==0 || strcmp(e,"self")==0)
                {
                    compileError("NameError","Error class not allowed to have members named \"super\" or \"self\"");
                }
                foo = add_to_vm_strings(e);
                zbytearr_push(&bytecode,LOAD_STR);
                addBytes(&bytecode,foo);
                bytes_done+=5;
            }
            symtable M;
            symtable_init(&M);
            ptr_vector_push(&locals,&M);
            int32_t before = STACK_SIZE;
            
            inclass = true;
            className = name;
            size_t block_size;
            if(extendedClass)
                block_size = compile(ast->childs.arr[3]);
            else
                block_size = compile(ast->childs.arr[2]);
            inclass = false;
            

            locals.size--;
            int32_t totalMembers = classMemb.size;
            classMemb.size = 0;
            STACK_SIZE = before;
            if(extendedClass)
            {
                zbytearr_push(&bytecode,BUILD_DERIVED_CLASS);
                add_lntable_entry(bytes_done);
            }
            else
                zbytearr_push(&bytecode,BUILD_CLASS);

            addBytes(&bytecode,totalMembers);
            foo = add_to_vm_strings(name);
            addBytes(&bytecode,foo);
            bytes_done+=9;
        }
        else if (ast->type == NodeType::RETURN_NODE)
        {
            if(inConstructor)
                compileError("SyntaxError","Error class constructors should not return anything!");
            //stmt in it's block
            if(ast->childs.arr[1]->type == NodeType::NUM && isnum(ast->childs.arr[1]->val))
            {
                zbytearr_push(&bytecode,RETURN_INT32);
                addBytes(&bytecode,(int32_t)atoi(ast->childs.arr[1]->val));
                bytes_done += 5;
            }
            else
            {
                expr_bytecode(ast->childs.arr[1]);
                add_lntable_entry(bytes_done);
                if(inGen)
                    zbytearr_push(&bytecode,CO_STOP);
                else
                    zbytearr_push(&bytecode,OP_RETURN);
                bytes_done += 1;
            }
        }
        else if (ast->type == NodeType::THROW_node)
        {   
            expr_bytecode(ast->childs.arr[1]);
            add_lntable_entry(bytes_done);
            zbytearr_push(&bytecode,THROW);
            bytes_done += 1;
        }
        else if (ast->type == NodeType::YIELD_node)
        {
            if(inConstructor)
                compileError("SyntaxError","Error class constructor can not be generators!");
            
            expr_bytecode(ast->childs.arr[1]);            
            add_lntable_entry(bytes_done);
            zbytearr_push(&bytecode,YIELD);
            bytes_done += 1;
        }
        else if (ast->type == NodeType::BREAK_node)
        {
            zbytearr_push(&bytecode,GOTONPSTACK);
            sizet_vector_push(&breakIdx,bytes_done+5);
            foo = 0;
            for(size_t i=localsBegin;i<locals.size;i++)
                foo+=((symtable*)locals.arr[i])->size;
            if(infor)
                foo-=1; //don't pop loop control variable
            addBytes(&bytecode,foo);
            foo = 0;
            addBytes(&bytecode,foo);
            bytes_done += 9;
        }
        else if(ast->type==NodeType::gc)
        {
            zbytearr_push(&bytecode,GC);
            bytes_done+=1;
        }
        else if (ast->type == NodeType::CONTINUE_node)
        {
            if(infor)
            {
                zbytearr_push(&bytecode,GOTO);
                bytes_done+=1;
                sizet_vector_push(&contIdx,bytes_done);
                addBytes(&bytecode,0);
                bytes_done+=4;
            }
            else
            {
                zbytearr_push(&bytecode,GOTONPSTACK);
                sizet_vector_push(&contIdx,bytes_done+5);
                foo = 0;
                for(size_t i=localsBegin;i<locals.size;i++)
                    foo+=((symtable*)locals.arr[i])->size;
                if(infor)
                    foo -= 1;
                addBytes(&bytecode,foo);
                foo = 0;
                addBytes(&bytecode,foo);
                bytes_done += 9;
            }
        }
        else if (ast->type == NodeType::call)
        {
            const char* name = ast->childs.arr[1]->val;
            bool udf = !function_exists(name);  
            if (udf)
            {
                expr_bytecode(ast);
                zbytearr_push(&bytecode,POP_STACK);
                bytes_done+=1;
            }
            else
            {
                for (size_t k = 0; k < ast->childs.arr[2]->childs.size; k += 1)
                {
                    expr_bytecode(ast->childs.arr[2]->childs.arr[k]);
                }
                add_lntable_entry(bytes_done);
                zbytearr_push(&bytecode,CALL);
                bytes_done += 1;
                size_t index = 0;
                bool add = true;
                BuiltinFunc fn_addr;
                bmap_get(&funcs,name,&fn_addr);
                for(index = 0;index < vm_builtin.size();index+=1)
                {
                    if(vm_builtin[index]==fn_addr)
                    {
                        add = false;
                        break;
                    }
                }
                if(add)
                {
                    vm_builtin.push_back(fn_addr);
                    foo = vm_builtin.size()-1;
                }
                else
                    foo = index;
                addBytes(&bytecode,foo);
                bytes_done += 5;
                zbytearr_push(&bytecode,(char)ast->childs.arr[2]->childs.size);

            }
        }
        else if(ast->type == NodeType::file_node)
        {
            if(infunc)
                compileError("SyntaxError","Error file import inside function!");
            if(inclass)
                compileError("SyntaxError","Error file import inside class!");
                
            const char* C = filename;
            filename = ast->childs.arr[1]->val;
            short K = fileTOP;
            fileTOP = str_vector_search(files,filename);
            bool a = infunc;
            bool b = inclass;
            infunc = false;
            inclass = false;
            size_t size = compile(ast->childs.arr[2]);
            infunc = a;
            inclass = b;
            filename = C;
            fileTOP = K;
        }
        else
        {
            printf("SyntaxError in file %s\n%s\nThe line does not seem to be a meaningful statement!\n", filename, NodeTypeNames[(int)ast->type]);
            exit(1);
        }
        last_stmt_type = ast->type;
        ast = ast->childs.arr[ast->childs.size-1];
    }
    return bytes_done - bytes_done_before;

}
void Compiler::optimize_jmps(zbytearr* bytecode)
{
    for(size_t i=0; i < andJMPS.size; i++)
    {
        size_t e = andJMPS.arr[i];
        int offset;
        memcpy(&offset,bytecode->arr+e+1,4);
        size_t k = e+5+offset;
        int newoffset;
        while(k < bytes_done && bytecode->arr[k] == JMPIFFALSENOPOP)
        {
        memcpy(&newoffset,bytecode->arr+k+1,4);
        offset+=newoffset+5;
        k = k+5+newoffset;
        }
        memcpy(bytecode->arr+e+1,&offset,4);
    }
    //optimize short circuit jumps for or
    for(size_t i=0; i < orJMPS.size; i++)
    {
        size_t e = orJMPS.arr[i];
        int offset;
        memcpy(&offset,bytecode->arr+e+1,4);
        size_t k = e+5+offset;
        int newoffset;
        while(k < bytes_done && bytecode->arr[k] == NOPOPJMPIF)
        {
        memcpy(&newoffset,bytecode->arr+k+1,4);
        offset+=newoffset+5;
        k = k+5+newoffset;
        }
        memcpy(bytecode->arr+e+1,&offset,4);
    }
}
void Compiler::reduceStackTo(int size)//for REPL
{
    while((int)STACK_SIZE > size)
    {
        int lastidx = (int)STACK_SIZE-1;
        int firstidx = (int)size;
        for(size_t i = 0;i < globals.size; i++)
        {
            if(globals.table[i].stat == SYMT_OCCUPIED && globals.table[i].val >= firstidx && globals.table[i].val <= lastidx)
            {
                globals.table[i].stat = SYMT_DELETED;
                STACK_SIZE -= 1;
            }
        }
    }
}
zclass* Compiler::make_error_class(const char* name,zclass* error)
{
    zclass* k = vm_alloc_zclass();
    int32_t idx = add_to_vm_strings(name);
    k->name = name;
    StrMap_assign(&(k->members),&(error->members));
    StrMap_assign(&(k->members),&(error->privateMembers));
    symtable_emplace(&globals,name,STACK_SIZE++);
    zlist_push(&STACK,zobj_from_class(k));
    return k;
}
void Compiler::add_builtin(const char* name,BuiltinFunc fn)
{
    bmap_set(&funcs,name,fn);
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
uint8_t* Compiler::compileProgram(Node* ast,int32_t argc,const char* argv[],int32_t options)//compiles as a complete program adds NPOP_STACK and OP_EXIT
{
    //If prev vector is empty then this program will be compiled as an independent new one
    //otherwise this program will be an addon to the previous one
    //new = prev +curr
    bytes_done = bytecode.size;
    compileAllFuncs = (options & OPT_COMPILE_DEADCODE);
    line_num = 1;
    andJMPS.size = 0;
    orJMPS.size = 0;
    backpatches.size = 0;
    if(bytecode.size == 0)
    {
        symtable_emplace(&globals,"argv",0);
        symtable_emplace(&globals,"stdin",1);
        symtable_emplace(&globals,"stdout",2);
        symtable_emplace(&globals,"stderr",3);
        STACK.size = 0;
        zlist_push(&STACK,make_argv_list(argc,argv));
        zlist_push(&STACK,make_zfile(stdin));
        zlist_push(&STACK,make_zfile(stdout));
        zlist_push(&STACK,make_zfile(stderr));
        STACK_SIZE = STACK.size;
        
        add_to_vm_strings("msg");
        
        zobject nil;
        nil.type = Z_NIL;

        Error = vm_alloc_zclass();
        Error->name = "Error";
        StrMap_emplace(&(Error->members),"msg",nil);
        //Any class inheriting from Error will have 'msg'
        symtable_emplace(&globals,"Error",STACK_SIZE);
        add_to_vm_strings("__construct__");
        zfun* fun = vm_alloc_zfun();
        fun->name = "__construct__";
        fun->args = 2;
        fun->i = 5;
        fun->_klass = NULL;
        ptr_vector_push(&vm_important,fun);

        zbytearr_push(&bytecode,JMP);
        addBytes(&bytecode,21);
        zbytearr_push(&bytecode,LOAD_LOCAL);
        addBytes(&bytecode,0);
        zbytearr_push(&bytecode,LOAD_LOCAL);
        addBytes(&bytecode,1);
        zbytearr_push(&bytecode,ASSIGNMEMB);
        addBytes(&bytecode,0);
        zbytearr_push(&bytecode,LOAD_LOCAL);
        addBytes(&bytecode,0);
        zbytearr_push(&bytecode,OP_RETURN);

        bytes_done+=26;
        zobject construct;
        construct.type = Z_FUNC;
        construct.ptr = (void*)fun;
        StrMap_emplace(&(Error->members),"__construct__",construct);
        zlist_push(&STACK,zobj_from_class(Error));
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

    compile(ast);
    bool popGlobals = (options & OPT_POP_GLOBALS);
    if(globals.size!=0 && popGlobals)
    {
        zbytearr_push(&bytecode,NPOP_STACK);
        foo = globals.size;
        addBytes(&bytecode,foo);
        bytes_done+=5;
    }

    zbytearr_push(&bytecode,OP_EXIT);
    bytes_done+=1;
    if (bytes_done != bytecode.size)
    {
        printf("Zuko encountered an internal error.\nError Code: 10\n");
        printf("%zu %zu\n",bytes_done,bytecode.size);
        exit(EXIT_FAILURE);
    }
    //final phase
    //apply backpatches
    for(size_t i=0;i<backpatches.size;i++)
    {
        zpair p = backpatches.arr[i];
        memcpy(bytecode.arr+p.x,&p.y,sizeof(int32_t));
    }
    //optimize short circuit jumps for 'and' and 'or' operators
    optimize_jmps(&bytecode);
    return bytecode.arr;
}

void Compiler::destroy()
{
    sizet_vector_destroy(&andJMPS);
    sizet_vector_destroy(&orJMPS);
    symtable_destroy(&globals);
    ptr_vector_destroy(&locals);
    str_vector_destroy(&classMemb);
    sizet_vector_destroy(&breakIdx);
    sizet_vector_destroy(&contIdx);    
    str_vector_destroy(&prefixes);
    str_vector_destroy(&symRef);
    pair_vector_destroy(&backpatches);
}