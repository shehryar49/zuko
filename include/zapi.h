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
#ifndef ZUKO_API_H
#define ZUKO_API_H
#define ZUKO_API_VERSION 1
#include "zobject.h"
#include "zstr.h"
#include "zbytearray.h"
#include "strmap.h"
#include "funobject.h"
#include "klass.h"
#include "klassobject.h"
#include "module.h"
#include "nativefun.h"
#include "coroutineobj.h"
#include "zlist.h"
#include "zdict.h"
#ifdef __cplusplus
extern "C"{
#endif

//Error classes
extern Klass* Error;
extern Klass* TypeError;
extern Klass* ValueError;
extern Klass* MathError; 
extern Klass* NameError;
extern Klass* IndexError;
extern Klass* ArgumentError;
extern Klass* FileIOError;
extern Klass* KeyError;
extern Klass* OverflowError;
extern Klass* FileOpenError;
extern Klass* FileSeekError; 
extern Klass* ImportError;
extern Klass* ThrowError;
extern Klass* MaxRecursionError;
extern Klass* AccessError;
//these classes are set by either the compiler or by the api_setup()


/**************/
typedef ZList*(*fn1)();//allocList
typedef ZDict*(*fn2)();//allocZDict
typedef ZStr*(*fn3)(size_t);//allocString
typedef void*(*fn4)();//allocMutString
typedef zfile*(*fn5)();//allocFileObject
typedef Klass*(*fn6)();//allocKlass
typedef KlassObject*(*fn7)(Klass*);//allocKlassObject
typedef NativeFunction*(*fn8)();//allocNativeFunObj
typedef Module*(*fn9)();//allocModule
typedef ZByteArr*(*fn10)();//allocBytearray
typedef bool(*fn11)(ZObject*,ZObject*,int,ZObject*);//callobject
typedef void(*fn12)(void*);//markImportant
typedef void(*fn13)(void*);//unmarkImpotant
typedef struct apiFuncions
{
  fn1 a1;//api function 1
  fn2 a2;//and so on
  fn3 a3;
  fn4 a4;
  fn5 a5;
  fn6 a6;
  fn7 a7;
  fn8 a8;
  fn9 a9;
  fn10 a10;
  fn11 a11;
  fn12 a12;
  fn13 a13;
  Klass* k1;
  Klass* k2;
  Klass* k3;
  Klass* k4;
  Klass* k5;
  Klass* k6;
  Klass* k7;
  Klass* k8;
  Klass* k9;
  Klass* k10;
  Klass* k11;
  Klass* k12;
  Klass* k13;
  Klass* k14;
  Klass* k15;
  Klass* k16;
}apiFuncions;
/**************/
#ifndef ZUKO_BUILDING_MODULE
// This header was included by the interpreter
// just forward declare helper functions
// Allocator functions
Klass* vm_allocKlass();
KlassObject * vm_allocKlassObject(Klass*);
ZList* vm_allocList();
ZByteArr* vm_allocByteArray();
ZDict * vm_allocDict();
ZStr* vm_allocString(size_t);
FunObject * vm_allocFunObject();
FunObject * vm_allocCoroutine();
Coroutine * vm_allocCoObj();
zfile * vm_alloczfile();
Module * vm_allocModule();
NativeFunction * vm_allocNativeFunObj();
FunObject* vm_allocFunObject();
bool vm_callObject(ZObject*,ZObject*,int,ZObject*);
void vm_markImportant(void*);
void vm_unmarkImportant(void*);
#else

// This header was included by a module
// Define function pointer variables for each allocator function
// and set them after interpreter passes the values to the modules
//
extern fn1 vm_allocList;
extern fn2 vm_allocDict;
extern fn3 vm_allocString;
extern fn4 vm_allocMutString;
extern fn5 vm_allocFileObject;
extern fn6 vm_allocKlass;
extern fn7 vm_allocKlassObject;
extern fn8 vm_allocNativeFunObj;
extern fn9 vm_allocModule;
extern fn10 vm_allocByteArray;
extern fn11 vm_callObject;
extern fn12 vm_markImportant;
extern fn13 vm_unmarkImportant;
  

#ifdef _WIN32
__declspec(dllexport)
#endif
int api_setup(apiFuncions* p,int ver);
#endif

Klass* makeDerivedKlass(Klass* base);
ZObject ZObjFromStr(const char*);// makes deep copy of str
ZObject Z_Err(Klass*,const char*);
ZObject ZObjFromMethod(const char*,NativeFunPtr,Klass*);
ZObject ZObjFromFunction(const char*,NativeFunPtr);
void Module_addNativeFun(Module* m,const char* name,NativeFunPtr p);
void Module_addSigNativeFun(Module* m,const char* name,NativeFunPtr p,const char* sig);
void Klass_addNativeMethod(Klass* k,const char* name,NativeFunPtr p);
void Klass_addSigNativeMethod(Klass* k,const char* name,NativeFunPtr p,const char* sig);
#ifdef __cplusplus
}
#endif

#endif