#include "bignum.h"
#include "zapi.h"
using namespace std;
#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif
ZObject nil;
extern "C"
{

  EXPORT ZObject init();
  EXPORT ZObject bignum__construct(ZObject*,int);
  EXPORT ZObject bignum__add(ZObject*,int);
  EXPORT ZObject bignum__sub(ZObject*,int);
  EXPORT ZObject bignum__div(ZObject*,int);
  EXPORT ZObject bignum__mul(ZObject*,int);
  EXPORT ZObject bignum__smallerthan(ZObject*,int);
  EXPORT ZObject bignum__smallerthaneq(ZObject*,int);
  EXPORT ZObject bignum__greaterthan(ZObject*,int);
  EXPORT ZObject bignum__greaterthaneq(ZObject*,int);
  EXPORT ZObject bignum__eq(ZObject*,int);
  EXPORT ZObject bignum__noteq(ZObject*,int);
  EXPORT ZObject bignum__strval(ZObject*,int);
  EXPORT ZObject bignum__increment(ZObject*,int);
  
  EXPORT ZObject bignum__destroy(ZObject*,int);
  
  Klass* bignumKlass;
  ////////
  //Implementations
  
  ZObject bignum__construct(ZObject* args,int n)
  {
    if(n!=2)
      return Z_Err(ArgumentError,"2 arguments needed!");
    if(args[0].type!='o' || args[1].type!='s')
      return Z_Err(TypeError,"Invalid argument types!");
    if(((KlassObject*)args[0].ptr)->klass!=bignumKlass)
       return Z_Err(TypeError,"First argument must be an object of class bignum");
    bignum* b = new bignum(AS_STR(args[1]) -> val);
    if(!b)
      return Z_Err(Error,"Error allocating memory");
    KlassObject* k = (KlassObject*)args[0].ptr;
    StrMap_set(&(k->members),".ptr",ZObjFromPtr((void*)b));
    return nil;
  }
  ZObject init()
  {
    nil.type = 'n';
    Module* d  = vm_allocModule();
    d->name = "bignum";
    bignumKlass = vm_allocKlass();
    bignumKlass->name = "bignum";
    Klass_addNativeMethod(bignumKlass,"__construct__",&bignum__construct);
    Klass_addMember(bignumKlass,".ptr",nil);
    Klass_addNativeMethod(bignumKlass,"__add__",&bignum__add);
    Klass_addNativeMethod(bignumKlass,"__sub__",&bignum__sub);
    Klass_addNativeMethod(bignumKlass,"__div__",&bignum__div);
    Klass_addNativeMethod(bignumKlass,"__mul__",&bignum__mul);
    Klass_addNativeMethod(bignumKlass,"__smallerthan__",&bignum__smallerthan);
    Klass_addNativeMethod(bignumKlass,"__smallerthaneq__",&bignum__smallerthaneq);
    Klass_addNativeMethod(bignumKlass,"__greaterthan__",&bignum__greaterthan);
    Klass_addNativeMethod(bignumKlass,"__greaterthaneq__",&bignum__greaterthaneq);
    Klass_addNativeMethod(bignumKlass,"__eq__",&bignum__eq);
    Klass_addNativeMethod(bignumKlass,"__noteq__",&bignum__noteq);
    Klass_addNativeMethod(bignumKlass,"strval",&bignum__strval);
    Klass_addNativeMethod(bignumKlass,"increment",&bignum__increment);
    Klass_addNativeMethod(bignumKlass,"__del__",&bignum__destroy);
   
    //add class to module
    Module_addKlass(d,"bignum",bignumKlass);
    return ZObjFromModule(d);
  }
  ZObject bignum__add(ZObject* args,int n)
  {
    if(n!=2)
      return Z_Err(ValueError,"2 arguments needed");
    if(args[0].type!='o' || args[1].type!='o')
      return  Z_Err(TypeError,"Error bignum object needed");
    if(((KlassObject*)args[0].ptr)->klass!=bignumKlass ||((KlassObject*)args[0].ptr)->klass!=bignumKlass)
      return  Z_Err(TypeError,"Error bignum object needed");
    KlassObject* self = (KlassObject*)args[0].ptr;
    KlassObject* rhs = (KlassObject*)args[1].ptr;
    bignum* a = (bignum*)AS_PTR(KlassObj_getMember(self,".ptr"));
    bignum* b = (bignum*)AS_PTR(KlassObj_getMember(rhs,".ptr"));
    bignum* c = new bignum((*a)+(*b));
    KlassObject* ret = vm_allocKlassObject(bignumKlass);
    KlassObj_setMember(ret,".ptr",ZObjFromPtr(c));
    //store new object in return result
    return ZObjFromKlassObj(ret);
  }
  ZObject bignum__sub(ZObject* args,int n)
  {
    if(n!=2)
      return  Z_Err(ValueError,"2 arguments needed");
    if(args[0].type!='o' || args[1].type!='o')
      return  Z_Err(TypeError,"Error bignum object needed");
    if(((KlassObject*)args[0].ptr)->klass!=bignumKlass ||((KlassObject*)args[0].ptr)->klass!=bignumKlass)
      return  Z_Err(TypeError,"Error bignum object needed");
    KlassObject* self = (KlassObject*)args[0].ptr;
    KlassObject* rhs = (KlassObject*)args[1].ptr;
    
    bignum* a = (bignum*)AS_PTR(KlassObj_getMember(self,".ptr"));
    bignum* b = (bignum*)AS_PTR(KlassObj_getMember(rhs,".ptr"));
    bignum* c = new bignum((*a)-(*b));
    KlassObject* ret = vm_allocKlassObject(bignumKlass);
    KlassObj_setMember(ret,".ptr",ZObjFromPtr(c));
    //store new object in return result
    return ZObjFromKlassObj(ret);
  }
  ZObject bignum__div(ZObject* args,int n)
  {
    if(n!=2)
      return Z_Err(ValueError,"2 arguments needed");
    if(args[0].type!='o' || args[1].type!='o')
      return Z_Err(TypeError,"Error bignum object needed"); 
    if(((KlassObject*)args[0].ptr)->klass!=bignumKlass ||((KlassObject*)args[0].ptr)->klass!=bignumKlass)
      return Z_Err(TypeError,"Error bignum object needed"); 
    
    KlassObject* self = (KlassObject*)args[0].ptr;
    KlassObject* rhs = (KlassObject*)args[1].ptr;
    bignum* a = (bignum*)AS_PTR(KlassObj_getMember(self,".ptr"));
    bignum* b = (bignum*)AS_PTR(KlassObj_getMember(rhs,".ptr"));
    bignum* c = new bignum((*a)/(*b));
    KlassObject* ret = vm_allocKlassObject(bignumKlass);
    KlassObj_setMember(ret,".ptr",ZObjFromPtr(c));
    //store new object in return result
    return ZObjFromKlassObj(ret);
  }
  ZObject bignum__mul(ZObject* args,int n)
  {
    if(n!=2)
      return  Z_Err(ValueError,"2 arguments needed");
    if(args[0].type!='o' || args[1].type!='o')
      return  Z_Err(TypeError,"Error bignum object needed");
    if(((KlassObject*)args[0].ptr)->klass!=bignumKlass ||((KlassObject*)args[0].ptr)->klass!=bignumKlass)
      return  Z_Err(TypeError,"Error bignum object needed");
    
    KlassObject* self = (KlassObject*)args[0].ptr;
    KlassObject* rhs = (KlassObject*)args[1].ptr;
    
    bignum* a = (bignum*)AS_PTR(KlassObj_getMember(self,".ptr"));
    bignum* b = (bignum*)AS_PTR(KlassObj_getMember(rhs,".ptr"));
    bignum* c = new bignum((*a)*(*b));
    KlassObject* ret = vm_allocKlassObject(bignumKlass);
    KlassObj_setMember(ret,".ptr",ZObjFromPtr(c));
    //store new object in return result
    return ZObjFromKlassObj(ret);
  }
  ZObject bignum__eq(ZObject* args,int n)
  {
    if(n!=2)
      return  Z_Err(ValueError,"2 arguments needed");
    if(args[0].type!='o' || args[1].type!='o')
      return  Z_Err(TypeError,"Error bignum object needed");
    if(((KlassObject*)args[0].ptr)->klass!=bignumKlass ||((KlassObject*)args[0].ptr)->klass!=bignumKlass)
      return  Z_Err(TypeError,"Error bignum object needed");
    
    KlassObject* self = (KlassObject*)args[0].ptr;
    KlassObject* rhs = (KlassObject*)args[1].ptr;
    
    bignum* a = (bignum*)AS_PTR(KlassObj_getMember(self,".ptr"));
    bignum* b = (bignum*)AS_PTR(KlassObj_getMember(rhs,".ptr"));
    ZObject rr;
    rr.type = Z_BOOL;
    rr.i = *a == *b;
    return rr;
  }
  ZObject bignum__noteq(ZObject* args,int n)
  {
    if(n!=2)
      return  Z_Err(ValueError,"2 arguments needed");
    if(args[0].type!='o' || args[1].type!='o')
      return  Z_Err(TypeError,"Error bignum object needed");
    if(((KlassObject*)args[0].ptr)->klass!=bignumKlass ||((KlassObject*)args[0].ptr)->klass!=bignumKlass)
      return  Z_Err(TypeError,"Error bignum object needed");
    
    KlassObject* self = (KlassObject*)args[0].ptr;
    KlassObject* rhs = (KlassObject*)args[1].ptr;
    
    bignum* a = (bignum*)AS_PTR(KlassObj_getMember(self,".ptr"));
    bignum* b = (bignum*)AS_PTR(KlassObj_getMember(rhs,".ptr"));
    ZObject rr;
    rr.type = Z_BOOL;
    rr.i = !(*a == *b);
    return rr;
  }
  ZObject bignum__smallerthan(ZObject* args,int n)
  {
    if(n!=2)
      return  Z_Err(ValueError,"2 arguments needed");
    if(args[0].type!='o' || args[1].type!='o')
      return  Z_Err(TypeError,"Error bignum object needed");
    if(((KlassObject*)args[0].ptr)->klass!=bignumKlass ||((KlassObject*)args[0].ptr)->klass!=bignumKlass)
      return  Z_Err(TypeError,"Error bignum object needed");
    
    KlassObject* self = (KlassObject*)args[0].ptr;
    KlassObject* rhs = (KlassObject*)args[1].ptr;
    
    bignum* a = (bignum*)AS_PTR(KlassObj_getMember(self,".ptr"));
    bignum* b = (bignum*)AS_PTR(KlassObj_getMember(rhs,".ptr"));
    ZObject rr;
    rr.type = Z_BOOL;
    rr.i = (*a < *b);
    return rr;
  }
  ZObject bignum__greaterthan(ZObject* args,int n)
  {
    if(n!=2)
      return  Z_Err(ValueError,"2 arguments needed");
    if(args[0].type!='o' || args[1].type!='o')
      return  Z_Err(TypeError,"Error bignum object needed");
    if(((KlassObject*)args[0].ptr)->klass!=bignumKlass ||((KlassObject*)args[0].ptr)->klass!=bignumKlass)
      return  Z_Err(TypeError,"Error bignum object needed");
    
    KlassObject* self = (KlassObject*)args[0].ptr;
    KlassObject* rhs = (KlassObject*)args[1].ptr;
    
    bignum* a = (bignum*)AS_PTR(KlassObj_getMember(self,".ptr"));
    bignum* b = (bignum*)AS_PTR(KlassObj_getMember(rhs,".ptr"));
    ZObject rr;
    rr.type = Z_BOOL;
    rr.i = (*a > *b);
    return rr;
  }
  //
  ZObject bignum__smallerthaneq(ZObject* args,int n)
  {
    if(n!=2)
      return  Z_Err(ValueError,"2 arguments needed");
    if(args[0].type!='o' || args[1].type!='o')
      return  Z_Err(TypeError,"Error bignum object needed");
    if(((KlassObject*)args[0].ptr)->klass!=bignumKlass ||((KlassObject*)args[0].ptr)->klass!=bignumKlass)
      return  Z_Err(TypeError,"Error bignum object needed");
    KlassObject* self = (KlassObject*)args[0].ptr;
    KlassObject* rhs = (KlassObject*)args[1].ptr;
    
    bignum* a = (bignum*)AS_PTR(KlassObj_getMember(self,".ptr"));
    bignum* b = (bignum*)AS_PTR(KlassObj_getMember(rhs,".ptr"));
    ZObject rr;
    rr.type = Z_BOOL;
    rr.i = (*a < *b || *a == *b);
    return rr;
  }
  ZObject bignum__greaterthaneq(ZObject* args,int n)
  {
    if(n!=2)
      return  Z_Err(ValueError,"2 arguments needed");
    if(args[0].type!='o' || args[1].type!='o')
      return  Z_Err(TypeError,"Error bignum object needed");
    if(((KlassObject*)args[0].ptr)->klass!=bignumKlass ||((KlassObject*)args[0].ptr)->klass!=bignumKlass)
      return  Z_Err(TypeError,"Error bignum object needed");
    
    KlassObject* self = (KlassObject*)args[0].ptr;
    KlassObject* rhs = (KlassObject*)args[1].ptr;
    
    bignum* a = (bignum*)AS_PTR(KlassObj_getMember(self,".ptr"));
    bignum* b = (bignum*)AS_PTR(KlassObj_getMember(rhs,".ptr"));
    ZObject rr;
    rr.type = Z_BOOL;
    rr.i = (*a > *b || *a == *b);
    return rr;
  }
  ZObject bignum__destroy(ZObject* args,int n)
  {
    KlassObject* d = (KlassObject*)args[0].ptr;
    ZObject ptr = nil;
    StrMap_get(&(d->members),".ptr",&ptr);
    if(ptr.type!=Z_POINTER)
      return nil;  
    bignum* a = (bignum*)ptr.ptr;
    delete a;
    return nil;
  }
  ZObject bignum__strval(ZObject* args,int n)
  {
    if(n!=1)
      return Z_Err(ArgumentError,"1 argument needed");
    if(args[0].type!='o' || ((KlassObject*)args[0].ptr)->klass!=bignumKlass)
      return Z_Err(TypeError,"Error bignum object needed!");
    KlassObject* self = (KlassObject*)args[0].ptr;
    bignum a = *(bignum*)AS_PTR(KlassObj_getMember(self,".ptr"));
    return ZObjFromStr(a.str().c_str());
  }
  ZObject bignum__increment(ZObject* args,int32_t n)
  {
     if(n!=1)
      return Z_Err(ArgumentError,"1 argument needed");
    if(args[0].type!=Z_OBJ || ((KlassObject*)args[0].ptr)->klass!=bignumKlass)
      return Z_Err(TypeError,"Error bignum object needed!");
    KlassObject* self = (KlassObject*)args[0].ptr;
    ZObject ptr;
    StrMap_get(&(self->members),".ptr",&ptr);
    bignum& a = *(bignum*)ptr.ptr;
    a.increment();
    return nil;
  }
}
