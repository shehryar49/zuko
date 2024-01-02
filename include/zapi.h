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

#include "zobject.h"
#include "zstr.h"
#include <vector>
#include <unordered_map>
#include <string>
using namespace std;






typedef ZObject(*NativeFunPtr)(ZObject*,int);




struct Klass
{
  string name;
  std::unordered_map<string,ZObject> members;
  std::unordered_map<string,ZObject> privateMembers;
  inline void addMember(const string& name,ZObject val)
  {
    members.emplace(name,val);
  }
  inline void addPrivateMember(const string& name,ZObject val)
  {
    privateMembers.emplace(name,val);
  }
  void addNativeMethod(const string& name,NativeFunPtr r);
  
};
struct KlassObject
{
  Klass* klass;
  std::unordered_map<string,ZObject> members;
  std::unordered_map<string,ZObject> privateMembers;
};
struct Module
{
  std::string name;
  std::unordered_map<string,ZObject> members;
  void addNativeFunction(const string& name,NativeFunPtr r);
};
struct FunObject
{
  Klass* klass;//functions can be binded to classes as methods in which case they will have access to private members of that class
  //and also keep the class alive with them
  string name;
  size_t i;
  size_t args;
  zlist opt; //default/optional parameters
  #ifdef __cplusplus
  FunObject& operator=(const FunObject&) = delete;
  #endif
};


struct NativeFunction
{
  Klass* klass;//address of class the function is member of (if any NULL otherwise)
  NativeFunPtr addr;
  string name;//name of function (used when printing errors)
  NativeFunction& operator=(const NativeFunction&) = delete;
};
enum CoState
{
  SUSPENDED,
  RUNNING,
  STOPPED,
};
struct Coroutine
{
  int curr;//index in bytecode from where to resume the coroutine
  zlist locals;
  CoState state;
  string name;
  FunObject* fun;//function from which this coroutine object was made
  bool giveValOnResume;
  Coroutine& operator=(const Coroutine&) = delete;
};

//Error classes
Klass* Error;
Klass* TypeError;
Klass* ValueError;
Klass* MathError; 
Klass* NameError;
Klass* IndexError;
Klass* ArgumentError;
Klass* FileIOError;
Klass* KeyError;
Klass* OverflowError;
Klass* FileOpenError;
Klass* FileSeekError; 
Klass* ImportError;
Klass* ThrowError;
Klass* MaxRecursionError;
Klass* AccessError;
//these classes are set by either the compiler or by the api_setup()

//
//Helper and extension API Functions
inline ZObject ZObjFromStrPtr(ZStr* s)
{
  ZObject ret;
  ret.type = 's';
  ret.ptr = (void*)s;
  return ret;
}
inline ZObject ZObjFromMStrPtr(string* s)
{
  ZObject ret;
  ret.type = Z_MSTR;
  ret.ptr = (void*)s;
  return ret;
}
inline ZObject ZObjFromInt(int32_t x)
{
  ZObject ret;
  ret.type = 'i';
  ret.i = x;
  return ret;
}
inline ZObject ZObjFromDouble(double f)
{
  ZObject ret;
  ret.type = 'f';
  ret.f = f;
  return ret;
}
inline ZObject ZObjFromInt64(int64_t x)
{
  ZObject ret;
  ret.type = 'l';
  ret.l = x;
  return ret;
}
inline ZObject ZObjFromPtr(void* p)
{
  ZObject ret;
  ret.type = 'p';
  ret.ptr = p;
  return ret;
}
inline ZObject ZObjFromByte(uint8_t x)
{
  ZObject ret;
  ret.type = 'm';
  ret.i = x;
  return ret;
}
inline ZObject ZObjFromBool(bool b)
{
  ZObject ret;
  ret.type = Z_BOOL;
  ret.i = b;
  return ret;
}

inline ZObject ZObjFromList(zlist* l)
{
  ZObject ret;
  ret.type = Z_LIST;
  ret.ptr = (void*)l;
  return ret;
}
inline ZObject ZObjFromDict(ZDict* l)
{
  ZObject ret;
  ret.type = Z_DICT;
  ret.ptr = (void*)l;
  return ret;
}
inline ZObject ZObjFromModule(Module* k)
{
  ZObject ret;
  ret.type = Z_MODULE;
  ret.ptr = (void*)k;
  return ret;
}
inline ZObject ZObjFromKlass(Klass* k)
{
  ZObject ret;
  ret.type = Z_CLASS;
  ret.ptr = (void*)k;
  return ret;
}
inline ZObject ZObjFromKlassObj(KlassObject* ki)
{
  ZObject ret;
  ret.type = Z_OBJ;
  ret.ptr = (void*)ki;
  return ret;
}
inline ZObject ZObjFromByteArr(vector<uint8_t>* k)
{
  ZObject ret;
  ret.type = Z_BYTEARR;
  ret.ptr = (void*)k;
  return ret;
}
inline ZObject ZObjFromFile(zfile* file)
{
  ZObject ret;
  ret.type = Z_FILESTREAM;
  ret.ptr = (void*)file;
  return ret;
}

#define AS_BOOL(x) (bool)x.i
#define AS_INT(x) x.i
#define AS_INT64(x) x.l
#define AS_DOUBLE(x) x.f
#define AS_BYTE(x) (uint8_t)x.i
#define AS_STR(x) ((ZStr*)x.ptr)
#define AS_MSTR(x) *(string*)x.ptr
#define AS_DICT(x) *(ZDict*)x.ptr
#define AS_LIST(x) *(PltList*)x.ptr
#define AS_KLASS(x) *(Klass*)x.ptr
#define AS_KlASSOBJECT(x) *(KlassObject*)x.ptr
#define AS_BYTEARRAY(x) *(vector<uint8_t>*)x.ptr
#define AS_FILEOBJECT(x) *(FileObject*)x.ptr
#define AS_PTR(x) x.ptr


typedef zlist*(*fn1)();//allocList
typedef ZDict*(*fn2)();//allocZDict
typedef ZStr*(*fn3)(size_t);//allocString
typedef string*(*fn4)();//allocMutString
typedef zfile*(*fn5)();//allocFileObject
typedef Klass*(*fn6)();//allocKlass
typedef KlassObject*(*fn7)();//allocKlassObject
typedef NativeFunction*(*fn8)();//allocNativeFunObj
typedef Module*(*fn9)();//allocModule
typedef vector<uint8_t>*(*fn10)();//allocBytearray
typedef bool(*fn11)(ZObject*,ZObject*,int,ZObject*);//callobject
typedef void(*fn12)(void*);//markImportant
typedef void(*fn13)(void*);//unmarkImpotant

//
fn1 vm_allocList;
fn2 vm_allocDict;
fn3 vm_allocString;
fn4 vm_allocMutString;
fn5 vm_allocFileObject;
fn6 vm_allocKlass;
fn7 vm_allocKlassObject;
fn8 vm_allocNativeFunObj;
fn9 vm_allocModule;
fn10 vm_allocByteArray;
fn11 vm_callObject;
fn12 vm_markImportant;
fn13 vm_unmarkImportant;
//The below helper functions make use of above function pointers
inline ZObject ZObjFromStr(const char* str)
{
  size_t len = strlen(str);
  ZStr* ptr = vm_allocString(len);
  memcpy(ptr->val,str,len);
  ZObject ret;
  ret.type = 's';
  ret.ptr = (void*)ptr;
  return ret;
}

ZObject Z_Err(Klass* errKlass,string des)
{
  ZObject ret;
  KlassObject* p = vm_allocKlassObject();
  p->klass = errKlass;
  p->members = errKlass->members;
  p->privateMembers = errKlass->privateMembers;
  p->members["msg"] = ZObjFromStr(des.c_str()); // OPTIMIZE!
  ret.type = Z_ERROBJ;//indicates an object in thrown state
  ret.ptr = (void*) p;
  return ret;
}
inline ZObject ZObjFromMethod(string name,NativeFunPtr r,Klass* k)
{
  ZObject ret;
  NativeFunction* fn = vm_allocNativeFunObj();
  fn->name = name;
  fn->klass = k;
  fn->addr = r;
  ret.type = 'y';
  ret.ptr = (void*)fn;
  return ret;
}
inline ZObject ZObjFromFunction(string name,NativeFunPtr r,Klass* k=NULL)
{
  ZObject ret;
  NativeFunction* fn = vm_allocNativeFunObj();
  fn->name = name;
  fn->klass = k;
  fn->addr = r;
  ret.type = 'y';
  ret.ptr = (void*)fn;
  return ret;
}

void Klass::addNativeMethod(const string& name,NativeFunPtr r)
{
  NativeFunction* fn = vm_allocNativeFunObj();
  fn->name = name;
  fn->klass = this;
  fn->addr = r;
  ZObject tmp;
  tmp.type = Z_NATIVE_FUNC;
  tmp.ptr = (void*)fn;
  this->members.emplace(name,tmp);
}
////////
void Module::addNativeFunction(const string& name,NativeFunPtr r)
{
  NativeFunction* fn = vm_allocNativeFunObj();
  fn->name = name;
  fn->klass = NULL;
  fn->addr = r;
  ZObject tmp;
  tmp.type = Z_NATIVE_FUNC;
  tmp.ptr = (void*)fn;
  this->members.emplace(name,tmp);
}

//
extern "C"
{
  struct apiFuncions
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

  };
  #ifdef _WIN32
  #ifndef ZUKO_INTERPRETER //to make sure this header is included in a shared library
  //and not the zuko interpreter
  __declspec(dllexport)
  #endif
  #endif
  void api_setup(apiFuncions* p)
  {
    vm_allocList = p->a1;
    vm_allocDict = p->a2;
    vm_allocString = p->a3;
    vm_allocMutString = p->a4;
    vm_allocFileObject = p->a5;
    vm_allocKlass = p->a6;
    vm_allocKlassObject = p->a7;
    vm_allocNativeFunObj = p->a8;
    vm_allocModule = p->a9;
    vm_allocByteArray = p->a10;
    vm_callObject = p->a11;
    vm_markImportant = p->a12;
    vm_unmarkImportant = p->a13;
    Error = p->k1;
    TypeError = p->k2;
    ValueError = p->k3;
    MathError = p->k4; 
    NameError = p->k5;
    IndexError = p->k6;
    ArgumentError = p->k7;
    FileIOError = p->k8;
    KeyError = p->k9;
    OverflowError = p->k10;
    FileOpenError = p->k11;
    FileSeekError = p->k12; 
    ImportError = p->k13;
    ThrowError = p->k14;
    MaxRecursionError = p->k15;
    AccessError = p->k16;
  }
}

#endif