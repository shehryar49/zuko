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
#include "vm.h"
#include "lexer.h"
#include "parser.h"
#include "opcode.h"
#include "zfileobj.h"


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
  int32_t scope = 0;
  size_t line_num = 1;


  std::unordered_map<string,bool> symRef;
  std::vector<SymbolTable> locals;
  std::vector<std::string> prefixes = {""};
  int32_t* num_of_constants;
  std::string filename;
  std::vector<std::string>* files;
  std::vector<string>* sources;
  short fileTOP;
  std::unordered_map<size_t,ByteSrc>* LineNumberTable;
  bool compileAllFuncs;
  //For optimizing short circuit jumps after code generation
  std::vector<size_t> andJMPS;
  std::vector<size_t> orJMPS;
  //Context
  bool inConstructor = false;
  bool inGen = false;
  bool inclass = false;
  bool infunc = false;
  std::vector<int32_t> breakIdx; // a break statement sets this after adding GOTO to allow
  // the enclosing loop to backpatch the offset
  std::vector<int32_t> contIdx; // same as above
  int32_t localsBegin;
  std::string className;
  std::vector<string> classMemb;
  int32_t foo = 0;
  int32_t fnScope;
  bool returnStmtAtFnScope;
  std::vector<uint8_t> bytecode;
  std::vector<Pair> backpatches;
public:
  friend void REPL();
  SymbolTable globals;
  size_t bytes_done = 0;
  //init function is used instead of constructor
  //this way the Compiler class becomes reusable
  //by initializing it again
  void init(std::string filename,ProgramInfo& p);
  void compileError(std::string type,std::string msg);
  int32_t isDuplicateConstant(zobject x);
  int32_t addToVMStringTable(const std::string& n);
  void addLnTableEntry(size_t opcodeIdx);
  int32_t addBuiltin(const std::string& name);
  vector<uint8_t> exprByteCode(Node* ast);
  vector<string> scanClass(Node* ast);
  int32_t resolveName(std::string name,bool& isGlobal,bool blowUp=true,bool* isFromSelf=NULL);
  vector<uint8_t> compile(Node* ast);
  void optimizeJMPS(vector<uint8_t>& bytecode);
  void reduceStackTo(int size);//for REPL

  zclass* makeErrKlass(std::string name,zclass* error);
  vector<uint8_t>& compileProgram(Node* ast,int32_t argc,const char* argv[],bool compileNonRefFns = false,bool popGlobals=true);//compiles as a complete program adds NPOP_STACK and OP_EXIT

};
#endif
