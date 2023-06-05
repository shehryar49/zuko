#ifndef COMPILER_H_
#define COMPILER_H_
#include "plutonium.h"
#include "vm.h"
#include "lexer.h"
using namespace std;
#define JUMPOFFSet_Size 4
extern unordered_map<string,BuiltinFunc> funcs;
extern bool REPL_MODE;
void REPL();
class Compiler
{
private:
  int32_t STACK_SIZE = 0;//simulating STACK's size
  int32_t scope = 0;
  size_t line_num = 1;
  vector<size_t> contTargets;
  vector<size_t> breakTargets;
  vector<int32_t> indexOfLastWhileLocals;
  vector<string>* fnReferenced;
  vector<std::unordered_map<string,int32_t>> locals;
  vector<string> prefixes;
  int32_t* num_of_constants;
  string filename;
  vector<string>* files;
  vector<string>* sources;
  short fileTOP;
  unordered_map<size_t,ByteSrc>* LineNumberTable;
  bool compileAllFuncs;
  vector<size_t> andJMPS;
  vector<size_t> orJMPS;
  
public:
  std::unordered_map<string,int32_t> globals;
  size_t bytes_done = 0;
  void init(vector<string>* fnR,int32_t* e,vector<string>* fnames,vector<string>* fsc,unordered_map<size_t,ByteSrc>* ltable,string filename)
  {
    fnReferenced = fnR;
    num_of_constants = e;
    files = fnames;
    sources = fsc;
    fileTOP = (short)(std::find(files->begin(),files->end(),filename) - files->begin());
    LineNumberTable = ltable;
    this->filename = filename;
  }
  void compileError(string type,string msg)
  {
    fprintf(stderr,"\nFile %s\n",filename.c_str());
    fprintf(stderr,"%s at line %ld\n",type.c_str(),line_num);
    auto it = std::find(files->begin(),files->end(),filename);
    size_t i = it-files->begin();
    string& source_code = (*sources)[i];
    size_t l = 1;
    string line = "";
    size_t k = 0;
    while(l<=line_num)
    {
        if(source_code[k]=='\n')
            l+=1;
        else if(l==line_num)
            line+=source_code[k];
        k+=1;
        if(k>=source_code.length())
        {
          break;
        }
    }
    fprintf(stderr,"%s\n",lstrip(line).c_str());
    fprintf(stderr,"%s\n",msg.c_str());
    if(REPL_MODE)
      REPL();
    exit(0);
  }
  int32_t isDuplicateConstant(PltObject x)
  {
      for (int32_t k = 0; k < vm.total_constants; k += 1)
      {
          if (vm.constants[k] == x)
              return k;
      }
      return -1;
  }

  vector<uint8_t> exprByteCode(Node* ast)
  {
      PltObject reg;
      vector<uint8_t> bytes;
      if (ast->childs.size() == 0)
      {
          if (ast->val.substr(0, 5) == "num: ")
          {
              string n = ast->val.substr(ast->val.find(':') + 2);
              if (isnum(n))
              {
                  reg.type = 'i';
                  reg.i = Int(n);
                  int32_t e = isDuplicateConstant(reg);
                  if (e != -1)
                      FOO.x = e;
                  else
                  {
                      FOO.x = vm.total_constants;
                      reg.type = 'i';
                      reg.i = Int(n);
                      vm.constants[vm.total_constants++] = reg;
                  }
                  bytes.push_back(LOAD_CONST);
                  bytes.push_back(FOO.bytes[0]);
                  bytes.push_back(FOO.bytes[1]);
                  bytes.push_back(FOO.bytes[2]);
                  bytes.push_back(FOO.bytes[3]);
                  bytes_done += 1 + sizeof(int32_t);
              }
              else if (isInt64(n))
              {
                  reg.type = 'l';
                  reg.l = toInt64(n);
                  int32_t e = isDuplicateConstant(reg);
                  if (e != -1)
                      FOO.x = e;
                  else
                  {
                      FOO.x = vm.total_constants;
                      reg.type = 'l';
                      reg.l = toInt64(n);
                      vm.constants[vm.total_constants++] = reg;
                  }
                  bytes.push_back(LOAD_CONST);
                  bytes.push_back(FOO.bytes[0]);
                  bytes.push_back(FOO.bytes[1]);
                  bytes.push_back(FOO.bytes[2]);
                  bytes.push_back(FOO.bytes[3]);
                  bytes_done += 1 + sizeof(int32_t);
              }
              else
              {
                  compileError("OverflowError", "Error integer "+n+" causes overflow!");
              }
              return bytes;
          }
          else if (substr(0, 6, ast->val) == "float: ")
          {
              string n = ast->val.substr(7);
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
              int32_t e = isDuplicateConstant(reg);
              if (e != -1)
                  FOO.x = e;
              else
              {
                  FOO.x = vm.total_constants;

                  vm.constants[vm.total_constants++] = reg;
              }
              bytes.push_back(LOAD_CONST);
              bytes.push_back(FOO.bytes[0]);
              bytes.push_back(FOO.bytes[1]);
              bytes.push_back(FOO.bytes[2]);
              bytes.push_back(FOO.bytes[3]);
              bytes_done += 1 + sizeof(int32_t);
              return bytes;
          }
          else if (ast->val.substr(0, 6) == "bool: ")
          {
              bytes.push_back(LOAD_CONST);
              string n = ast->val.substr(ast->val.find(':') + 2);
              bool a = (n == "true") ? true : false;
              reg.type = 'b';
              reg.i = a;
              int32_t e = isDuplicateConstant(reg);
              if (e != -1)
                  FOO.x = e;
              else
              {
                  FOO.x = vm.total_constants;
                  vm.constants[vm.total_constants++] = reg;
              }
              bytes.push_back(FOO.bytes[0]);
              bytes.push_back(FOO.bytes[1]);
              bytes.push_back(FOO.bytes[2]);
              bytes.push_back(FOO.bytes[3]);
              bytes_done += 1 + sizeof(int32_t);
              return bytes;
          }
          else if (ast->val.substr(0, 8) == "string: ")
          {
              bytes.push_back(LOAD_STR);
              string n = ast->val.substr(ast->val.find(':') + 2);
              auto it = std::find(vm.strings.begin(),vm.strings.end(),n);
              if(it==vm.strings.end())
              {
                 vm.strings.push_back(n);
                 FOO.x = vm.strings.size()-1;
              }
              else
                FOO.x = it - vm.strings.begin();
              bytes.push_back(FOO.bytes[0]);
              bytes.push_back(FOO.bytes[1]);
              bytes.push_back(FOO.bytes[2]);
              bytes.push_back(FOO.bytes[3]);
              bytes_done += 5;
              return bytes;
          }
          else if (substr(0, 8, ast->val) == "keyword: ")
          {
              if (ast->val.substr(9) != "nil")
                  compileError("SyntaxError", "Invalid Syntax");
              bytes.push_back(LOAD_CONST);
              PltObject ret;
              FOO.x = 0;
              bytes.push_back(FOO.bytes[0]);
              bytes.push_back(FOO.bytes[1]);
              bytes.push_back(FOO.bytes[2]);
              bytes.push_back(FOO.bytes[3]);
              bytes_done += 5;
              return bytes;
          }
          else if (ast->val.substr(0, 4) == "id: ")
          {
              string name = ast->val.substr(4);
              bool isGlobal = false;
              FOO.x = resolveName(name,isGlobal);
              if(!isGlobal)
              {
                bytes.push_back(LOAD_LOCAL);
                bytes.push_back(FOO.bytes[0]);
                bytes.push_back(FOO.bytes[1]);
                bytes.push_back(FOO.bytes[2]);
                bytes.push_back(FOO.bytes[3]);
                bytes_done+=5;
                return bytes;
              }
              else
              {
                bytes.push_back(LOAD_GLOBAL);
                bytes.push_back(FOO.bytes[0]);
                bytes.push_back(FOO.bytes[1]);
                bytes.push_back(FOO.bytes[2]);
                bytes.push_back(FOO.bytes[3]);
                bytes_done += 5;
                return bytes;
              }
          }
          else if (ast->val.substr(0, 6) == "byte: ")
          {
              bytes.push_back(LOAD_CONST);
              uint8_t b = tobyte(ast->val.substr(6));
              reg.type = 'm';
              reg.i = b;
              int32_t e = isDuplicateConstant(reg);
              if (e != -1)
                  FOO.x = e;
              else
              {
                  FOO.x = vm.total_constants;
                  vm.constants[vm.total_constants++] = reg;
              }
              bytes.push_back(FOO.bytes[0]);
              bytes.push_back(FOO.bytes[1]);
              bytes.push_back(FOO.bytes[2]);
              bytes.push_back(FOO.bytes[3]);
              bytes_done += 1 + sizeof(int32_t);
              return bytes;
          }

      }
      if (ast->val == "list")
      {
          
          for (size_t k = 0; k < ast->childs.size(); k += 1)
          {
              vector<uint8_t> elem = exprByteCode(ast->childs[k]);
              bytes.insert(bytes.end(), elem.begin(), elem.end());
          }
          bytes.push_back(LOAD);
          bytes.push_back('j');
          FOO.x = ast->childs.size();
          bytes.push_back(FOO.bytes[0]);
          bytes.push_back(FOO.bytes[1]);
          bytes.push_back(FOO.bytes[2]);
          bytes.push_back(FOO.bytes[3]);
          bytes_done += 2 + 4;
          return bytes;
      }
      if (ast->val == "dict")
      {
          
          for (size_t k = 0; k < ast->childs.size(); k += 1)
          {
              vector<uint8_t> elem = exprByteCode(ast->childs[k]);
              bytes.insert(bytes.end(), elem.begin(), elem.end());
          }
          ByteSrc tmp = { fileTOP,line_num };
          LineNumberTable->emplace(bytes_done, tmp);//runtime errors can occur
          bytes.push_back(LOAD);
          bytes.push_back('a');
          FOO.x = ast->childs.size() / 2;//number of key value pairs in dictionary
          bytes.push_back(FOO.bytes[0]);
          bytes.push_back(FOO.bytes[1]);
          bytes.push_back(FOO.bytes[2]);
          bytes.push_back(FOO.bytes[3]);
          bytes_done += 2 + 4;
          return bytes;
      }

      if (ast->val == "+")
      {
          
          vector<uint8_t> a = exprByteCode(ast->childs[0]);
          bytes.insert(bytes.end(), a.begin(), a.end());
          vector<uint8_t> b = exprByteCode(ast->childs[1]);
          bytes.insert(bytes.end(), b.begin(), b.end());
          bytes.push_back(ADD);
          
          bytes_done += 1;
          ByteSrc tmp = { fileTOP,line_num };
          LineNumberTable->emplace(bytes_done - 1, tmp);
          return bytes;
      }
      if (ast->val == "-")
      {
          
          vector<uint8_t> a = exprByteCode(ast->childs[0]);
          bytes.insert(bytes.end(), a.begin(), a.end());
          vector<uint8_t> b = exprByteCode(ast->childs[1]);
          bytes.insert(bytes.end(), b.begin(), b.end());
          bytes.push_back(SUB);
          
          bytes_done += 1;
          ByteSrc tmp = { fileTOP,line_num };
          LineNumberTable->emplace(bytes_done - 1, tmp);
          return bytes;
      }
      if (ast->val == "/")
      {
          
          vector<uint8_t> a = exprByteCode(ast->childs[0]);
          bytes.insert(bytes.end(), a.begin(), a.end());
          vector<uint8_t> b = exprByteCode(ast->childs[1]);
          bytes.insert(bytes.end(), b.begin(), b.end());
          bytes.push_back(DIV);
          
          bytes_done += 1;
          ByteSrc tmp = { fileTOP,line_num };
          LineNumberTable->emplace(bytes_done - 1, tmp);
          return bytes;
      }
      if (ast->val == "*")
      {
          
          vector<uint8_t> a = exprByteCode(ast->childs[0]);
          bytes.insert(bytes.end(), a.begin(), a.end());
          vector<uint8_t> b = exprByteCode(ast->childs[1]);
          bytes.insert(bytes.end(), b.begin(), b.end());
          bytes.push_back(MUL);
          
          bytes_done += 1;
          ByteSrc tmp = { fileTOP,line_num };
          LineNumberTable->emplace(bytes_done - 1, tmp);
          return bytes;
      }
      if (ast->val == "^")
      {
          
          vector<uint8_t> a = exprByteCode(ast->childs[0]);
          bytes.insert(bytes.end(), a.begin(), a.end());
          vector<uint8_t> b = exprByteCode(ast->childs[1]);
          bytes.insert(bytes.end(), b.begin(), b.end());
          bytes.push_back(XOR);
          
          bytes_done += 1;
          ByteSrc tmp = { fileTOP,line_num };
          LineNumberTable->emplace(bytes_done - 1, tmp);
          return bytes;
      }
      if (ast->val == "%")
      {
          
          vector<uint8_t> a = exprByteCode(ast->childs[0]);
          bytes.insert(bytes.end(), a.begin(), a.end());
          vector<uint8_t> b = exprByteCode(ast->childs[1]);
          bytes.insert(bytes.end(), b.begin(), b.end());
          bytes.push_back(MOD);
          
          bytes_done += 1;
          ByteSrc tmp = { fileTOP,line_num };
          LineNumberTable->emplace(bytes_done - 1, tmp);
          return bytes;
      }
      if (ast->val == "<<")
      {
          
          vector<uint8_t> a = exprByteCode(ast->childs[0]);
          bytes.insert(bytes.end(), a.begin(), a.end());
          vector<uint8_t> b = exprByteCode(ast->childs[1]);
          bytes.insert(bytes.end(), b.begin(), b.end());
          bytes.push_back(LSHIFT);
          
          bytes_done += 1;
          ByteSrc tmp = { fileTOP,line_num };
          LineNumberTable->emplace(bytes_done - 1, tmp);
          return bytes;
      }
      if (ast->val == ">>")
      {
          
          vector<uint8_t> a = exprByteCode(ast->childs[0]);
          bytes.insert(bytes.end(), a.begin(), a.end());
          vector<uint8_t> b = exprByteCode(ast->childs[1]);
          bytes.insert(bytes.end(), b.begin(), b.end());
          bytes.push_back(RSHIFT);
          
          bytes_done += 1;
          ByteSrc tmp = { fileTOP,line_num };
          LineNumberTable->emplace(bytes_done - 1, tmp);
          return bytes;
      }
      if (ast->val == "&")
      {
          
          vector<uint8_t> a = exprByteCode(ast->childs[0]);
          bytes.insert(bytes.end(), a.begin(), a.end());
          vector<uint8_t> b = exprByteCode(ast->childs[1]);
          bytes.insert(bytes.end(), b.begin(), b.end());
          bytes.push_back(BITWISE_AND);
          
          bytes_done += 1;
          ByteSrc tmp = { fileTOP,line_num };
          LineNumberTable->emplace(bytes_done - 1, tmp);
          return bytes;
      }

      if (ast->val == "|")
      {
          
          vector<uint8_t> a = exprByteCode(ast->childs[0]);
          bytes.insert(bytes.end(), a.begin(), a.end());
          vector<uint8_t> b = exprByteCode(ast->childs[1]);
          bytes.insert(bytes.end(), b.begin(), b.end());
          bytes.push_back(BITWISE_OR);
          
          bytes_done += 1;
          ByteSrc tmp = { fileTOP,line_num };
          LineNumberTable->emplace(bytes_done - 1, tmp);
          return bytes;
      }
      if (ast->val == ".")
      {
          
          if (ast->childs[1]->val == "call")
          {
              Node* callnode = ast->childs[1];

              vector<uint8_t> a = exprByteCode(ast->childs[0]);
              bytes.insert(bytes.end(), a.begin(), a.end());
              Node* args = callnode->childs[2];
              for (size_t f = 0; f < args->childs.size(); f += 1)
              {
                  vector<uint8_t> arg = exprByteCode(args->childs[f]);
                  bytes.insert(bytes.end(), arg.begin(), arg.end());
              }
                bytes.push_back(CALLMETHOD);
              ByteSrc tmp = { fileTOP,line_num };
              LineNumberTable->emplace(bytes_done, tmp);
              string memberName = callnode->childs[1]->val;

              auto it = std::find(vm.strings.begin(),vm.strings.end(),memberName);
              if(it==vm.strings.end())
              {
                 vm.strings.push_back(memberName);
                 FOO.x = vm.strings.size()-1;
              }
              else
                FOO.x = it - vm.strings.begin();
              bytes.push_back(FOO.bytes[0]);
              bytes.push_back(FOO.bytes[1]);
              bytes.push_back(FOO.bytes[2]);
              bytes.push_back(FOO.bytes[3]);
              bytes.push_back(args->childs.size());
              
              bytes_done +=6;

              return bytes;
          }
          else
          {
              vector<uint8_t> a = exprByteCode(ast->childs[0]);
              bytes.insert(bytes.end(), a.begin(), a.end());
              ByteSrc tmp = { fileTOP,line_num };
              LineNumberTable->emplace(bytes_done, tmp);
              bytes.push_back(MEMB);
              if (substr(0, 3, ast->childs[1]->val) != "id: ")
                  compileError("SyntaxError", "Invalid Syntax");
              string name = ast->childs[1]->val;
              name = name.substr(4);
          
              auto it = std::find(vm.strings.begin(),vm.strings.end(),name);
              if(it==vm.strings.end())
              {
                 vm.strings.push_back(name);
                 FOO.x = vm.strings.size()-1;
              }
              else
                FOO.x = it - vm.strings.begin();
              bytes.push_back(FOO.bytes[0]);
              bytes.push_back(FOO.bytes[1]);
              bytes.push_back(FOO.bytes[2]);
              bytes.push_back(FOO.bytes[3]);
              
              bytes_done +=5;
              return bytes;
          }
      }
      if (ast->val == "and")
      {
          vector<uint8_t> a = exprByteCode(ast->childs[0]);
          bytes.insert(bytes.end(), a.begin(), a.end());

          bytes.push_back(JMPIFFALSENOPOP);
          andJMPS.push_back(bytes_done);
       //   printf("added andJMP at %ld\n",bytes_done);
          int32_t I = bytes.size();
          bytes.push_back(0);
          bytes.push_back(0);
          bytes.push_back(0);
          bytes.push_back(0);
          bytes_done+=5;
          vector<uint8_t> b = exprByteCode(ast->childs[1]);
          bytes.insert(bytes.end(), b.begin(), b.end());
        //  bytes.push_back(AND);
        //  bytes_done += 1;
         // ByteSrc tmp = { fileTOP,line_num };
        //  LineNumberTable->emplace(bytes_done - 1, tmp);
          FOO.x = b.size();
          bytes[I] = FOO.bytes[0];
          bytes[I+1] = FOO.bytes[1];        
          bytes[I+2] = FOO.bytes[2];
          bytes[I+3] = FOO.bytes[3];
          return bytes;
      }
      if (ast->val == "is")
      {
          
          vector<uint8_t> a = exprByteCode(ast->childs[0]);
          bytes.insert(bytes.end(), a.begin(), a.end());
          vector<uint8_t> b = exprByteCode(ast->childs[1]);
          bytes.insert(bytes.end(), b.begin(), b.end());
          bytes.push_back(IS);
          
          bytes_done += 1;
          ByteSrc tmp = { fileTOP,line_num };
          LineNumberTable->emplace(bytes_done - 1, tmp);
          return bytes;
      }
      if (ast->val == "or")
      {
          
          vector<uint8_t> a = exprByteCode(ast->childs[0]);
          bytes.insert(bytes.end(), a.begin(), a.end());
          bytes.push_back(NOPOPJMPIF);
          orJMPS.push_back(bytes_done);
          int32_t I = bytes.size();
          bytes.push_back(0);
          bytes.push_back(0);
          bytes.push_back(0);
          bytes.push_back(0);
          bytes_done+=5;
          vector<uint8_t> b = exprByteCode(ast->childs[1]);
          bytes.insert(bytes.end(), b.begin(), b.end());

          FOO.x = b.size();
          bytes[I] = FOO.bytes[0];
          bytes[I+1] = FOO.bytes[1];        
          bytes[I+2] = FOO.bytes[2];
          bytes[I+3] = FOO.bytes[3];
          return bytes;
      }
      if (ast->val == "<")
      {
          
          vector<uint8_t> a = exprByteCode(ast->childs[0]);
          bytes.insert(bytes.end(), a.begin(), a.end());
          vector<uint8_t> b = exprByteCode(ast->childs[1]);
          bytes.insert(bytes.end(), b.begin(), b.end());
          bytes.push_back(SMALLERTHAN);
          
          bytes_done += 1;
          ByteSrc tmp = { fileTOP,line_num };
          LineNumberTable->emplace(bytes_done - 1, tmp);
          return bytes;
      }
      if (ast->val == ">")
      {
          
          vector<uint8_t> a = exprByteCode(ast->childs[0]);
          bytes.insert(bytes.end(), a.begin(), a.end());
          vector<uint8_t> b = exprByteCode(ast->childs[1]);
          bytes.insert(bytes.end(), b.begin(), b.end());
          bytes.push_back(GREATERTHAN);
          
          bytes_done += 1;
          ByteSrc tmp = { fileTOP,line_num };
          LineNumberTable->emplace(bytes_done - 1, tmp);
          return bytes;
      }
      if (ast->val == "==")
      {
          
          vector<uint8_t> a = exprByteCode(ast->childs[0]);
          bytes.insert(bytes.end(), a.begin(), a.end());
          vector<uint8_t> b = exprByteCode(ast->childs[1]);
          bytes.insert(bytes.end(), b.begin(), b.end());
          bytes.push_back(EQ);
          
          bytes_done += 1;
          ByteSrc tmp = { fileTOP,line_num };
          LineNumberTable->emplace(bytes_done - 1, tmp);
          return bytes;
      }
      if (ast->val == "!=")
      {
          
          vector<uint8_t> a = exprByteCode(ast->childs[0]);
          bytes.insert(bytes.end(), a.begin(), a.end());
          vector<uint8_t> b = exprByteCode(ast->childs[1]);
          bytes.insert(bytes.end(), b.begin(), b.end());
          bytes.push_back(NOTEQ);
          
          bytes_done += 1;
          ByteSrc tmp = { fileTOP,line_num };
          LineNumberTable->emplace(bytes_done - 1, tmp);
          return bytes;
      }
      if (ast->val == ">=")
      {
          
          vector<uint8_t> a = exprByteCode(ast->childs[0]);
          bytes.insert(bytes.end(), a.begin(), a.end());
          vector<uint8_t> b = exprByteCode(ast->childs[1]);
          bytes.insert(bytes.end(), b.begin(), b.end());
          bytes.push_back(GROREQ);
          
          bytes_done += 1;
          ByteSrc tmp = { fileTOP,line_num };
          LineNumberTable->emplace(bytes_done - 1, tmp);
          return bytes;
      }
      if (ast->val == "<=")
      {
          
          vector<uint8_t> a = exprByteCode(ast->childs[0]);
          bytes.insert(bytes.end(), a.begin(), a.end());
          vector<uint8_t> b = exprByteCode(ast->childs[1]);
          bytes.insert(bytes.end(), b.begin(), b.end());
          bytes.push_back(SMOREQ);
          
          bytes_done += 1;
          ByteSrc tmp = { fileTOP,line_num };
          LineNumberTable->emplace(bytes_done - 1, tmp);
          return bytes;
      }
      if (ast->val == "neg")
      {
          
          vector<uint8_t> val = exprByteCode(ast->childs[0]);
          bytes.insert(bytes.end(), val.begin(), val.end());
          bytes.push_back(NEG);
          
          bytes_done += 1;
          ByteSrc tmp = { fileTOP,line_num };
          LineNumberTable->emplace(bytes_done - 1, tmp);
          return bytes;
      }
      if (ast->val == "~")
      {
          
          vector<uint8_t> val = exprByteCode(ast->childs[0]);
          bytes.insert(bytes.end(), val.begin(), val.end());
          bytes.push_back(COMPLEMENT);
          
          bytes_done += 1;
          ByteSrc tmp = { fileTOP,line_num };
          LineNumberTable->emplace(bytes_done - 1, tmp);
          return bytes;
      }
      if (ast->val == "!")
      {
          
          vector<uint8_t> val = exprByteCode(ast->childs[0]);
          bytes.insert(bytes.end(), val.begin(), val.end());
          bytes.push_back(NOT);
          
          bytes_done += 1;
          ByteSrc tmp = { fileTOP,line_num };
          LineNumberTable->emplace(bytes_done - 1, tmp);
          return bytes;
      }
      if (ast->val == "index")
      {
        
        vector<uint8_t> a = exprByteCode(ast->childs[0]);
        bytes.insert(bytes.end(), a.begin(), a.end());
        vector<uint8_t> b = exprByteCode(ast->childs[1]);
        bytes.insert(bytes.end(), b.begin(), b.end());
        bytes.push_back(INDEX);
        
        bytes_done += 1;
        ByteSrc tmp = { fileTOP,line_num };
        LineNumberTable->emplace(bytes_done - 1, tmp);
        return bytes;
      }
      if (ast->val.substr(0, 4) == "call")
      {
          string name = ast->childs[1]->val;
          string nameToSet;

          
          bool udf = false;
          if (funcs.find(name) != funcs.end())
          {

          }
          else
          {
            udf = true;
          }
          if (udf)
          {
              for (size_t k = 0; k < ast->childs[2]->childs.size(); k += 1)
              {
                  vector<uint8_t> val = exprByteCode(ast->childs[2]->childs[k]);
                  bytes.insert(bytes.end(), val.begin(), val.end());
              }
              ByteSrc tmp = { fileTOP,line_num };
              Node* E = NewNode("id: "+name);
              vector<uint8_t> fn = exprByteCode(E);
              delete E;
              bytes.insert(bytes.end(),fn.begin(),fn.end());
              LineNumberTable->emplace(bytes_done, tmp);

              bytes.push_back(CALLUDF);
              bytes.push_back(ast->childs[2]->childs.size());
              bytes_done+=2;
              return bytes;
          }
          else
          {
              for (size_t k = 0; k < ast->childs[2]->childs.size(); k += 1)
              {
                  vector<uint8_t> val = exprByteCode(ast->childs[2]->childs[k]);
                  bytes.insert(bytes.end(), val.begin(), val.end());
              }
              ByteSrc tmp = { fileTOP,line_num };
              LineNumberTable->emplace(bytes_done, tmp);
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
                FOO.x = vm.builtin.size()-1;
              }
              else
                FOO.x = index;
              bytes.push_back(FOO.bytes[0]);
              bytes.push_back(FOO.bytes[1]);
              bytes.push_back(FOO.bytes[2]);
              bytes.push_back(FOO.bytes[3]);
              bytes.push_back((char)ast->childs[2]->childs.size());
              bytes_done += 6;
              return bytes;
          }
      }
      if (ast->val == "yield")
      {
        if(inConstructor)
          compileError("SyntaxError","Error class constructor can not be generators!");
        
        vector<uint8_t> val = exprByteCode(ast->childs[1]);
        bytes.insert(bytes.end(), val.begin(), val.end());              
        ByteSrc tmp = { fileTOP,line_num };
        LineNumberTable->emplace(bytes_done, tmp);
        bytes.push_back(YIELD_AND_EXPECTVAL);
        bytes_done += 1;
        return bytes;
      }  
      compileError("SyntaxError", "Invalid syntax in expression");
      exit(0);
      return bytes;//to prevent compiler warning
  }
  Node* getLastMemberName(Node* ast)
  {
    if(ast->childs[1]->val==".")
      return getLastMemberName(ast->childs[1]);
    if(substr(0,3,ast->childs[1]->val)!="id: ")
      compileError("SyntaxError","Invalid Syntax");
    return ast;
  }
  Node* getLastFnName(Node* ast)
  {
    if(ast->val==".")
      return getLastFnName(ast->childs[1]);
    if(ast->val!="call")
    {
      compileError("SyntaxError","Invalid Syntax");
    }
    return ast;
  }
  vector<string> scanClass(Node* ast)
  {
    vector<string> names;
    while(ast->val!="endclass")
    {
      if(ast->val=="declare")
      {
        string n = ast->childs[1]->val.substr(4);
        if(n[0]=='@')
         n = n.substr(1);
        if(std::find(names.begin(),names.end(),n)!=names.end() || std::find(names.begin(),names.end(),"@"+n)!=names.end())
        {
          line_num = atoi(ast->childs[0]->val.substr(5).c_str());
          compileError("NameError","Error redeclaration of "+n+".");
        }
        names.push_back(ast->childs[1]->val.substr(4));
      }
      else if(substr(0,4,ast->val)=="func ")
      {

        string n = ast->val.substr(5);
        if(n[0]=='@')
         n = n.substr(1);
        if(std::find(names.begin(),names.end(),n)!=names.end() || std::find(names.begin(),names.end(),"@"+n)!=names.end())
        {
          line_num = atoi(ast->childs[0]->val.substr(5).c_str());
          compileError("NameError","Error redeclaration of "+n+".");
        }
        names.push_back(ast->val.substr(5));
      }
      else if(substr(0,3,ast->val)=="gen ") //generator function or coroutine
      {
          line_num = atoi(ast->childs[0]->val.substr(5).c_str());
          compileError("NameError","Error coroutine inside class not allowed.");
      }
      
      else if(ast->val=="class")
      {
        line_num = atoi(ast->childs[0]->val.substr(5).c_str());
        compileError("SyntaxError","Error nested classes not supported");
      
      }
      ast = ast->childs.back();
    }
    return names;
  }
  Node* getLastIndex(Node* ast)
  {
    if(ast->childs[1]->val=="index")
      return getLastMemberName(ast->childs[1]);
    if(substr(0,3,ast->childs[1]->val)!="id: ")
      compileError("SyntaxError","Invalid Syntax");
    return ast;
  }
  int32_t resolveName(string name,bool& isGlobal,bool blowUp=true)
  {
    for(int32_t i=locals.size()-1;i>=0;i-=1)
    {
      if(locals[i].find(name)!=locals[i].end())
        return locals[i][name];
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

    if(globals.find(name) != globals.end())
    {
        isGlobal = true;
        return globals[name];
    }
    else
      if(blowUp)
          compileError("NameError","Error name "+name+" is not defined!");
    return -1;
  }
  bool inConstructor = false;
  bool inGen = false;
  vector<uint8_t> compile(Node* ast, bool infunc,bool inclass=false)
  {
      vector<uint8_t> program;
      bool isGen = false;
      while (ast->val != "EOP" && ast->val!="endclass" && ast->val!="endfor" && ast->val!="endnm" && ast->val!="endtry" && ast->val!="endcatch" && ast->val != "endif" && ast->val != "endfunc" && ast->val != "endelif" && ast->val != "endwhile" && ast->val != "endelse")
      {
          if (substr(0, 4, ast->childs[0]->val) == "line ")
              line_num = Int(ast->childs[0]->val.substr(5));
          if (ast->val == "declare")
          {
              
              vector<uint8_t> val = exprByteCode(ast->childs[2]);
              program.insert(program.end(), val.begin(), val.end());

              string name = ast->childs[1]->val.substr(4);
              if (scope == 0)
              {
                  if (globals.find(name) != globals.end())
                  {
                    compileError("NameError", "Error redeclaration of variable " + name);
                  }
                  FOO.x = STACK_SIZE;
                  if(!inclass)
                    globals.emplace(name,FOO.x);
                  STACK_SIZE+=1;
              }
              else
              {
                  if(locals.back().find(name)!=locals.back().end())
                      compileError("NameError", "Error redeclaration of variable " + name);
                  FOO.x = STACK_SIZE;
                  if(!inclass)
                  locals.back().emplace(name,FOO.x);
                  STACK_SIZE+=1;
              }

          }
          else if (ast->val == "import" || ast->val=="importas")
          {
              string name = ast->childs[1]->val;
              name = name.substr(4);// id:
              string vname = (ast->val=="importas") ? ast->childs[2]->val.substr(4) : name;
              bool f;
              if(resolveName(vname,f,false)!=-1)
              {
                compileError("NameError","Error redeclaration of name "+name);
              }
              ByteSrc tmp = { fileTOP,line_num };
              LineNumberTable->emplace(bytes_done, tmp);
              program.push_back(IMPORT);
              FOO.x = (int32_t)vm.strings.size();
              vm.strings.push_back(name);
              
              program.push_back(FOO.bytes[0]);
              program.push_back(FOO.bytes[1]);
              program.push_back(FOO.bytes[2]);
              program.push_back(FOO.bytes[3]);
              bytes_done += 5;
              if(scope==0)
                globals.emplace(vname,STACK_SIZE);
              else
                locals.back().emplace(vname,STACK_SIZE);
              STACK_SIZE+=1;
          }
          else if (ast->val == "=")
          {
              
              string name = ast->childs[1]->val;
              bool doit = true;
              if (name.substr(0, 4) == "id: ")
              {
                  if (ast->childs[2]->childs.size() == 2)
                  {
                      if (ast->childs[2]->val == "+" && ast->childs[2]->childs[0]->val == ast->childs[1]->val && ast->childs[2]->childs[1]->val == "num: 1")
                      {
                        name = name.substr(4);
                        bool isGlobal = false;
                        int32_t idx = resolveName(name,isGlobal,false);
                        if(idx==-1)
                          compileError("NameError","Error name "+name+" is not defined!");
                        ByteSrc tmp = { fileTOP,line_num };
                        LineNumberTable->emplace(bytes_done, tmp);
                        FOO.x = idx;
                        if(!isGlobal)
                          program.push_back(INPLACE_INC);
                        else
                          program.push_back(INC_GLOBAL);
                        program.push_back(FOO.bytes[0]);
                        program.push_back(FOO.bytes[1]);
                        program.push_back(FOO.bytes[2]);
                        program.push_back(FOO.bytes[3]);
                        bytes_done+=5;
                        doit = false;
                      }
                  }
                  if (doit)
                  {
                      name = name.substr(4);
                      bool isGlobal = false;
                      int32_t idx = resolveName(name,isGlobal,false);
                      if(idx==-1)
                         compileError("NameError","Error name "+name+" is not defined!");
                      vector<uint8_t> val = exprByteCode(ast->childs[2]);
                      program.insert(program.end(), val.begin(), val.end());
                      ByteSrc tmp = { fileTOP,line_num };
                      LineNumberTable->emplace(bytes_done, tmp);
                      FOO.x = idx;
                      if(!isGlobal)
                        program.push_back(ASSIGN);
                      else
                        program.push_back(ASSIGN_GLOBAL);
                      program.push_back(FOO.bytes[0]);
                      program.push_back(FOO.bytes[1]);
                      program.push_back(FOO.bytes[2]);
                      program.push_back(FOO.bytes[3]);
                      bytes_done += 5;
                  }
              }
              else if (name == "index")
              {
                  //reassign index
                  vector<uint8_t> M = exprByteCode(ast->childs[1]->childs[0]);
                  program.insert(program.end(),M.begin(),M.end());
                  M = exprByteCode(ast->childs[2]);
                  program.insert(program.end(),M.begin(),M.end());
                  M = exprByteCode(ast->childs[1]->childs[1]);
                  program.insert(program.end(),M.begin(),M.end());
                  program.push_back(ASSIGNINDEX);
                  ByteSrc tmp = { fileTOP,line_num };
                  LineNumberTable->emplace(bytes_done, tmp);
                  bytes_done+=1;
              }
              else if (name == ".")
              {
                  //reassign object member
                  string rhs = ast->childs[1]->val;
                  vector<uint8_t> lhs = exprByteCode(ast->childs[1]->childs[0]);
                  program.insert(program.end(),lhs.begin(),lhs.end());
                  if(ast->childs[1]->childs[1]->val.substr(0,4)!="id: ")
                    compileError("SyntaxError","Invalid Syntax");
                  string mname = ast->childs[1]->childs[1]->val.substr(4);
                  vector<uint8_t> val = exprByteCode(ast->childs[2]);
                  program.insert(program.end(), val.begin(), val.end());
                  program.push_back(ASSIGNMEMB);
                  for(auto e: mname)
                    program.push_back(e);
                  program.push_back(0);
                  ByteSrc tmp = { fileTOP,line_num };
                  LineNumberTable->emplace(bytes_done, tmp);
                  bytes_done+=mname.length()+2;

              }
          }
          else if (ast->val == ".")
          {
              Node* bitch = NewNode(".");
              bitch->childs.push_back(ast->childs[1]);
              bitch->childs.push_back(ast->childs[2]);
              vector<uint8_t> stmt = exprByteCode(bitch);
              delete bitch;
              program.insert(program.end(),stmt.begin(),stmt.end());
              program.push_back(POP_STACK);
              bytes_done +=1;
          }
          else if (ast->val == "while" || ast->val=="dowhile")
          {
              
              if(ast->val=="dowhile")
                   bytes_done+=5;//do while uses one extra jump before the condition
              contTargets.push_back(bytes_done);
                          size_t L = bytes_done;
              vector<uint8_t> cond = exprByteCode(ast->childs[1]);
              breakTargets.push_back(bytes_done);

              bytes_done += 1 + JUMPOFFSet_Size;//JMPFALSE <4 byte where>

              indexOfLastWhileLocals.push_back(locals.size());
              scope += 1;
              int32_t before = STACK_SIZE;
              std::unordered_map<string,int32_t> m;
              locals.push_back(m);

              vector<uint8_t> block = compile(ast->childs[2], infunc);

              STACK_SIZE = before;
              breakTargets.pop_back();
              contTargets.pop_back();
              scope -= 1;
              indexOfLastWhileLocals.pop_back();
              int32_t whileLocals = locals.back().size();
              locals.pop_back();
              int32_t where = block.size() + 1 + JUMPOFFSet_Size;
              if(whileLocals!=0)
                where+=4;//4 for NPOP_STACK
             if(ast->val=="dowhile")
             {
                program.push_back(JMP);
                FOO.x = cond.size()+ 5;
                program.push_back(FOO.bytes[0]);
                program.push_back(FOO.bytes[1]);
                program.push_back(FOO.bytes[2]);
                program.push_back(FOO.bytes[3]);
              }
              program.insert(program.end(), cond.begin(), cond.end());
              //Check while condition
              program.push_back(JMPIFFALSE);
              FOO.x = where;
              program.push_back(FOO.bytes[0]);
              program.push_back(FOO.bytes[1]);
              program.push_back(FOO.bytes[2]);
              program.push_back(FOO.bytes[3]);
              //
              program.insert(program.end(), block.begin(), block.end());
              if(whileLocals!=0)
              {
                program.push_back(GOTONPSTACK);
                FOO.x = whileLocals;
                program.push_back(FOO.bytes[0]);
                program.push_back(FOO.bytes[1]);
                program.push_back(FOO.bytes[2]);
                program.push_back(FOO.bytes[3]);
                bytes_done += 4;
              }
              else
                program.push_back(GOTO);
              FOO.x = L;

              program.push_back(FOO.bytes[0]);
              program.push_back(FOO.bytes[1]);
              program.push_back(FOO.bytes[2]);
              program.push_back(FOO.bytes[3]);
              bytes_done += 1 + JUMPOFFSet_Size;

          }
          else if (ast->val == "for" )
          {
              
              size_t lnCopy = line_num;
              bool decl = ast->childs[1]->val == "decl";
              size_t L = bytes_done;
              string loop_var_name = ast->childs[2]->val;
              string I = ast->childs[5]->val;
            vector<uint8_t> initValue = exprByteCode(ast->childs[3]);

            program.insert(program.end(),initValue.begin(),initValue.end());
            bool isGlobal = false;
            int32_t lcvIdx ;
              if(!decl)
              {
                string name = loop_var_name;
                FOO.x = resolveName(name,isGlobal);
                lcvIdx = FOO.x;
                if(!isGlobal)
                {
                  program.push_back(ASSIGN);
                  L+=5;
                }
                else
                  program.push_back(ASSIGN_GLOBAL);
                program.push_back(FOO.bytes[0]);
                program.push_back(FOO.bytes[1]);
                program.push_back(FOO.bytes[2]);
                program.push_back(FOO.bytes[3]);
                bytes_done+=5;

              }
              contTargets.push_back(bytes_done);
              bytes_done+=6;
              if(isGlobal)
                bytes_done-=1;
              vector<uint8_t> finalValue = exprByteCode(ast->childs[4]);
              //
              breakTargets.push_back(bytes_done+1);
              int32_t before = STACK_SIZE;
              if(decl)
              {
                scope += 1;
                before = STACK_SIZE+1;
                std::unordered_map<string,int32_t> m;
                scope+=1;
                locals.push_back(m);
                lcvIdx = STACK_SIZE;
                locals.back().emplace(loop_var_name,STACK_SIZE);
                STACK_SIZE+=1;
                indexOfLastWhileLocals.push_back(locals.size());
                locals.push_back(m);
              }
              else
              {
                scope += 1;
                std::unordered_map<string,int32_t> m;
                indexOfLastWhileLocals.push_back(locals.size());
                locals.push_back(m);
              }

              bytes_done+=2+JUMPOFFSet_Size;//for jump before block
              vector<uint8_t> block = compile(ast->childs[6], infunc);

              STACK_SIZE = before;
              breakTargets.pop_back();
              contTargets.pop_back();
              if(decl)
                scope -= 2;
              else
                scope-=1;
              indexOfLastWhileLocals.pop_back();
              int32_t whileLocals = locals.back().size();
              locals.pop_back();
              if(decl)
                locals.pop_back();
              int32_t where;
              if(!isGlobal)
              {
                program.push_back(LOAD);
                program.push_back('v');
              }
              else
                program.push_back(LOAD_GLOBAL);
              FOO.x = lcvIdx;
              program.push_back(FOO.bytes[0]);
              program.push_back(FOO.bytes[1]);
              program.push_back(FOO.bytes[2]);
              program.push_back(FOO.bytes[3]);//load loop control variable
              program.insert(program.end(),finalValue.begin(),finalValue.end());
              program.push_back(SMOREQ);

              ////
              vector<uint8_t> inc;
              line_num = lnCopy;


              if(I!="num: 1")
              {
                inc = exprByteCode(ast->childs[5]);
                where = block.size() + 1 + JUMPOFFSet_Size+inc.size()+12;
                if(isGlobal)
                  where-=1;
              }
              else
              {
                where = block.size() + 1 + JUMPOFFSet_Size+5;

              }
              if(whileLocals!=0)
                where+=4;//4 for NPOP_STACK
              ////
              //Check condition
              program.push_back(JMPIFFALSE);
              FOO.x = where;
              program.push_back(FOO.bytes[0]);
              program.push_back(FOO.bytes[1]);
              program.push_back(FOO.bytes[2]);
              program.push_back(FOO.bytes[3]);
              //
              block.insert(block.end(),inc.begin(),inc.end());
              if(I=="num: 1")
              {
                if(!isGlobal)
                  block.push_back(INPLACE_INC);
                else
                  block.push_back(INC_GLOBAL);
                FOO.x = lcvIdx;
                block.push_back(FOO.bytes[0]);
                block.push_back(FOO.bytes[1]);
                block.push_back(FOO.bytes[2]);
                block.push_back(FOO.bytes[3]);
                bytes_done+=5;
              }
              else
              {
                if(!isGlobal)
                {
                  block.push_back(LOAD);
                  block.push_back('v');
                }
                else
                  block.push_back(LOAD_GLOBAL);
                FOO.x = lcvIdx;
                block.push_back(FOO.bytes[0]);
                block.push_back(FOO.bytes[1]);
                block.push_back(FOO.bytes[2]);
                block.push_back(FOO.bytes[3]);
                block.push_back(ADD);
                if(!isGlobal )
                  block.push_back(ASSIGN);
                else
                  block.push_back(ASSIGN_GLOBAL);
                block.push_back(FOO.bytes[0]);
                block.push_back(FOO.bytes[1]);
                block.push_back(FOO.bytes[2]);
                block.push_back(FOO.bytes[3]);
                if(isGlobal)
                  bytes_done+=11;
                else
                  bytes_done+=12;
              }
              program.insert(program.end(), block.begin(), block.end());
              if(whileLocals!=0)
              {
                program.push_back(GOTONPSTACK);
                FOO.x = whileLocals;
                program.push_back(FOO.bytes[0]);
                program.push_back(FOO.bytes[1]);
                program.push_back(FOO.bytes[2]);
                program.push_back(FOO.bytes[3]);
                bytes_done += 4;
              }
              else
                program.push_back(GOTO);
              FOO.x = L+initValue.size();
              if(isGlobal)
                FOO.x+=5;
              program.push_back(FOO.bytes[0]);
              program.push_back(FOO.bytes[1]);
              program.push_back(FOO.bytes[2]);
              program.push_back(FOO.bytes[3]);
              if(decl)
              {
                program.push_back(POP_STACK);
                bytes_done+=1;
                STACK_SIZE-=1;
              }
              bytes_done += 1 + JUMPOFFSet_Size;
          }
          else if (ast->val == "foreach" )
          {
              
              string loop_var_name = ast->childs[1]->val;

              vector<uint8_t> startIdx = exprByteCode(ast->childs[3]);
              program.insert(program.end(),startIdx.begin(),startIdx.end());
              int32_t E = STACK_SIZE;
              STACK_SIZE+=1;

              std::unordered_map<string,int32_t> m;
              bool add = true;
              size_t index = 0;
              int32_t fnIdx = 0;
              for(index = 0;index < vm.builtin.size();index+=1)
              {
                if(vm.builtin[index]==funcs["len"])
                {
                  add = false;
                  break;
                }
              }
              if(add)
              {
                vm.builtin.push_back(funcs["len"]);
                fnIdx = vm.builtin.size()-1;
              }
              else
                fnIdx = index;
              //Bytecode to calculate length of list we are looping
              vector<uint8_t> LIST = exprByteCode(ast->childs[2]);
              program.insert(program.end(),LIST.begin(),LIST.end());
              int32_t ListStackIdx = STACK_SIZE;
                STACK_SIZE+=1;
              FOO.x = E;
              program.push_back(INPLACE_INC);
              program.push_back(FOO.bytes[0]);
              program.push_back(FOO.bytes[1]);
              program.push_back(FOO.bytes[2]);
              program.push_back(FOO.bytes[3]);
              contTargets.push_back(bytes_done);
              bytes_done+=5;
  //////////
              int32_t before = STACK_SIZE;
              scope+=1;
              indexOfLastWhileLocals.push_back(locals.size());
              locals.push_back(m);
              locals.back().emplace(loop_var_name,STACK_SIZE);
              STACK_SIZE+=1;
              size_t L = bytes_done;
              ByteSrc tmp = { fileTOP,line_num };
              LineNumberTable->emplace(L+12, tmp);
              bytes_done+=20+6+1+JUMPOFFSet_Size+6;
              breakTargets.push_back(bytes_done-1-JUMPOFFSet_Size-7-6);
              vector<uint8_t> block = compile(ast->childs[4],infunc);
              program.push_back(LOAD);
              program.push_back('v');
              FOO.x = E;
              program.push_back(FOO.bytes[0]);
              program.push_back(FOO.bytes[1]);
              program.push_back(FOO.bytes[2]);
              program.push_back(FOO.bytes[3]);
              //
              program.push_back(LOAD);
              program.push_back('v');
              FOO.x = ListStackIdx;
              program.push_back(FOO.bytes[0]);
              program.push_back(FOO.bytes[1]);
              program.push_back(FOO.bytes[2]);
              program.push_back(FOO.bytes[3]);
              //

              program.push_back(CALLFORVAL);
              FOO.x = fnIdx;
              program.push_back(FOO.bytes[0]);
              program.push_back(FOO.bytes[1]);
              program.push_back(FOO.bytes[2]);
              program.push_back(FOO.bytes[3]);
              program.push_back(1);
              program.push_back(SMALLERTHAN);
              program.push_back(JMPIFFALSE);
              int32_t where = 7+block.size()+1+JUMPOFFSet_Size+5+6;
              int32_t whileLocals = locals.back().size();
              if(whileLocals!=0)
                where+=4;
              FOO.x = where;
              program.push_back(FOO.bytes[0]);
              program.push_back(FOO.bytes[1]);
              program.push_back(FOO.bytes[2]);
              program.push_back(FOO.bytes[3]);

            //  program.insert(program.end(),LIST.begin(),LIST.end());
              FOO.x = ListStackIdx;
              program.push_back(LOAD);
              program.push_back('v');
              program.push_back(FOO.bytes[0]);
              program.push_back(FOO.bytes[1]);
              program.push_back(FOO.bytes[2]);
              program.push_back(FOO.bytes[3]);
              program.push_back(LOAD);
              program.push_back('v');
              FOO.x = E;
              program.push_back(FOO.bytes[0]);
              program.push_back(FOO.bytes[1]);
              program.push_back(FOO.bytes[2]);
              program.push_back(FOO.bytes[3]);
              program.push_back(INDEX);
              program.insert(program.end(),block.begin(),block.end());
              program.push_back(INPLACE_INC);
              FOO.x = E;
              program.push_back(FOO.bytes[0]);
              program.push_back(FOO.bytes[1]);
              program.push_back(FOO.bytes[2]);
              program.push_back(FOO.bytes[3]);
              bytes_done+=5;
              STACK_SIZE = before-2;
              breakTargets.pop_back();
              contTargets.pop_back();
              scope -= 1;
              indexOfLastWhileLocals.pop_back();

              locals.pop_back();
              if(whileLocals!=0)
              {
                program.push_back(GOTONPSTACK);
                FOO.x = whileLocals;
                program.push_back(FOO.bytes[0]);
                program.push_back(FOO.bytes[1]);
                program.push_back(FOO.bytes[2]);
                program.push_back(FOO.bytes[3]);
                bytes_done += 4;
              }
              else
                program.push_back(GOTO);
              FOO.x = L;
              program.push_back(FOO.bytes[0]);
              program.push_back(FOO.bytes[1]);
              program.push_back(FOO.bytes[2]);
              program.push_back(FOO.bytes[3]);
              program.push_back(NPOP_STACK);
              FOO.x = 2;
              program.push_back(FOO.bytes[0]);
              program.push_back(FOO.bytes[1]);
              program.push_back(FOO.bytes[2]);
              program.push_back(FOO.bytes[3]);
              bytes_done += 1 + JUMPOFFSet_Size+JUMPOFFSet_Size+1;
          }
          else if(ast->val == "namespace")
          {
            if(infunc)
            {
              compileError("SyntaxError","Namespace declartion inside function!");
            }
            string name = ast->childs[1]->val;
            string prefix;
            for(auto e: prefixes)
              prefix+=e;
            prefixes.push_back(prefix+name+"::");
            vector<uint8_t> block = compile(ast->childs[2],false);
            prefixes.pop_back();
            program.insert(program.end(),block.begin(),block.end());
            //Hasta La Vista Baby
          }
          else if (ast->val == "if")
          {
              
              vector<uint8_t> cond = exprByteCode(ast->childs[1]->childs[0]);
              if(ast->childs[2]->val=="endif")
              {
                program.insert(program.end(),cond.begin(),cond.end());
                program.push_back(POP_STACK);
                bytes_done+=1;
                ast = ast->childs.back();
                continue;
              }
              bytes_done += 1 + JUMPOFFSet_Size;
              scope += 1;
              int32_t before = STACK_SIZE;
              std::unordered_map<string,int32_t> m;
              locals.push_back(m);
              vector<uint8_t> block = compile(ast->childs[2], infunc);
              scope -= 1;
              STACK_SIZE = before;
              bool hasLocals = (locals.back().size()!=0);

              program.insert(program.end(), cond.begin(), cond.end());
              program.push_back(JMPIFFALSE);
              FOO.x = block.size();//
              if(hasLocals)
                FOO.x+=5;
              program.push_back(FOO.bytes[0]);
              program.push_back(FOO.bytes[1]);
              program.push_back(FOO.bytes[2]);
              program.push_back(FOO.bytes[3]);
              program.insert(program.end(), block.begin(), block.end());
              if(hasLocals)
              {
                program.push_back(NPOP_STACK);
                FOO.x = locals.back().size();
                program.push_back(FOO.bytes[0]);
                program.push_back(FOO.bytes[1]);
                program.push_back(FOO.bytes[2]);
                program.push_back(FOO.bytes[3]);
                bytes_done += 5;
              }
              locals.pop_back();
          }
          else if (ast->val == "ifelse")
          {
              
              vector<uint8_t> ifcond = exprByteCode(ast->childs[1]->childs[0]);
              bytes_done += 1 + JUMPOFFSet_Size;
              scope += 1;
              int32_t before = STACK_SIZE;
              std::unordered_map<string,int32_t> m;
              locals.push_back(m);
              vector<uint8_t> ifblock = compile(ast->childs[2], infunc);
              scope -= 1;
              STACK_SIZE = before;
              //
              int32_t iflocals = locals.back().size();
              locals.pop_back();
              program.insert(program.end(), ifcond.begin(), ifcond.end());
              program.push_back(JMPIFFALSE);
              FOO.x = ifblock.size() + 1 + JUMPOFFSet_Size;

              if(iflocals!=0)
              {
                FOO.x+=4;
                bytes_done+=4;
              }
              program.push_back(FOO.bytes[0]);
              program.push_back(FOO.bytes[1]);
              program.push_back(FOO.bytes[2]);
              program.push_back(FOO.bytes[3]);
              program.insert(program.end(), ifblock.begin(), ifblock.end());


              scope += 1;
              bytes_done += 1 + JUMPOFFSet_Size;
              before = STACK_SIZE;
              locals.push_back(m);
              vector<uint8_t> elseblock = compile(ast->childs[3], infunc);
              scope -= 1;
              STACK_SIZE = before;
              int32_t elseLocals = locals.back().size();
              locals.pop_back();
              if(iflocals!=0)
              {
                program.push_back(JMPNPOPSTACK);
                FOO.x = iflocals;
                program.push_back(FOO.bytes[0]);
                program.push_back(FOO.bytes[1]);
                program.push_back(FOO.bytes[2]);
                program.push_back(FOO.bytes[3]);
              }
              else
                program.push_back(JMP);
              FOO.x = elseblock.size();
              if(elseLocals!=0)
                FOO.x+=5;
              program.push_back(FOO.bytes[0]);
              program.push_back(FOO.bytes[1]);
              program.push_back(FOO.bytes[2]);
              program.push_back(FOO.bytes[3]);
              program.insert(program.end(), elseblock.begin(), elseblock.end());
              if(elseLocals!=0)
              {
                program.push_back(NPOP_STACK);
                FOO.x = elseLocals;
                program.push_back(FOO.bytes[0]);
                program.push_back(FOO.bytes[1]);
                program.push_back(FOO.bytes[2]);
                program.push_back(FOO.bytes[3]);
                bytes_done += 5;
              }
          }
          else if (ast->val == "ifelifelse")
          {
              
              vector<uint8_t> ifcond = exprByteCode(ast->childs[1]->childs[0]);
              bytes_done += 1 + JUMPOFFSet_Size;//JumpIfFalse after if condition
              scope += 1;
              std::unordered_map<string,int32_t> m;
              locals.push_back(m);
              int32_t before = STACK_SIZE;
              vector<uint8_t> ifblock = compile(ast->childs[2], infunc);
              scope -= 1;
              STACK_SIZE = before;
              int32_t iflocals = locals.back().size();
              locals.pop_back();
              bytes_done += 1 + JUMPOFFSet_Size;//JMP of if block
              if(iflocals!=0)
                bytes_done+=4;//for NPOP_STACK+ 4 byte operand
              vector<vector<uint8_t>> elifConditions;
              int32_t elifBlocksSize = 0;//total size in bytes elif blocks take including their conditions
              int32_t elifBlockcounter = 3;//third node of ast
              vector<int32_t> elifLocalSizes ;
              vector<vector<uint8_t>> elifBlocks;
              for (size_t k = 1; k < ast->childs[1]->childs.size(); k += 1)
              {
                  vector<uint8_t> elifCond = exprByteCode(ast->childs[1]->childs[k]);
                  elifConditions.push_back(elifCond);
                  bytes_done += 1 + JUMPOFFSet_Size;//JMPIFFALSE of elif
                  scope += 1;
                  before = STACK_SIZE;
                  locals.push_back(m);
                  vector<uint8_t> elifBlock = compile(ast->childs[elifBlockcounter], infunc);
                  scope -= 1;
                  STACK_SIZE = before;
                  int32_t eliflocals = locals.back().size();
                  elifLocalSizes.push_back(eliflocals);
                  locals.pop_back();
                  bytes_done += 1 + JUMPOFFSet_Size;// JMP of elifBlock
                  elifBlocks.push_back(elifBlock);
                  elifBlocksSize += elifCond.size() + 1 + JUMPOFFSet_Size + elifBlock.size() + 1 + JUMPOFFSet_Size ;
                  if(eliflocals!=0)
                  {
                    elifBlocksSize+=4;
                    bytes_done+=4;
                  }
                  elifBlockcounter += 1;
              }
              scope += 1;
              locals.push_back(m);
              before = STACK_SIZE;
              vector<uint8_t> elseBlock = compile(ast->childs[ast->childs.size() - 2], infunc);
              scope -= 1;
              STACK_SIZE = before;
              int32_t elseLocals = locals.back().size();
              locals.pop_back();
              program.insert(program.end(), ifcond.begin(), ifcond.end());
              program.push_back(JMPIFFALSE);
              FOO.x = ifblock.size() + 1 + JUMPOFFSet_Size ;
              if(iflocals!=0)
                FOO.x+=4;

              program.push_back(FOO.bytes[0]);
              program.push_back(FOO.bytes[1]);
              program.push_back(FOO.bytes[2]);
              program.push_back(FOO.bytes[3]);
              program.insert(program.end(), ifblock.begin(), ifblock.end());
              if(iflocals!=0)
              {
                program.push_back(JMPNPOPSTACK);
                FOO.x = iflocals;
                program.push_back(FOO.bytes[0]);
                program.push_back(FOO.bytes[1]);
                program.push_back(FOO.bytes[2]);
                program.push_back(FOO.bytes[3]);
              }
              else
                program.push_back(JMP);
              FOO.x = elifBlocksSize + elseBlock.size() ;
              if(elseLocals!=0)
                FOO.x+=5;
              program.push_back(FOO.bytes[0]);
              program.push_back(FOO.bytes[1]);
              program.push_back(FOO.bytes[2]);
              program.push_back(FOO.bytes[3]);
              for (size_t k = 0; k < elifBlocks.size(); k += 1)
              {

                  elifBlocksSize -= elifBlocks[k].size() + elifConditions[k].size() + 1 + (2 * JUMPOFFSet_Size) + 1;
                  if(elifLocalSizes[k]!=0)
                    elifBlocksSize -= 4;
                  program.insert(program.end(), elifConditions[k].begin(), elifConditions[k].end());
                  program.push_back(JMPIFFALSE);
                  FOO.x = elifBlocks[k].size() + JUMPOFFSet_Size + 1;
                  if(elifLocalSizes[k]!=0)
                    FOO.x+=4;
                  program.push_back(FOO.bytes[0]);
                  program.push_back(FOO.bytes[1]);
                  program.push_back(FOO.bytes[2]);
                  program.push_back(FOO.bytes[3]);
                  program.insert(program.end(), elifBlocks[k].begin(), elifBlocks[k].end());
                  if(elifLocalSizes[k]!=0)
                  {

                    program.push_back(JMPNPOPSTACK);
                    FOO.x = elifLocalSizes[k];
                    program.push_back(FOO.bytes[0]);
                    program.push_back(FOO.bytes[1]);
                    program.push_back(FOO.bytes[2]);
                    program.push_back(FOO.bytes[3]);
                  }
                  else
                    program.push_back(JMP);
                  FOO.x = elifBlocksSize + elseBlock.size() ;
                  if(elseLocals!=0)
                    FOO.x+=5;
                  program.push_back(FOO.bytes[0]);
                  program.push_back(FOO.bytes[1]);
                  program.push_back(FOO.bytes[2]);
                  program.push_back(FOO.bytes[3]);
              }

              program.insert(program.end(), elseBlock.begin(), elseBlock.end());
              if(elseLocals!=0)
              {
                program.push_back(NPOP_STACK);
                FOO.x = elseLocals;
                program.push_back(FOO.bytes[0]);
                program.push_back(FOO.bytes[1]);
                program.push_back(FOO.bytes[2]);
                program.push_back(FOO.bytes[3]);
                bytes_done+=5;
              }
          }
          else if (ast->val == "ifelif")
          {
            
            vector<uint8_t> ifcond = exprByteCode(ast->childs[1]->childs[0]);
            bytes_done += 1 + JUMPOFFSet_Size;//JumpIfFalse after if condition
            scope += 1;
            std::unordered_map<string,int32_t> m;
            locals.push_back(m);
            int32_t before = STACK_SIZE;
            vector<uint8_t> ifblock = compile(ast->childs[2], infunc);
            scope -= 1;
            STACK_SIZE = before;
            int32_t iflocals = locals.back().size();
            locals.pop_back();
            bytes_done += 1 + JUMPOFFSet_Size;//JMP of if block
            if(iflocals!=0)
              bytes_done+=4;//for NPOP_STACK+ 4 byte operand
            vector<vector<uint8_t>> elifConditions;
            int32_t elifBlocksSize = 0;//total size in bytes elif blocks take including their conditions
            int32_t elifBlockcounter = 3;//third node of ast
            vector<int32_t> elifLocalSizes ;
            vector<vector<uint8_t>> elifBlocks;
            for (size_t k = 1; k < ast->childs[1]->childs.size(); k += 1)
            {
                vector<uint8_t> elifCond = exprByteCode(ast->childs[1]->childs[k]);
                elifConditions.push_back(elifCond);
                bytes_done += 1 + JUMPOFFSet_Size;//JMPIFFALSE of elif
                scope += 1;
                before = STACK_SIZE;
                locals.push_back(m);
                vector<uint8_t> elifBlock = compile(ast->childs[elifBlockcounter], infunc);
                scope -= 1;
                STACK_SIZE = before;
                int32_t eliflocals = locals.back().size();
                elifLocalSizes.push_back(eliflocals);
                locals.pop_back();
                bytes_done += 1 + JUMPOFFSet_Size;// JMP of elifBlock
                elifBlocks.push_back(elifBlock);
                elifBlocksSize += elifCond.size() + 1 + JUMPOFFSet_Size + elifBlock.size() + 1 + JUMPOFFSet_Size ;
                if(eliflocals!=0)
                {
                  elifBlocksSize+=4;
                  bytes_done+=4;
                }
                elifBlockcounter += 1;
            }

            program.insert(program.end(), ifcond.begin(), ifcond.end());
            program.push_back(JMPIFFALSE);
            FOO.x = ifblock.size() + 1 + JUMPOFFSet_Size ;
            if(iflocals!=0)
              FOO.x+=4;

            program.push_back(FOO.bytes[0]);
            program.push_back(FOO.bytes[1]);
            program.push_back(FOO.bytes[2]);
            program.push_back(FOO.bytes[3]);
            program.insert(program.end(), ifblock.begin(), ifblock.end());
            if(iflocals!=0)
            {
              program.push_back(JMPNPOPSTACK);
              FOO.x = iflocals;
              program.push_back(FOO.bytes[0]);
              program.push_back(FOO.bytes[1]);
              program.push_back(FOO.bytes[2]);
              program.push_back(FOO.bytes[3]);
            }
            else
              program.push_back(JMP);
            FOO.x = elifBlocksSize;

            program.push_back(FOO.bytes[0]);
            program.push_back(FOO.bytes[1]);
            program.push_back(FOO.bytes[2]);
            program.push_back(FOO.bytes[3]);
            for (size_t k = 0; k < elifBlocks.size(); k += 1)
            {
                elifBlocksSize -= elifBlocks[k].size() + elifConditions[k].size() + 1 + (2 * JUMPOFFSet_Size) + 1;
                if(elifLocalSizes[k]!=0)
                  elifBlocksSize -= 4;
                program.insert(program.end(), elifConditions[k].begin(), elifConditions[k].end());
                program.push_back(JMPIFFALSE);
                FOO.x = elifBlocks[k].size() + JUMPOFFSet_Size + 1;
                if(elifLocalSizes[k]!=0)
                  FOO.x+=4;
                program.push_back(FOO.bytes[0]);
                program.push_back(FOO.bytes[1]);
                program.push_back(FOO.bytes[2]);
                program.push_back(FOO.bytes[3]);
                program.insert(program.end(), elifBlocks[k].begin(), elifBlocks[k].end());
                if(elifLocalSizes[k]!=0)
                {

                  program.push_back(JMPNPOPSTACK);
                  FOO.x = elifLocalSizes[k];
                  program.push_back(FOO.bytes[0]);
                  program.push_back(FOO.bytes[1]);
                  program.push_back(FOO.bytes[2]);
                  program.push_back(FOO.bytes[3]);
                }
                else
                  program.push_back(JMP);
                FOO.x = elifBlocksSize ;
                program.push_back(FOO.bytes[0]);
                program.push_back(FOO.bytes[1]);
                program.push_back(FOO.bytes[2]);
                program.push_back(FOO.bytes[3]);
            }


          }
          else if(ast->val=="throw")
          {
            
            vector<uint8_t> code = exprByteCode(ast->childs[1]);
            vector<uint8_t> msg = exprByteCode(ast->childs[2]);
            program.insert(program.end(),code.begin(),code.end());
            program.insert(program.end(),msg.begin(),msg.end());
            program.push_back(THROW);
            ByteSrc tmp = { fileTOP,line_num };
            LineNumberTable->emplace(bytes_done, tmp);
            bytes_done+=1;

          }
          else if(ast->val=="trycatch")
          {
            program.push_back(ONERR_GOTO);
            bytes_done+=5;
            scope += 1;
            int32_t before = STACK_SIZE;
            std::unordered_map<string,int32_t> m;
            locals.push_back(m);
            vector<uint8_t> tryBlock = compile(ast->childs[2],false,false);
            scope -= 1;
            STACK_SIZE = before;
            int32_t trylocals = locals.back().size();
            locals.pop_back();

            FOO.x = bytes_done+6 +(trylocals==0 ? 0:5);
            program.push_back(FOO.bytes[0]);
            program.push_back(FOO.bytes[1]);
            program.push_back(FOO.bytes[2]);
            program.push_back(FOO.bytes[3]);
            program.insert(program.end(),tryBlock.begin(),tryBlock.end());
            if(trylocals!=0)
            {
              FOO.x = trylocals;
              program.push_back(NPOP_STACK);
              program.push_back(FOO.bytes[0]);
              program.push_back(FOO.bytes[1]);
              program.push_back(FOO.bytes[2]);
              program.push_back(FOO.bytes[3]);
              bytes_done+=5;

            }
            program.push_back(POP_EXCEP_TARG);
            bytes_done+=6;
            scope += 1;
            before = STACK_SIZE;
            m.emplace(ast->childs[1]->val,STACK_SIZE);
            STACK_SIZE+=1;
            locals.push_back(m);
            vector<uint8_t> catchBlock = compile(ast->childs[3],false,false);
            int32_t catchlocals = locals.back().size();
            scope-=1;
            FOO.x = catchBlock.size()+(catchlocals==1 ? 1 : 5);
            program.push_back(JMP);
            program.push_back(FOO.bytes[0]);
            program.push_back(FOO.bytes[1]);
            program.push_back(FOO.bytes[2]);
            program.push_back(FOO.bytes[3]);
            program.insert(program.end(),catchBlock.begin(),catchBlock.end());
            STACK_SIZE = before;

            if(catchlocals>1)
            {
            FOO.x = catchlocals;
            program.push_back(NPOP_STACK);
            program.push_back(FOO.bytes[0]);
            program.push_back(FOO.bytes[1]);
            program.push_back(FOO.bytes[2]);
            program.push_back(FOO.bytes[3]);
            bytes_done+=5;
            }
            {
              program.push_back(POP_STACK);
              bytes_done+=1;
            }
            locals.pop_back();

          }
          else if (substr(0, 4, ast->val) == "func " || (isGen = substr(0,3,ast->val) == "gen "))
          {
              int32_t selfIdx= 0;
              if(infunc)
              {
                line_num = atoi(ast->childs[0]->val.substr(5).c_str());
                compileError("SyntaxError","Error function within function not allowed!");
              }
              string name;
              if(isGen)
               name = ast->val.substr(4);
              else
                name = ast->val.substr(5);
              if(compileAllFuncs || std::find(fnReferenced->begin(),fnReferenced->end(),name)!=fnReferenced->end() || inclass)
              {
                if(funcs.find(name)!=funcs.end() && !inclass)
                  compileError("NameError","Error a builtin function with same name already exists!");
                if(scope==0 && globals.find(name)!=globals.end())
                  compileError("NameError","Error redeclaration of name "+name);
                else if(scope!=0 && locals.back().find(name)!=locals.back().end())
                  compileError("NameError","Error redeclaration of name "+name);
                if(!inclass)
                {
                  if(scope==0)
                    globals.emplace(name,STACK_SIZE);
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
                for (size_t k =0; k<ast->childs[1]->childs.size(); k += 1)
                {
                  string n = ast->childs[1]->childs[k]->val;
                  if(ast->childs[1]->childs[k]->childs.size()!=0)
                  {
                    if(isGen)
                      compileError("SyntaxError","Error default parameters not supported for couroutines");
                    expr = exprByteCode(ast->childs[1]->childs[k]->childs[0]);
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
                FOO.x = bytes_done+2+JUMPOFFSet_Size+JUMPOFFSet_Size+JUMPOFFSet_Size+1 +((isGen) ? 0 : 1);
                program.push_back(FOO.bytes[0]);
                program.push_back(FOO.bytes[1]);
                program.push_back(FOO.bytes[2]);
                program.push_back(FOO.bytes[3]);
                
                auto it = std::find(vm.strings.begin(),vm.strings.end(),name);
                if(it==vm.strings.end())
                {
                   vm.strings.push_back(name);
                   FOO.x = vm.strings.size()-1;
                }
                else
                  FOO.x = it - vm.strings.begin();
                program.push_back(FOO.bytes[0]);
                program.push_back(FOO.bytes[1]);
                program.push_back(FOO.bytes[2]);
                program.push_back(FOO.bytes[3]);
                if(inclass)
                    program.push_back(ast->childs[1]->childs.size()+1);
                else
                  program.push_back(ast->childs[1]->childs.size());
                //Push number of optional parameters
                if(!isGen)
                {
                  program.push_back(def_param);
                  bytes_done++;
                }
                bytes_done+=10;

                /////////
                bytes_done += 1 + JUMPOFFSet_Size ;
                locals.push_back(C);
                vector<uint8_t> paraBytes;
                
                
                
                scope += 1;
                inGen = isGen;
                vector<uint8_t> funcBody = compile(ast->childs[2], true,false);
                scope -= 1;
                inConstructor = false;
                inGen = false;
                locals.pop_back();
                STACK_SIZE = before;
                if(name!="__construct__")
                {
                  if (funcBody.size() == 0)
                  {
                        FOO.x = 0;
                        funcBody.push_back(LOAD_CONST);
                        funcBody.push_back(FOO.bytes[0]);
                        funcBody.push_back(FOO.bytes[1]);
                        funcBody.push_back(FOO.bytes[2]);
                        funcBody.push_back(FOO.bytes[3]);
                        if(isGen)
                          funcBody.push_back(CO_STOP);
                        else
                          funcBody.push_back(RETURN);
                        bytes_done +=6;
                  }
                  else
                  {
                    if (funcBody[funcBody.size() - 1] != RETURN)
                    {
                        FOO.x = 0;
                        funcBody.push_back(LOAD_CONST);
                        funcBody.push_back(FOO.bytes[0]);
                        funcBody.push_back(FOO.bytes[1]);
                        funcBody.push_back(FOO.bytes[2]);
                        funcBody.push_back(FOO.bytes[3]); 
                        if(isGen)
                          funcBody.push_back(CO_STOP);
                        else
                          funcBody.push_back(RETURN);
                        bytes_done +=6;
                    }
                  }
                }
                else
                {
                  funcBody.push_back(LOAD);
                  funcBody.push_back('v');
                  FOO.x = selfIdx;
                  funcBody.push_back(FOO.bytes[0]);
                  funcBody.push_back(FOO.bytes[1]);
                  funcBody.push_back(FOO.bytes[2]);
                  funcBody.push_back(FOO.bytes[3]); 
                  funcBody.push_back(RETURN);
                  bytes_done+=7;
                }
                program.push_back(JMP);
                FOO.x = paraBytes.size() + funcBody.size() ;
                program.push_back(FOO.bytes[0]);
                program.push_back(FOO.bytes[1]);
                program.push_back(FOO.bytes[2]);
                program.push_back(FOO.bytes[3]);
                program.insert(program.end(), paraBytes.begin(), paraBytes.end());
                program.insert(program.end(), funcBody.begin(), funcBody.end());
            }
            else
            {
              //printf("not compiling function %s because it is not fnReferenced\n",name.c_str());
            }
            isGen = false;
          }
          else if(ast->val=="class" || ast->val=="extclass")
          {
            if(infunc)
            {
              compileError("SyntaxError","Class declaration inside function!");
            }
            bool extendedClass = false;
            if(ast->val=="extclass")
            {
              extendedClass = true;
            }
            string name = ast->childs[1]->val;
            size_t C = line_num;
            if(scope==0)
            {
              if(globals.find(name)!=globals.end())
                compileError("NameError","Redeclaration of name "+name);
              globals.emplace(name,STACK_SIZE);
            }
            else
            {
              if(locals.back().find(name)!=locals.back().end())
                compileError("NameError","Redeclaration of name "+name);
              locals.back().emplace(name,STACK_SIZE);
            }
            STACK_SIZE+=1;
            vector<string> names = scanClass(ast->childs[2]);
            if(extendedClass)
            {
              vector<uint8_t> baseClass = exprByteCode(ast->childs[ast->childs.size()-2]);
              program.insert(program.end(),baseClass.begin(),baseClass.end());
            }
            for(auto e: names)
            {
              auto it = std::find(vm.strings.begin(),vm.strings.end(),e);
              if(e=="super")
              {
                compileError("NameError","Error class not allowed to have members named \"super\"");
              }
              if(it==vm.strings.end())
              {
                FOO.x = vm.strings.size();
                vm.strings.push_back(e);
              }
              else
                FOO.x = it - vm.strings.begin();
              program.push_back(LOAD_STR);
              program.push_back(FOO.bytes[0]);
              program.push_back(FOO.bytes[1]);
              program.push_back(FOO.bytes[2]);
              program.push_back(FOO.bytes[3]);
              bytes_done+=5;
            }
            std::unordered_map<string,int32_t> M;
            locals.push_back(M);
            int32_t before = STACK_SIZE;
            scope+=1;
            vector<uint8_t> block = compile(ast->childs[2],false,true);
            scope-=1;

            locals.pop_back();
            int32_t totalMembers = names.size();
            STACK_SIZE = before;

            program.insert(program.end(),block.begin(),block.end());
            if(extendedClass)
            {
              program.push_back(BUILD_DERIVED_CLASS);
              ByteSrc tmp = { fileTOP,C };
              LineNumberTable->emplace(bytes_done, tmp);
            }
            else
              program.push_back(BUILD_CLASS);
            FOO.x = totalMembers;
            program.push_back(FOO.bytes[0]);
            program.push_back(FOO.bytes[1]);
            program.push_back(FOO.bytes[2]);
            program.push_back(FOO.bytes[3]);
            auto it = std::find(vm.strings.begin(),vm.strings.end(),name);
            if(it==vm.strings.end())
            {
               vm.strings.push_back(name);
               FOO.x = vm.strings.size()-1;
            }
            else
              FOO.x = it - vm.strings.begin();
            program.push_back(FOO.bytes[0]);
            program.push_back(FOO.bytes[1]);
            program.push_back(FOO.bytes[2]);
            program.push_back(FOO.bytes[3]);
            bytes_done+=9;


          }
          else if (ast->val == "return")
          {
              if(inConstructor)
                compileError("SyntaxError","Error class constructors should not return anything!");
              
              vector<uint8_t> val = exprByteCode(ast->childs[1]);
              program.insert(program.end(), val.begin(), val.end());
              ByteSrc tmp = { fileTOP,line_num };
              LineNumberTable->emplace(bytes_done, tmp);
              if(inGen)
                program.push_back(CO_STOP);
              else
                program.push_back(RETURN);
              bytes_done += 1;
          }
          else if (ast->val == "yield")
          {
              if(inConstructor)
                compileError("SyntaxError","Error class constructor can not be generators!");
              
              vector<uint8_t> val = exprByteCode(ast->childs[1]);
              program.insert(program.end(), val.begin(), val.end());              
              ByteSrc tmp = { fileTOP,line_num };
              LineNumberTable->emplace(bytes_done, tmp);
              program.push_back(YIELD);
              bytes_done += 1;
          }
          else if (ast->val == "break")
          {

              program.push_back(BREAK);
              FOO.x = breakTargets.back();

              program.push_back(FOO.bytes[0]);
              program.push_back(FOO.bytes[1]);
              program.push_back(FOO.bytes[2]);
              program.push_back(FOO.bytes[3]);
              FOO.x = 0;
              for(int32_t i = locals.size()-1;i>=indexOfLastWhileLocals.back();i-=1)
                FOO.x += locals[i].size();

              program.push_back(FOO.bytes[0]);
              program.push_back(FOO.bytes[1]);
              program.push_back(FOO.bytes[2]);
              program.push_back(FOO.bytes[3]);
              bytes_done += 9;
          }
          else if(ast->val=="gc")
          {
            program.push_back(GC);
            bytes_done+=1;
          }
          else if (ast->val == "continue")
          {

              program.push_back(CONT);
              FOO.x = contTargets.back();
              program.push_back(FOO.bytes[0]);
              program.push_back(FOO.bytes[1]);
              program.push_back(FOO.bytes[2]);
              program.push_back(FOO.bytes[3]);
          //    FOO.x = scope - scopeOfLastWhile.back();
              FOO.x = 0;
              for(int32_t i = locals.size()-1;i>=indexOfLastWhileLocals.back();i-=1)
                FOO.x += locals[i].size();
              program.push_back(FOO.bytes[0]);
              program.push_back(FOO.bytes[1]);
              program.push_back(FOO.bytes[2]);
              program.push_back(FOO.bytes[3]);
              bytes_done += 9;
          }
          else if (ast->val == "call")
          {

              string name = ast->childs[1]->val;
              string nameToSet;
              
              bool udf;
              if (funcs.find(name) != funcs.end())
              {
                  udf = false;
              }
              else
              {
                udf = true;
              }
              for (size_t k = 0; k < ast->childs[2]->childs.size(); k += 1)
              {
                  vector<uint8_t> val = exprByteCode(ast->childs[2]->childs[k]);
                  program.insert(program.end(), val.begin(), val.end());
              }
              if (udf)
              {
                  Node* E = NewNode("id: "+name);
                  vector<uint8_t> fn = exprByteCode(E);
                  delete E;
                  program.insert(program.end(),fn.begin(),fn.end());
                  ByteSrc tmp = { fileTOP,line_num };
                  LineNumberTable->emplace(bytes_done, tmp);
                  program.push_back(CALLUDF);
                  program.push_back(ast->childs[2]->childs.size());
                  bytes_done += 3;
                  program.push_back(POP_STACK);
              }
              else
              {
                  ByteSrc tmp = { fileTOP,line_num };
                  LineNumberTable->emplace(bytes_done, tmp);
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
                    FOO.x = vm.builtin.size()-1;
                  }
                  else
                    FOO.x = index;
                  program.push_back(FOO.bytes[0]);
                  program.push_back(FOO.bytes[1]);
                  program.push_back(FOO.bytes[2]);
                  program.push_back(FOO.bytes[3]);
                  bytes_done += 5;
                  program.push_back((char)ast->childs[2]->childs.size());

              }
          }
          else if(ast->val == "file")
          {
            if(infunc)
              compileError("SyntaxError","Error file import inside function!");
            if(inclass)
              compileError("SyntaxError","Error file import inside class!");
              
            string C = filename;
            filename = ast->childs[1]->val;
            short K = fileTOP;
            fileTOP = std::find(files->begin(),files->end(),filename) - files->begin();
            vector<uint8_t> fByteCode = compile(ast->childs[2],false,false);
            program.insert(program.end(),fByteCode.begin(),fByteCode.end());
            filename = C;
            fileTOP = K;
          }
          else
          {
              printf("SyntaxError in file %s\n%s\nThe line does not seem to be a meaningful statement!\n", filename.c_str(), ast->val.c_str());
              exit(0);
          }
          ast = ast->childs.back();
      }
      return program;

  }
  void reduceStackTo(int size)//for REPL
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
  vector<uint8_t> compileProgram(Node* ast,int32_t argc,const char* argv[],vector<uint8_t> prev = {},bool compileNonRefFns = false,bool popGlobals = true)//compiles as a complete program adds NPOP_STACK and OP_EXIT
  {
    //If prev vector is empty then this program will be compiled as an independent new one
    //otherwise this program will be an addon to the previous one
    //new = prev +curr

    bytes_done = prev.size();
    compileAllFuncs = compileNonRefFns;
    line_num = 1;
    andJMPS.clear();
    orJMPS.clear();
    if(prev.size() == 0)
    {
      STACK_SIZE = 3;
      globals.clear();
      locals.clear();
      scope = 0;
      prefixes.clear();
      globals.emplace("argv",0);
      globals.emplace("stdin",1);
      globals.emplace("stdout",2);
      int32_t k = 2;
      PltList l;
      PltObject elem;
      elem.type = 's';
      while (k < argc)
      {
          //elem.s = argv[k];
          string* p = allocString();
          *p = argv[k];
          l.push_back(PObjFromStrPtr(p));
          k += 1;
      }
      PltObject A;
      A.type = 'j';
      PltList* p = allocList();
      *p = l;
      A.ptr = (void*)p;
      vm.STACK.push_back(A);
      FileObject* STDIN = allocFileObject();
      STDIN->open = true;
      STDIN ->fp = stdin;

      FileObject* STDOUT = allocFileObject();
      STDOUT->open = true;
      STDOUT ->fp = stdout;
      A.type = 'u';
      A.ptr = (void*)STDIN;
      vm.STACK.push_back(A);
      A.type = 'u';
      A.ptr = (void*)STDOUT;
      vm.STACK.push_back(A);
      vm.strings.push_back("super");
    }
    vector<uint8_t> bytecode = compile(ast,false);
    if(globals.size()!=0 && popGlobals)
    {
          bytecode.push_back(NPOP_STACK);
          FOO.x = globals.size();
          bytecode.push_back(FOO.bytes[0]);
          bytecode.push_back(FOO.bytes[1]);
          bytecode.push_back(FOO.bytes[2]);
          bytecode.push_back(FOO.bytes[3]);
          bytes_done+=5;
    }
    bytecode.push_back(OP_EXIT);
    bytes_done+=1;
    bytecode.insert(bytecode.begin(),prev.begin(),prev.end());
    if (bytes_done != bytecode.size())
    {
        printf("Plutonium encountered an internal error.\nError Code: 10\n");
       // printf("%ld   /  %ld  bytes done\n",bt,bytecode.size());
        exit(0);
    }
    //final phase
    //optimize short circuit jumps for and 
    for(auto e: andJMPS)
    {
      int offset;
      memcpy(&offset,&bytecode[e+1],4);
     // printf("offset = %d\n",offset);
      int k = e+5+offset;
    //  printf("k = %d  %d\n",k,bytecode[k] == JMPIFFALSENOPOP);
      int newoffset;
      while(k < bytes_done && bytecode[k] == JMPIFFALSENOPOP)
      {
        memcpy(&newoffset,&bytecode[k+1],4);
        offset+=newoffset+5;
        k = k+5+newoffset;
      }
      //printf("newoffset = %d\n",offset);
      memcpy(&bytecode[e+1],&offset,4);
    }
    //optimize short circuit jumps for or
    for(auto e: orJMPS)
    {
      int offset;
      memcpy(&offset,&bytecode[e+1],4);
     // printf("offset = %d\n",offset);
      int k = e+5+offset;
    //  printf("k = %d  %d\n",k,bytecode[k] == JMPIFFALSENOPOP);
      int newoffset;
      while(k < bytes_done && bytecode[k] == NOPOPJMPIF)
      {
        memcpy(&newoffset,&bytecode[k+1],4);
        offset+=newoffset+5;
        k = k+5+newoffset;
      }
      //printf("newoffset = %d\n",offset);
      memcpy(&bytecode[e+1],&offset,4);
    }
    return bytecode;
  }
};
#endif
