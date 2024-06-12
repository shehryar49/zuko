#include "bignum.h"
#include "klass.h"
#include "klassobject.h"
#include "zapi.h"
#include "zobject.h"
using namespace std;
#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif
zobject nil;
extern "C"
{

  EXPORT zobject init();
  EXPORT zobject bignum__construct(zobject*,int);
  EXPORT zobject bignum__add(zobject*,int);
  EXPORT zobject bignum__sub(zobject*,int);
  EXPORT zobject bignum__div(zobject*,int);
  EXPORT zobject bignum__mul(zobject*,int);
  EXPORT zobject bignum__smallerthan(zobject*,int);
  EXPORT zobject bignum__smallerthaneq(zobject*,int);
  EXPORT zobject bignum__greaterthan(zobject*,int);
  EXPORT zobject bignum__greaterthaneq(zobject*,int);
  EXPORT zobject bignum__eq(zobject*,int);
  EXPORT zobject bignum__noteq(zobject*,int);
  EXPORT zobject bignum__strval(zobject*,int);
  EXPORT zobject bignum__increment(zobject*,int);
  
  EXPORT zobject bignum__destroy(zobject*,int);
  
  zclass* bignumKlass;
  ////////
  //Implementations
  
  zobject bignum__construct(zobject* args,int n)
  {
    if(n!=2)
      return z_err(ArgumentError,"2 arguments needed!");
    if(args[0].type!='o' || args[1].type!='s')
      return z_err(TypeError,"Invalid argument types!");
    if(((zclass_object*)args[0].ptr)->_klass!=bignumKlass)
       return z_err(TypeError,"First argument must be an object of class bignum");
    bignum* b = new bignum(AS_STR(args[1]) -> val);
    if(!b)
      return z_err(Error,"Error allocating memory");
    zclass_object* k = (zclass_object*)args[0].ptr;
    StrMap_set(&(k->members),".ptr",zobj_from_ptr((void*)b));
    return nil;
  }
  zobject init()
  {
    nil.type = 'n';
    zmodule* d  = vm_alloc_zmodule();
    d->name = "bignum";
    bignumKlass = vm_alloc_zclass();
    bignumKlass->name = "bignum";
    zclass_add_method(bignumKlass,"__construct__",&bignum__construct);
    zclass_addmember(bignumKlass,".ptr",nil);
    zclass_add_method(bignumKlass,"__add__",&bignum__add);
    zclass_add_method(bignumKlass,"__sub__",&bignum__sub);
    zclass_add_method(bignumKlass,"__div__",&bignum__div);
    zclass_add_method(bignumKlass,"__mul__",&bignum__mul);
    zclass_add_method(bignumKlass,"__smallerthan__",&bignum__smallerthan);
    zclass_add_method(bignumKlass,"__smallerthaneq__",&bignum__smallerthaneq);
    zclass_add_method(bignumKlass,"__greaterthan__",&bignum__greaterthan);
    zclass_add_method(bignumKlass,"__greaterthaneq__",&bignum__greaterthaneq);
    zclass_add_method(bignumKlass,"__eq__",&bignum__eq);
    zclass_add_method(bignumKlass,"__noteq__",&bignum__noteq);
    zclass_add_method(bignumKlass,"strval",&bignum__strval);
    zclass_add_method(bignumKlass,"increment",&bignum__increment);
    zclass_add_method(bignumKlass,"__del__",&bignum__destroy);
   
    //add class to module
    zmodule_add_class(d,"bignum",bignumKlass);
    return zobj_from_module(d);
  }
  zobject bignum__add(zobject* args,int n)
  {
    if(n!=2)
      return z_err(ValueError,"2 arguments needed");
    if(args[0].type!='o' || args[1].type!='o')
      return  z_err(TypeError,"Error bignum object needed");
    if(((zclass_object*)args[0].ptr)->_klass!=bignumKlass ||((zclass_object*)args[0].ptr)->_klass!=bignumKlass)
      return  z_err(TypeError,"Error bignum object needed");
    zclass_object* self = (zclass_object*)args[0].ptr;
    zclass_object* rhs = (zclass_object*)args[1].ptr;
    bignum* a = (bignum*)AS_PTR(zclassobj_get(self,".ptr"));
    bignum* b = (bignum*)AS_PTR(zclassobj_get(rhs,".ptr"));
    bignum* c = new bignum((*a)+(*b));
    zclass_object* ret = vm_alloc_zclassobj(bignumKlass);
    zclassobj_set(ret,".ptr",zobj_from_ptr(c));
    //store new object in return result
    return zobj_from_classobj(ret);
  }
  zobject bignum__sub(zobject* args,int n)
  {
    if(n!=2)
      return  z_err(ValueError,"2 arguments needed");
    if(args[0].type!='o' || args[1].type!='o')
      return  z_err(TypeError,"Error bignum object needed");
    if(((zclass_object*)args[0].ptr)->_klass!=bignumKlass ||((zclass_object*)args[0].ptr)->_klass!=bignumKlass)
      return  z_err(TypeError,"Error bignum object needed");
    zclass_object* self = (zclass_object*)args[0].ptr;
    zclass_object* rhs = (zclass_object*)args[1].ptr;
    
    bignum* a = (bignum*)AS_PTR(zclassobj_get(self,".ptr"));
    bignum* b = (bignum*)AS_PTR(zclassobj_get(rhs,".ptr"));
    bignum* c = new bignum((*a)-(*b));
    zclass_object* ret = vm_alloc_zclassobj(bignumKlass);
    zclassobj_set(ret,".ptr",zobj_from_ptr(c));
    //store new object in return result
    return zobj_from_classobj(ret);
  }
  zobject bignum__div(zobject* args,int n)
  {
    if(n!=2)
      return z_err(ValueError,"2 arguments needed");
    if(args[0].type!='o' || args[1].type!='o')
      return z_err(TypeError,"Error bignum object needed"); 
    if(((zclass_object*)args[0].ptr)->_klass!=bignumKlass ||((zclass_object*)args[0].ptr)->_klass!=bignumKlass)
      return z_err(TypeError,"Error bignum object needed"); 
    
    zclass_object* self = (zclass_object*)args[0].ptr;
    zclass_object* rhs = (zclass_object*)args[1].ptr;
    bignum* a = (bignum*)AS_PTR(zclassobj_get(self,".ptr"));
    bignum* b = (bignum*)AS_PTR(zclassobj_get(rhs,".ptr"));
    bignum* c = new bignum((*a)/(*b));
    zclass_object* ret = vm_alloc_zclassobj(bignumKlass);
    zclassobj_set(ret,".ptr",zobj_from_ptr(c));
    //store new object in return result
    return zobj_from_classobj(ret);
  }
  zobject bignum__mul(zobject* args,int n)
  {
    if(n!=2)
      return  z_err(ValueError,"2 arguments needed");
    if(args[0].type!='o' || args[1].type!='o')
      return  z_err(TypeError,"Error bignum object needed");
    if(((zclass_object*)args[0].ptr)->_klass!=bignumKlass ||((zclass_object*)args[0].ptr)->_klass!=bignumKlass)
      return  z_err(TypeError,"Error bignum object needed");
    
    zclass_object* self = (zclass_object*)args[0].ptr;
    zclass_object* rhs = (zclass_object*)args[1].ptr;
    
    bignum* a = (bignum*)AS_PTR(zclassobj_get(self,".ptr"));
    bignum* b = (bignum*)AS_PTR(zclassobj_get(rhs,".ptr"));
    bignum* c = new bignum((*a)*(*b));
    zclass_object* ret = vm_alloc_zclassobj(bignumKlass);
    zclassobj_set(ret,".ptr",zobj_from_ptr(c));
    //store new object in return result
    return zobj_from_classobj(ret);
  }
  zobject bignum__eq(zobject* args,int n)
  {
    if(n!=2)
      return  z_err(ValueError,"2 arguments needed");
    if(args[0].type!='o' || args[1].type!='o')
      return  z_err(TypeError,"Error bignum object needed");
    if(((zclass_object*)args[0].ptr)->_klass!=bignumKlass ||((zclass_object*)args[0].ptr)->_klass!=bignumKlass)
      return  z_err(TypeError,"Error bignum object needed");
    
    zclass_object* self = (zclass_object*)args[0].ptr;
    zclass_object* rhs = (zclass_object*)args[1].ptr;
    
    bignum* a = (bignum*)AS_PTR(zclassobj_get(self,".ptr"));
    bignum* b = (bignum*)AS_PTR(zclassobj_get(rhs,".ptr"));
    zobject rr;
    rr.type = Z_BOOL;
    rr.i = *a == *b;
    return rr;
  }
  zobject bignum__noteq(zobject* args,int n)
  {
    if(n!=2)
      return  z_err(ValueError,"2 arguments needed");
    if(args[0].type!='o' || args[1].type!='o')
      return  z_err(TypeError,"Error bignum object needed");
    if(((zclass_object*)args[0].ptr)->_klass!=bignumKlass ||((zclass_object*)args[0].ptr)->_klass!=bignumKlass)
      return  z_err(TypeError,"Error bignum object needed");
    
    zclass_object* self = (zclass_object*)args[0].ptr;
    zclass_object* rhs = (zclass_object*)args[1].ptr;
    
    bignum* a = (bignum*)AS_PTR(zclassobj_get(self,".ptr"));
    bignum* b = (bignum*)AS_PTR(zclassobj_get(rhs,".ptr"));
    zobject rr;
    rr.type = Z_BOOL;
    rr.i = !(*a == *b);
    return rr;
  }
  zobject bignum__smallerthan(zobject* args,int n)
  {
    if(n!=2)
      return  z_err(ValueError,"2 arguments needed");
    if(args[0].type!='o' || args[1].type!='o')
      return  z_err(TypeError,"Error bignum object needed");
    if(((zclass_object*)args[0].ptr)->_klass!=bignumKlass ||((zclass_object*)args[0].ptr)->_klass!=bignumKlass)
      return  z_err(TypeError,"Error bignum object needed");
    
    zclass_object* self = (zclass_object*)args[0].ptr;
    zclass_object* rhs = (zclass_object*)args[1].ptr;
    
    bignum* a = (bignum*)AS_PTR(zclassobj_get(self,".ptr"));
    bignum* b = (bignum*)AS_PTR(zclassobj_get(rhs,".ptr"));
    zobject rr;
    rr.type = Z_BOOL;
    rr.i = (*a < *b);
    return rr;
  }
  zobject bignum__greaterthan(zobject* args,int n)
  {
    if(n!=2)
      return  z_err(ValueError,"2 arguments needed");
    if(args[0].type!='o' || args[1].type!='o')
      return  z_err(TypeError,"Error bignum object needed");
    if(((zclass_object*)args[0].ptr)->_klass!=bignumKlass ||((zclass_object*)args[0].ptr)->_klass!=bignumKlass)
      return  z_err(TypeError,"Error bignum object needed");
    
    zclass_object* self = (zclass_object*)args[0].ptr;
    zclass_object* rhs = (zclass_object*)args[1].ptr;
    
    bignum* a = (bignum*)AS_PTR(zclassobj_get(self,".ptr"));
    bignum* b = (bignum*)AS_PTR(zclassobj_get(rhs,".ptr"));
    zobject rr;
    rr.type = Z_BOOL;
    rr.i = (*a > *b);
    return rr;
  }
  //
  zobject bignum__smallerthaneq(zobject* args,int n)
  {
    if(n!=2)
      return  z_err(ValueError,"2 arguments needed");
    if(args[0].type!='o' || args[1].type!='o')
      return  z_err(TypeError,"Error bignum object needed");
    if(((zclass_object*)args[0].ptr)->_klass!=bignumKlass ||((zclass_object*)args[0].ptr)->_klass!=bignumKlass)
      return  z_err(TypeError,"Error bignum object needed");
    zclass_object* self = (zclass_object*)args[0].ptr;
    zclass_object* rhs = (zclass_object*)args[1].ptr;
    
    bignum* a = (bignum*)AS_PTR(zclassobj_get(self,".ptr"));
    bignum* b = (bignum*)AS_PTR(zclassobj_get(rhs,".ptr"));
    zobject rr;
    rr.type = Z_BOOL;
    rr.i = (*a < *b || *a == *b);
    return rr;
  }
  zobject bignum__greaterthaneq(zobject* args,int n)
  {
    if(n!=2)
      return  z_err(ValueError,"2 arguments needed");
    if(args[0].type!='o' || args[1].type!='o')
      return  z_err(TypeError,"Error bignum object needed");
    if(((zclass_object*)args[0].ptr)->_klass!=bignumKlass ||((zclass_object*)args[0].ptr)->_klass!=bignumKlass)
      return  z_err(TypeError,"Error bignum object needed");
    
    zclass_object* self = (zclass_object*)args[0].ptr;
    zclass_object* rhs = (zclass_object*)args[1].ptr;
    
    bignum* a = (bignum*)AS_PTR(zclassobj_get(self,".ptr"));
    bignum* b = (bignum*)AS_PTR(zclassobj_get(rhs,".ptr"));
    zobject rr;
    rr.type = Z_BOOL;
    rr.i = (*a > *b || *a == *b);
    return rr;
  }
  zobject bignum__destroy(zobject* args,int n)
  {
    zclass_object* d = (zclass_object*)args[0].ptr;
    zobject ptr = nil;
    StrMap_get(&(d->members),".ptr",&ptr);
    if(ptr.type!=Z_POINTER)
      return nil;  
    bignum* a = (bignum*)ptr.ptr;
    delete a;
    return nil;
  }
  zobject bignum__strval(zobject* args,int n)
  {
    if(n!=1)
      return z_err(ArgumentError,"1 argument needed");
    if(args[0].type!='o' || ((zclass_object*)args[0].ptr)->_klass!=bignumKlass)
      return z_err(TypeError,"Error bignum object needed!");
    zclass_object* self = (zclass_object*)args[0].ptr;
    bignum a = *(bignum*)AS_PTR(zclassobj_get(self,".ptr"));
    return zobj_from_str(a.str().c_str());
  }
  zobject bignum__increment(zobject* args,int32_t n)
  {
     if(n!=1)
      return z_err(ArgumentError,"1 argument needed");
    if(args[0].type!=Z_OBJ || ((zclass_object*)args[0].ptr)->_klass!=bignumKlass)
      return z_err(TypeError,"Error bignum object needed!");
    zclass_object* self = (zclass_object*)args[0].ptr;
    zobject ptr;
    StrMap_get(&(self->members),".ptr",&ptr);
    bignum& a = *(bignum*)ptr.ptr;
    a.increment();
    return nil;
  }
}
