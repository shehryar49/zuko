#include "zapi.h"

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




//Allocator function pointers
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

//Api setup
int api_setup(apiFuncions* p,int ver)
{
  if(ver != ZUKO_API_VERSION)
    return 0;  
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
  return 1;
}


//These helper functions utilized allocator functions
//that is why they are defined here
Klass* makeDerivedKlass(Klass* base)
{
  Klass* child = vm_allocKlass();
  ZObject super;
  super.type = Z_CLASS;
  super.ptr = (void*)base;
  StrMap_assign(&(child->members),&(base->members));
  StrMap_assign(&(child->privateMembers),&(base->privateMembers));
  Klass_setMember(child,"super",super);
  return child;
}

ZObject ZObjFromStr(const char* str)// makes deep copy of str
{
  size_t len = strlen(str);
  ZStr* ptr = vm_allocString(len);
  memcpy(ptr->val,str,len);
  ZObject ret;
  ret.type = 's';
  ret.ptr = (void*)ptr;
  return ret;
}
ZObject ZObjFromMethod(const char* name,NativeFunPtr r,Klass* k)
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
ZObject ZObjFromFunction(const char* name,NativeFunPtr r)
{
  ZObject ret;
  NativeFunction* fn = vm_allocNativeFunObj();
  fn->name = name;
  fn->addr = r;
  ret.type = 'y';
  ret.ptr = (void*)fn;
  return ret;
}


ZObject Z_Err(Klass* errKlass,const char* des)
{
  ZObject ret;
  KlassObject* p = vm_allocKlassObject(errKlass);
  StrMap_set(&(p->members),"msg",ZObjFromStr(des)); // OPTIMIZE!
  ret.type = Z_ERROBJ;//indicates an object in thrown state
  ret.ptr = (void*) p;
  return ret;
}


void Module_addNativeFun(Module* m,const char* name,NativeFunPtr p)
{
  ZObject val = ZObjFromFunction(name,p);
  StrMap_emplace(&(m->members),name,val);
}
void Module_addSigNativeFun(Module* m,const char* name,NativeFunPtr p,const char* sig)
{
  ZObject val = ZObjFromFunction(name,p);
  NativeFunction* fn = (NativeFunction*)val.ptr;
  fn->signature = sig;
  StrMap_emplace(&(m->members),name,val);
}

void Klass_addNativeMethod(Klass* k,const char* name,NativeFunPtr p)
{
  ZObject val = ZObjFromMethod(name,p,k);
  StrMap_emplace(&(k->members),name,val);
}
void Klass_addSigNativeMethod(Klass* k,const char* name,NativeFunPtr p,const char* sig)
{
  ZObject val = ZObjFromMethod(name,p,k);
  NativeFunction* fn = (NativeFunction*)val.ptr;
  fn->signature = sig;
  StrMap_emplace(&(k->members),name,val);
}