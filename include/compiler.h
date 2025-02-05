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

typedef struct compiled_fn
{
    Node* fn_node;
    size_t offset;
}compiled_fn;

typedef struct compiler_ctx
{
    int32_t STACK_SIZE;//simulating STACK's size
    int32_t localsBegin;
    int32_t foo;
    size_t line_num;
    size_t bytes_done;
    short fileTOP;
    const char* filename;
    const char* classname;
    str_vector symRef;
    lntable* line_num_table;
    //this allows code generation for direct function call
    ptr_vector compiled_functions; 
    symtable globals;
    ptr_vector locals;
    str_vector prefixes;
    str_vector* files;
    str_vector* sources;
    //For optimizing short circuit jumps after code generation
    sizet_vector and_jumps;
    sizet_vector or_jumps;  
    sizet_vector break_stmt_idx; // a break statement sets this after adding GOTO to allow
    // the enclosing loop to backpatch the offset
    sizet_vector continue_stmt_idx;
    str_vector curr_class_members;
    zbytearr bytecode; // bytecode generated so far
    pair_vector backpatches;

    //Context and other options
    bool in_constructor;
    bool in_coroutine;
    bool inclass;
    bool infunc;
    bool infor; //used by break and continue statements
    bool compile_deadcode; // whether to compile unused classes and functions or not
    NodeType last_stmt_type; // type of last statement compiled
}compiler_ctx;

compiler_ctx* create_compiler_context(zuko_src*);
void compile_error(compiler_ctx* ctx,const char* type,const char* msg);
void compiler_add_builtin(const char* name,BuiltinFunc fn);  
void compiler_reduce_stack_size(compiler_ctx* ctx,int size);//for REPL
//Compile options
static const int32_t OPT_COMPILE_DEADCODE = 1; // does exactly what it says
static const int32_t OPT_NOPOP_GLOBALS = 1 << 1; // to not add bytecode instructions for popping globals from VM STACK
static const int32_t OPT_NOEXIT = 1 << 2; // does not add OP_EXIT at the end of generated bytecode
uint8_t* compile_program(compiler_ctx* ctx,Node* ast,int32_t argc,const char* argv[],int32_t options);//compiles as a complete program adds NPOP_STACK and OP_EXIT
void compiler_destroy(compiler_ctx*);

#ifdef __cplusplus
}
#endif

#endif
