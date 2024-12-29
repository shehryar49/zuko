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
#ifndef COMPILER_H_
#define COMPILER_H_
#include "ast.h"
#include "builtinfunc.h"
#include "lntable.h"
#include "pair-vector.h"
#include "zuko-src.h"
#include "vm.h"
#include "parser.h"
#include "opcode.h"
#include "zfileobj.h"
#include "zbytearray.h"
#include "sizet_vector.h"
#include "symtable.h"
#include "ptr-vector.h"
#include "pair-vector.h"

#ifdef __cplusplus
extern "C"{
#endif

#define JUMPOFFSET_SIZE 4



typedef struct compiler_ctx
{
  int32_t STACK_SIZE;//simulating STACK's size
  int32_t localsBegin;
  int32_t foo;
  int32_t* num_of_constants;
  size_t line_num;
  size_t bytes_done;
  short fileTOP;
  const char* filename;
  const char* className;
  str_vector symRef;
  lntable* line_num_table;
  //std::unordered_map<std::string,std::pair<Node*,int32_t>> compiled_functions; //compiled functions (not methods)
  //this allows code generation for direct function call
  
  symtable globals;
  ptr_vector locals;
  str_vector prefixes;
  str_vector* files;
  str_vector* sources;
  //For optimizing short circuit jumps after code generation
  sizet_vector andJMPS;
  sizet_vector orJMPS;  
  sizet_vector breakIdx; // a break statement sets this after adding GOTO to allow
  // the enclosing loop to backpatch the offset
  sizet_vector contIdx;
  str_vector classMemb;
  zbytearr bytecode; // bytecode generated so far
  pair_vector backpatches;

  //Context and other options
  bool inConstructor;
  bool inGen;
  bool inclass;
  bool infunc;
  bool infor; //used by break and continue statements
  bool compileAllFuncs;
  bool returnStmtAtFnScope;
  NodeType last_stmt_type;
}compiler_ctx;

void compileError(compiler_ctx* ctx,const char* type,const char* msg);
//set_source function is used instead of constructor
//this way the Compiler class becomes reusable
//by initializing it again
compiler_ctx* create_compiler_ctx(zuko_src*);
void compiler_set_source(compiler_ctx* ctx,zuko_src* p,size_t root_idx);
void compiler_add_builtin(const char* name,BuiltinFunc fn);  
void compiler_reduceStackTo(compiler_ctx* ctx,int size);//for REPL
//Compile options
static const int32_t OPT_COMPILE_DEADCODE = 0x1; // does exactly what it says
static const int32_t OPT_POP_GLOBALS = 0x1 << 1; // to add bytecode instructions to pop globals from VM STACK
uint8_t* compile_program(compiler_ctx* ctx,Node* ast,int32_t argc,const char* argv[],int32_t options);//compiles as a complete program adds NPOP_STACK and OP_EXIT
void compiler_destroy(compiler_ctx*);

#ifdef __cplusplus
}
#endif

#endif
