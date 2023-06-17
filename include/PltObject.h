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
#ifndef PLTOBJECT_H_
#define PLTOBJECT_H_
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
using namespace std;
struct HashFunction;
struct PltObject;
#define PltList vector<PltObject>
#define Dictionary std::unordered_map<PltObject,PltObject,HashFunction>

//
//Types for different plutonium objects
#define PLT_LIST 'j'
#define PLT_DICT 'a'
#define PLT_INT 'i'
#define PLT_INT64 'l'
#define PLT_FLOAT 'f'
#define PLT_BYTE  'm'
#define PLT_NATIVE_FUNC 'y'//native function
#define PLT_MODULE 'q'
#define PLT_STR 's'
#define PLT_FILESTREAM 'u'
#define PLT_NIL 'n'
#define PLT_OBJ 'o' //objects created using plutonium code
#define PLT_CLASS 'v' //Plutonium code class
#define PLT_BOOL 'b'
#define PLT_POINTER 'p' //just a pointer that means something only to c++ code,the interpreter just stores it
#define PLT_FUNC 'w' //plutonium code function
#define PLT_COROUTINE 'g'
#define PLT_COROUTINE_OBJ 'z'
#define PLT_ERROBJ 'e' //same as an object,just in thrown state
#define PLT_BYTEARR 'c'
//

struct FileObject
{
  FILE* fp;
  bool open;
};
extern "C" struct PltObject
{
    union
    {
        double f;
        int64_t l;
        int32_t i;
        void* ptr;
    };
    char type='n';
};
struct HashFunction
    {
      size_t operator()(const PltObject& obj) const
      {
        size_t a = std::hash<char>()(obj.type);
        size_t b;
        size_t hashPltObject(const PltObject&);
        b = hashPltObject(obj) << 1;
        return a ^ b;
      }
  };
bool operator==(const PltObject& lhs,const PltObject& other)
    {
        if(other.type!=lhs.type)
            return false;
        if(lhs.type=='n')
          return true;
        else if(lhs.type=='i')
          return lhs.i==other.i;
        else if(lhs.type=='l')
          return lhs.l==other.l;
        else if(lhs.type=='f')
          return lhs.f==other.f;
        else if(lhs.type=='b')
          return lhs.i==other.i;
        else if(lhs.type=='m')
          return lhs.i==other.i;
        else if(lhs.type=='u')
          return ((FileObject*)lhs.ptr)==((FileObject*)other.ptr);
        else if(lhs.type=='s')
          return *(string*)other.ptr==*(string*)lhs.ptr;
        else if(lhs.type=='j')
            return *(PltList*)lhs.ptr==*(PltList*)other.ptr;
        else if(other.type=='y' || other.type=='r' || other.type == PLT_CLASS)
          return lhs.ptr==other.ptr;
        else if(lhs.type=='q' || lhs.type=='z')
          return lhs.ptr==other.ptr;
        else if(lhs.type=='a')
            return *(Dictionary*)lhs.ptr==*(Dictionary*)other.ptr;
        return false;
    }
size_t hashPltObject(const PltObject& a)
{
    char t = a.type;
    if(t=='s')
        return std::hash<std::string>()(*(string*)a.ptr);
    else if(t=='i')
        return std::hash<int>()(a.i);
    else if(t=='l')
        return std::hash<long long int>()(a.l);
    else if(t=='f')
        return std::hash<double>()(a.f);
    else if(t=='m')
        return std::hash<unsigned char>()(a.i);
    else if(t=='b')
        return std::hash<bool>()(a.i);
    //other types not supported as keys in dictionaries
    return 0;
}
struct Klass
{
  string name;
  std::unordered_map<string,PltObject> members;
  std::unordered_map<string,PltObject> privateMembers;
};
struct KlassInstance
{
  Klass* klass;
  std::unordered_map<string,PltObject> members;
  std::unordered_map<string,PltObject> privateMembers;
};
struct Module
{
  std::string name;
  std::unordered_map<string,PltObject> members;
};
struct FunObject
{
  Klass* klass;//functions can be binded to classes as methods in which case they will have access to private members of that class
  //and also keep the class alive with them
  string name;
  size_t i;
  size_t args;
  PltList opt; //default/optional parameters
};

typedef PltObject(*NativeFunPtr)(PltObject*,int);
struct NativeFunction
{
  Klass* klass;//address of class the function is member of (if any NULL otherwise)
  NativeFunPtr addr;
  string name;//name of function (used when printing errors)
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
  PltList locals;
  CoState state;
  string name;
  FunObject* fun;//function from which this coroutine object was made
  bool giveValOnResume;
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
//Helper and extension API Functions
inline PltObject PObjFromStrPtr(string* s)
{
  PltObject ret;
  ret.type = 's';
  ret.ptr = (void*)s;
  return ret;
}

inline PltObject PObjFromInt(int32_t x)
{
  PltObject ret;
  ret.type = 'i';
  ret.i = x;
  return ret;
}
inline PltObject PObjFromDouble(double f)
{
  PltObject ret;
  ret.type = 'f';
  ret.f = f;
  return ret;
}
inline PltObject PObjFromInt64(int64_t x)
{
  PltObject ret;
  ret.type = 'l';
  ret.l = x;
  return ret;
}
inline PltObject PObjFromPtr(void* p)
{
  PltObject ret;
  ret.type = 'p';
  ret.ptr = p;
  return ret;
}
inline PltObject PObjFromByte(uint8_t x)
{
  PltObject ret;
  ret.type = 'm';
  ret.i = x;
  return ret;
}
inline PltObject PObjFromBool(bool b)
{
  PltObject ret;
  ret.type = PLT_BOOL;
  ret.i = b;
  return ret;
}

inline PltObject PObjFromList(PltList* l)
{
  PltObject ret;
  ret.type = PLT_LIST;
  ret.ptr = (void*)l;
  return ret;
}
inline PltObject PObjFromDict(Dictionary* l)
{
  PltObject ret;
  ret.type = PLT_DICT;
  ret.ptr = (void*)l;
  return ret;
}
inline PltObject PObjFromModule(Module* k)
{
  PltObject ret;
  ret.type = PLT_MODULE;
  ret.ptr = (void*)k;
  return ret;
}
inline PltObject PObjFromKlass(Klass* k)
{
  PltObject ret;
  ret.type = PLT_CLASS;
  ret.ptr = (void*)k;
  return ret;
}
inline PltObject PObjFromKlassInst(KlassInstance* ki)
{
  PltObject ret;
  ret.type = PLT_OBJ;
  ret.ptr = (void*)ki;
  return ret;
}
inline PltObject PObjFromByteArr(vector<uint8_t>* k)
{
  PltObject ret;
  ret.type = PLT_BYTEARR;
  ret.ptr = (void*)k;
  return ret;
}
inline PltObject PObjFromFile(FileObject* file)
{
  PltObject ret;
  ret.type = PLT_FILESTREAM;
  ret.ptr = (void*)file;
  return ret;
}

typedef PltList*(*fn1)();
typedef Dictionary*(*fn2)();
typedef string*(*fn3)();
typedef void*(*fn4)();//unused for now
typedef FileObject*(*fn5)();
typedef Klass*(*fn6)();
typedef KlassInstance*(*fn7)();
typedef NativeFunction*(*fn8)();
typedef Module*(*fn9)();
typedef vector<uint8_t>*(*fn10)();
typedef bool(*fn11)(PltObject*,PltObject*,int,PltObject*);
typedef void(*fn12)(void*);
typedef void(*fn13)(void*);

//
fn1 vm_allocList;
fn2 vm_allocDict;
fn3 vm_allocString;
//fn4 vm_allocErrObject;
fn5 vm_allocFileObject;
fn6 vm_allocKlass;
fn7 vm_allocKlassInstance;
fn8 vm_allocNativeFunObj;
fn9 vm_allocModule;
fn10 vm_allocByteArray;
fn11 vm_callObject;
fn12 vm_markImportant;
fn13 vm_unmarkImportant;
inline PltObject PObjFromStr(string s)
{
  string* ptr = vm_allocString();
  *ptr = s;
  PltObject ret;
  ret.type = 's';
  ret.ptr = (void*)ptr;
  return ret;
}
PltObject Plt_Err(Klass* errKlass,string des)
{
  PltObject ret;
  KlassInstance* p = vm_allocKlassInstance();
  p->klass = errKlass;
  p->members = errKlass->members;
  p->privateMembers = errKlass->privateMembers;
  p->members["msg"] = PObjFromStr(des);
  ret.type = PLT_ERROBJ;//indicates an object in thrown state
  ret.ptr = (void*) p;
  return ret;
}
inline PltObject PObjFromMethod(string name,NativeFunPtr r,Klass* k)
{
  PltObject ret;
  NativeFunction* fn = vm_allocNativeFunObj();
  fn->name = name;
  fn->klass = k;
  fn->addr = r;
  ret.type = 'y';
  ret.ptr = (void*)fn;
  return ret;
}
inline PltObject PObjFromFunction(string name,NativeFunPtr r,Klass* k=NULL)
{
  PltObject ret;
  NativeFunction* fn = vm_allocNativeFunObj();
  fn->name = name;
  fn->klass = k;
  fn->addr = r;
  ret.type = 'y';
  ret.ptr = (void*)fn;
  return ret;
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
  __declspec(dllexport)
  #endif
  void api_setup(apiFuncions* p)
  {
    vm_allocList = p->a1;
    vm_allocDict = p->a2;
    vm_allocString = p->a3;
  //  vm_allocErrObject = p->a4;
    vm_allocFileObject = p->a5;
    vm_allocKlass = p->a6;
    vm_allocKlassInstance = p->a7;
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
