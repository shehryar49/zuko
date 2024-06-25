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

#include <cstdint>
#include <vector>
#include "apifunctions.h" 
#include "builtinfunc.h"
#include "programinfo.h"

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



#define AS_STD_STR(x) (std::string)(((zstr*)x.ptr)->val)

struct MemInfo //Data structure used by GC
{
  char type;
  bool isMarked;
  size_t size; // only used for Z_RAW
};


extern "C"
{
  zobject zobj_from_str(const char* str);// makes deep copy of str
  zobject z_err(zclass* errKlass,const char* des);
  zlist* vm_alloc_zlist();
  zbytearr* vm_alloc_zbytearr();
  uint8_t* vm_alloc_raw(size_t);
  zstr* vm_alloc_zstr(size_t);
  zclass* vm_alloc_zclass();
  zmodule* vm_alloc_zmodule();
  zclass_object* vm_alloc_zclassobj(zclass*);
  Coroutine* vm_alloc_coro_obj();//allocates coroutine object
  zfun* vm_alloc_zfun();
  zfun* vm_alloc_coro(); //coroutine can be represented by fun_object
  zfile * vm_alloc_zfile();
  zdict* vm_alloc_zdict();
  znativefun* vm_alloc_znativefun();
  //callObject also behaves as a kind of try/catch since v0.3.1
  bool vm_call_object(zobject*,zobject*,int,zobject*);
  void vm_mark_important(void* mem);
  void vm_unmark_important(void* mem);
  uint8_t* vm_realloc_raw(void*,size_t);
  uint8_t* vm_alloc_raw(size_t);
}
//
//Error classes (defined in vm.cpp)
extern zclass* Error;
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

class VM
{
private:
  uint8_t* program = NULL;
  uint32_t program_size;
  uint8_t *k; // instruction pointer
  std::vector<uint8_t*> callstack;
  bool viaCO = false;
  std::vector<size_t> tryStackCleanup;
  std::vector<int32_t> frames = {0}; // stores starting stack indexes of all stack frames
  size_t orgk = 0;
  std::vector<uint8_t*> except_targ;
  std::vector<size_t> tryLimitCleanup;
  #ifdef _WIN32
    std::vector<HINSTANCE> moduleHandles;
  #else
    std::vector<void *> moduleHandles;
  #endif

  std::vector<zfun*> executing = {NULL}; // pointer to zuko function object we are executing, NULL means control is not in a function
  size_t GC_THRESHHOLD;
  size_t GC_MIN_COLLECT;
  std::unordered_map<size_t, ByteSrc> *LineNumberTable;
  std::vector<std::string> *files;
  std::vector<std::string> *sources;
  zlist aux; // auxiliary space for markV2
  zlist STACK;
  std::vector<void*> important;//important memory not to free even when not reachable
  std::vector<BuiltinFunc> builtin; // addresses of builtin native functions
  
  size_t GC_Cycles = 0;
  std::vector<zstr> strings; // string constants used in bytecode
  zobject *constants = NULL;
  int32_t total_constants = 0; // total constants stored in the array constants
  apiFuncions api; // to share VM's allocation api with modules
  // just a struct with a bunch of function pointers
  zobject nil;
  size_t spitErr(zclass* e, std::string msg); /* used to show a runtime error
  e is the error class*/
  void markV2(const zobject &obj);
  void mark();
  void collectGarbage();
  inline void DoThreshholdBusiness();
  bool invokeOperator(const std::string& meth, zobject A, size_t args, const char* op, zobject *rhs = NULL, bool raiseErrOnNF = true); // check if the object has the specified operator overloaded and prepares to call it by updating callstack and frames
public:
  size_t allocated = 0;
  std::unordered_map<void*, MemInfo> memory;
  friend class Compiler;
  friend zobject SET_GC_PARAMS(zobject*,int32_t);
  //These functions are outside the class, so they can be called by C code
  friend bool vm_call_object(zobject*,zobject*,int,zobject*);
  friend void vm_mark_important(void*);
  friend void vm_unmark_important(void*);
  //the REPL mainloop function
  friend void REPL();
  friend void dis(vector<uint8_t>&);
  VM();
  
  void load(std::vector<uint8_t>& bytecode,ZukoSource& p);
  void interpret(size_t offset = 0, bool panic = true); //by default panic if stack is not empty when finished
  //in REPL mode panic is set to false
  ~VM();
};

extern VM vm;

#endif
