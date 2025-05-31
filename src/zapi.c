#define ZUKO_BUILDING_MODULE
#include "zapi.h"
#include "zobject.h"
#include "apiver.h"
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




//Allocator function pointers
fn1 vm_alloc_zlist;
fn2 vm_alloc_zdict;
fn3 vm_alloc_zstr;
fn4 vm_allocMutString;
fn5 vm_alloc_zfile;
fn6 vm_alloc_zclass;
fn7 vm_alloc_zclassobj;
fn8 vm_alloc_znativefun;
fn9 vm_alloc_zmodule;
fn10 vm_alloc_zbytearr;
fn11 vm_call_object;
fn12 vm_mark_important;
fn13 vm_unmark_important;
fn14 vm_increment_allocated;
fn14 vm_decrement_allocated;

//Api setup
int api_setup(api_functions* p,int ver)
{
    if(ver != ZUKO_API_VERSION)
        return 0;  
    vm_alloc_zlist = p->a1;
    vm_alloc_zdict = p->a2;
    vm_alloc_zstr = p->a3;
    vm_allocMutString = p->a4;
    vm_alloc_zfile = p->a5;
    vm_alloc_zclass = p->a6;
    vm_alloc_zclassobj = p->a7;
    vm_alloc_znativefun = p->a8;
    vm_alloc_zmodule = p->a9;
    vm_alloc_zbytearr = p->a10;
    vm_call_object = p->a11;
    vm_mark_important = p->a12;
    vm_unmark_important = p->a13;
    vm_increment_allocated = p->a14;
    vm_decrement_allocated = p->a15;
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
    return 1;
}


//These helper functions utilized allocator functions
//that is why they are defined here
zclass* zclass_make_derived(zclass* base)
{
  zclass* child = vm_alloc_zclass(NULL);
  zobject super;
  super.type = Z_CLASS;
  super.ptr = (void*)base;
  strmap_assign(&(child->members),&(base->members));
  zclass_setmember(child,"super",super);
  return child;
}

zobject zobj_from_str(const char* str)// makes deep copy of str
{
  size_t len = strlen(str);
  zstr* ptr = vm_alloc_zstr(len);
  memcpy(ptr->val,str,len*sizeof(char));
  zobject ret;
  ret.type = 's';
  ret.ptr = (void*)ptr;
  return ret;
}
zobject zobj_from_method(const char* name,NativeFunPtr r,zclass* k)
{
  zobject ret;
  znativefun* fn = vm_alloc_znativefun();
  fn->name = name;
  fn->_klass = k;
  fn->addr = r;
  ret.type = Z_NATIVE_FUNC;
  ret.ptr = (void*)fn;
  return ret;
}
zobject zobj_from_function(const char* name,NativeFunPtr r)
{
  zobject ret;
  znativefun* fn = vm_alloc_znativefun();
  fn->name = name;
  fn->addr = r;
  ret.type = Z_NATIVE_FUNC;
  ret.ptr = (void*)fn;
  return ret;
}


zobject z_err(zclass* errklass,const char* des)
{
  zobject ret;
  zclass_object* p = vm_alloc_zclassobj(errklass);
  strmap_set(&(p->members),"msg",zobj_from_str(des)); // OPTIMIZE!
  ret.type = Z_ERROBJ;//indicates an object in thrown state
  ret.ptr = (void*) p;
  return ret;
}


void zmodule_add_fun(zmodule* m,const char* name,NativeFunPtr p)
{
  zobject val = zobj_from_function(name,p);
  strmap_emplace(&(m->members),name,val);
}
void zmodule_add_sig_fun(zmodule* m,const char* name,NativeFunPtr p,const char* sig)
{
  zobject val = zobj_from_function(name,p);
  znativefun* fn = (znativefun*)val.ptr;
  fn->signature = sig;
  strmap_emplace(&(m->members),name,val);
}

void zclass_add_method(zclass* k,const char* name,NativeFunPtr p)
{
  zobject val = zobj_from_method(name,p,k);
  strmap_emplace(&(k->members),name,val);
}
void zclass_add_sig_method(zclass* k,const char* name,NativeFunPtr p,const char* sig)
{
  zobject val = zobj_from_method(name,p,k);
  znativefun* fn = (znativefun*)val.ptr;
  fn->signature = sig;
  strmap_emplace(&(k->members),name,val);
}
