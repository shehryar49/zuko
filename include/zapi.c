#include "zapi.h"
#include "zobject.h"

//Error classes
zclass* Error;
zclass* TypeError;
zclass* ValueError;
zclass* MathError; 
zclass* NameError;
zclass* IndexError;
zclass* ArgumentError;
zclass* FileIOError;
zclass* KeyError;
zclass* OverflowError;
zclass* FileOpenError;
zclass* FileSeekError; 
zclass* ImportError;
zclass* ThrowError;
zclass* MaxRecursionError;
zclass* AccessError;




//Allocator function pointers
fn1 vm_allocList;
fn2 vm_allocDict;
fn3 vm_allocString;
fn4 vm_allocMutString;
fn5 vm_allocFileObject;
fn6 vm_allocklass;
fn7 vm_allocklassObject;
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
  vm_allocklass = p->a6;
  vm_allocklassObject = p->a7;
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
zclass* makeDerivedklass(zclass* base)
{
  zclass* child = vm_allocklass();
  zobject super;
  super.type = Z_CLASS;
  super.ptr = (void*)base;
  StrMap_assign(&(child->members),&(base->members));
  StrMap_assign(&(child->privateMembers),&(base->privateMembers));
  zclass_setmember(child,"super",super);
  return child;
}

zobject ZObjFromStr(const char* str)// makes deep copy of str
{
  size_t len = strlen(str);
  zstr* ptr = vm_allocString(len);
  memcpy(ptr->val,str,len);
  zobject ret;
  ret.type = 's';
  ret.ptr = (void*)ptr;
  return ret;
}
zobject ZObjFromMethod(const char* name,NativeFunPtr r,zclass* k)
{
  zobject ret;
  znativefun* fn = vm_allocNativeFunObj();
  fn->name = name;
  fn->_klass = k;
  fn->addr = r;
  ret.type = 'y';
  ret.ptr = (void*)fn;
  return ret;
}
zobject ZObjFromFunction(const char* name,NativeFunPtr r)
{
  zobject ret;
  znativefun* fn = vm_allocNativeFunObj();
  fn->name = name;
  fn->addr = r;
  ret.type = 'y';
  ret.ptr = (void*)fn;
  return ret;
}


zobject Z_Err(zclass* errklass,const char* des)
{
  zobject ret;
  zclass_object* p = vm_allocklassObject(errklass);
  StrMap_set(&(p->members),"msg",zobj_from_str(des)); // OPTIMIZE!
  ret.type = Z_ERROBJ;//indicates an object in thrown state
  ret.ptr = (void*) p;
  return ret;
}


void module_addNativeFun(zmodule* m,const char* name,NativeFunPtr p)
{
  zobject val = zobj_from_function(name,p);
  StrMap_emplace(&(m->members),name,val);
}
void module_addSigNativeFun(zmodule* m,const char* name,NativeFunPtr p,const char* sig)
{
  zobject val = ZObjFromFunction(name,p);
  znativefun* fn = (znativefun*)val.ptr;
  fn->signature = sig;
  StrMap_emplace(&(m->members),name,val);
}

void klass_addNativeMethod(zclass* k,const char* name,NativeFunPtr p)
{
  zobject val = ZObjFromMethod(name,p,k);
  StrMap_emplace(&(k->members),name,val);
}
void klass_addSigNativeMethod(zclass* k,const char* name,NativeFunPtr p,const char* sig)
{
  zobject val = ZObjFromMethod(name,p,k);
  znativefun* fn = (znativefun*)val.ptr;
  fn->signature = sig;
  StrMap_emplace(&(k->members),name,val);
}