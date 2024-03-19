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
#ifndef VM_H_
#define VM_H_

#include "zuko.h"
#include "builtinfunc.h"

using namespace std;
#ifdef THREADED_INTERPRETER
  #ifdef __GNUC__
    #define NEXT_INST goto *targets[*k];
    #define CASE_CP
    #define ISTHREADED //threaded interpreter can be and will be implemented
  #else
    #define NEXT_INST continue
    #define CASE_CP case
  #endif
#else
  #define NEXT_INST continue
  #define CASE_CP case
#endif
using namespace std;


#define AS_STD_STR(x) (string)(((ZStr*)x.ptr)->val)

struct MemInfo
{
  char type;
  bool isMarked;
};


extern "C"
{
  bool vm_callObject(ZObject*,ZObject*,int,ZObject*);
  void vm_markImportant(void*);
  void vm_unmarkImportant(void*);
}

class VM
{
private:
  vector<uint8_t*> callstack;
  uint8_t* program = NULL;
  uint32_t program_size;
  uint8_t *k;
  bool viaCO = false;
  vector<size_t> tryStackCleanup;
  vector<int32_t> frames = {0}; // stores starting stack indexes of all stack frames
  size_t orgk = 0;
  vector<uint8_t *> except_targ;
  vector<size_t> tryLimitCleanup;
  // int32_t Gc_Cycles = 0;
  #ifdef _WIN32
    vector<HINSTANCE> moduleHandles;
  #else
    vector<void *> moduleHandles;
  #endif

  vector<FunObject *> executing = {NULL}; // pointer to zuko function object we are executing,NULL means control is not in a function
  
  
  // referenced in bytecode
  size_t GC_THRESHHOLD;
  std::unordered_map<size_t, ByteSrc> *LineNumberTable;
  vector<string> *files;
  vector<string> *sources;
  ZList aux; // auxiliary space for markV2
  ZList STACK;
  vector<void*> important;//important memory not to free even when not reachable
  
public:
  vector<BuiltinFunc> builtin; // addresses of builtin native functions
  friend class Compiler;
  
  friend bool vm_callObject(ZObject*,ZObject*,int,ZObject*);
  friend void vm_markImportant(void*);
  friend void vm_unmarkImportant(void*);

  friend void REPL();
  std::unordered_map<void *, MemInfo> memory;
  size_t allocated = 0;
  size_t GC_Cycles = 0;
  vector<ZStr> strings; // string constants used in bytecode
  ZObject *constants = NULL;
  int32_t total_constants = 0; // total constants stored in the array constants
  apiFuncions api; // to share VM's allocation api with modules
  // just a struct with a bunch of function pointers
  ZObject nil;
  VM();
  void load(vector<uint8_t>& bytecode,ProgramInfo& p);
  size_t spitErr(Klass* e, string msg); // used to show a runtime error
  void markV2(const ZObject &obj);
  void mark();
  void collectGarbage();
  inline void DoThreshholdBusiness();
  bool invokeOperator(string meth, ZObject A, size_t args, const char* op, ZObject *rhs = NULL, bool raiseErrOnNF = true); // check if the object has the specified operator overloaded and prepares to call it by updating callstack and frames
  void interpret(size_t offset = 0, bool panic = true); //by default panic if stack is not empty when finished
  ~VM();
};

extern VM vm;

#endif
