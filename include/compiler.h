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
using namespace std;

#define JUMPOFFSet_Size 4
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
  vector<size_t> contTargets;
  vector<size_t> breakTargets;
  vector<int32_t> indexOfLastWhileLocals;
  unordered_map<string,bool> symRef;
  vector<SymbolTable> locals;
  vector<string> prefixes = {""};
  int32_t* num_of_constants;
  string filename;
  vector<string>* files;
  vector<string>* sources;
  short fileTOP;
  unordered_map<size_t,ByteSrc>* LineNumberTable;
  bool compileAllFuncs;
  vector<size_t> andJMPS;
  vector<size_t> orJMPS;
  //Context
  bool inConstructor = false;
  bool inGen = false;
  bool inclass = false;
  bool infunc = false;
  vector<int32_t> breakIdx; // a break statement sets this after adding GOTO to allow
  // the enclosing loop to backpatch the offset
  vector<int32_t> contIdx; // same as above
  int32_t localsBegin;
  string className;
  vector<string> classMemb;
  int32_t foo = 0;
  int32_t fnScope;
  bool returnStmtAtFnScope;
  vector<uint8_t> bytecode;
  vector<Pair> backpatches;
public:
  friend void REPL();
  SymbolTable globals;
  size_t bytes_done = 0;
  //init function is used instead of constructor
  //this way the Compiler class becomes reusable
  //by initializing it again
  void init(string filename,ProgramInfo& p);
  void compileError(string type,string msg);
  int32_t isDuplicateConstant(ZObject x);
  int32_t addToVMStringTable(const string& n);
  void addLnTableEntry(size_t opcodeIdx);
  int32_t addBuiltin(const string& name);
  vector<uint8_t> exprByteCode(Node* ast);
  vector<string> scanClass(Node* ast);
  int32_t resolveName(string name,bool& isGlobal,bool blowUp=true,bool* isFromSelf=NULL);
  vector<uint8_t> compile(Node* ast);
  void optimizeJMPS(vector<uint8_t>& bytecode);
  void reduceStackTo(int size);//for REPL

  Klass* makeErrKlass(string name,Klass* error);
  vector<uint8_t>& compileProgram(Node* ast,int32_t argc,const char* argv[],bool compileNonRefFns = false,bool popGlobals=true);//compiles as a complete program adds NPOP_STACK and OP_EXIT

};
#endif
