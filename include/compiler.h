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
#include "programinfo.h"
#include "vm.h"
#include "parser.h"
#include "opcode.h"
#include "zfileobj.h"
#include <unordered_map>


#define JUMPOFFSET_SIZE 4
#define SymbolTable std::unordered_map<string,int32_t>



struct Pair
{
  int32_t x;
  int32_t y;
  Pair(int32_t a,int32_t b) : x(a) , y(b) {}
};
class Compiler
{
private:
  int32_t STACK_SIZE = 0;//simulating STACK's size
  //int32_t scope = 0;
  int32_t localsBegin;
  int32_t foo = 0;
  int32_t* num_of_constants;
  size_t line_num = 1;
  size_t bytes_done = 0;
  short fileTOP;
  std::string filename;
  std::string className;

  std::unordered_map<string,bool> symRef;
  std::unordered_map<size_t,ByteSrc>* LineNumberTable;
  std::unordered_map<std::string,std::pair<Node*,int32_t>> compiled_functions; //compiled functions (not methods)
  //this allows code generation for direct function call
  SymbolTable globals;
  
  std::vector<SymbolTable> locals;
  std::vector<std::string> prefixes = {""};
  std::vector<std::string>* files;
  std::vector<string>* sources;
  //For optimizing short circuit jumps after code generation
  std::vector<size_t> andJMPS;
  std::vector<size_t> orJMPS;  
  std::vector<int32_t> breakIdx; // a break statement sets this after adding GOTO to allow
  // the enclosing loop to backpatch the offset
  std::vector<int32_t> contIdx; // same as above
  std::vector<string> classMemb;
  std::vector<uint8_t> bytecode; // bytecode generated so far
  std::vector<Pair> backpatches;

  //Context and other options
  bool inConstructor = false;
  bool inGen = false;
  bool inclass = false;
  bool infunc = false;
  bool infor = false; //used by break and continue statements
  bool compileAllFuncs = false;
  bool returnStmtAtFnScope = false;
  NodeType last_stmt_type;
  void compileError(std::string type,std::string msg);
  void add_lntable_entry(size_t opcodeIdx);
  void optimize_jmps(vector<uint8_t>& bytecode);

  int32_t is_duplicate_constant(zobject x);
  int32_t add_to_vm_strings(const std::string& n);
  int32_t add_builtin_to_vm(const std::string& name);
  int32_t resolve_name(std::string name,bool& isGlobal,bool blowUp=true,bool* isFromSelf=NULL);  
  
  vector<uint8_t> expr_bytecode(Node* ast);
  vector<string> scan_class(Node* ast);
  vector<uint8_t> compile(Node* ast);

  zclass* make_error_class(std::string name,zclass* error);

public:
  friend void REPL();

  //set_source function is used instead of constructor
  //this way the Compiler class becomes reusable
  //by initializing it again
  void set_source(ZukoSource& p,size_t root_idx = 0);
  void add_builtin(const std::string& name,BuiltinFunc fn);  
  void reduceStackTo(int size);//for REPL

  //Compile options
  static const int32_t OPT_COMPILE_DEADCODE = 0x1; // does exactly what it says
  static const int32_t OPT_POP_GLOBALS = 0x1 << 1; // to add bytecode instructions to pop globals from VM STACK
  vector<uint8_t>& compileProgram(Node* ast,int32_t argc,const char* argv[],int32_t options = OPT_POP_GLOBALS);//compiles as a complete program adds NPOP_STACK and OP_EXIT

};
#endif
