#include "bignum.h"
#include "zobject.h"
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
    bignum* b = new bignum(*((string*)args[1].ptr));
    if(!b)
      return Z_Err(Error,"Error allocating memory");
    KlassObject* k = (KlassObject*)args[0].ptr;
    k->members[".ptr"]  = ZObjFromPtr((void*)b);
    return nil;
  }
  ZObject init()
  {
      nil.type = 'n';
    Module* d  = vm_allocModule();
    d->name = "bignum";
    bignumKlass = vm_allocKlass();
    bignumKlass->name = "bignum";
    bignumKlass->members.emplace("__construct__",ZObjFromMethod("__construct__",&bignum__construct,bignumKlass));
    bignumKlass->members.emplace(".ptr",nil);
    bignumKlass->members.emplace("__add__",ZObjFromMethod("__add__",&bignum__add,bignumKlass));
    bignumKlass->members.emplace("__sub__",ZObjFromMethod("__sub__",&bignum__sub,bignumKlass));
    bignumKlass->members.emplace("__div__",ZObjFromMethod("__div__",&bignum__div,bignumKlass));
    bignumKlass->members.emplace("__mul__",ZObjFromMethod("__mul__",&bignum__mul,bignumKlass));
    bignumKlass->members.emplace("__smallerthan__",ZObjFromMethod("__smallerthan__",&bignum__smallerthan,bignumKlass));
    bignumKlass->members.emplace("__smallerthaneq__",ZObjFromMethod("__smallerthaneq__",&bignum__smallerthaneq,bignumKlass));
    bignumKlass->members.emplace("__greaterthan__",ZObjFromMethod("__greaterthan__",&bignum__greaterthan,bignumKlass));
    bignumKlass->members.emplace("__greaterthaneq__",ZObjFromMethod("__greaterthaneq__",&bignum__greaterthaneq,bignumKlass));
    bignumKlass->members.emplace("__eq__",ZObjFromMethod("__eq__",&bignum__eq,bignumKlass));
    bignumKlass->members.emplace("__noteq__",ZObjFromMethod("__noteq__",&bignum__noteq,bignumKlass));
    bignumKlass->members.emplace("strval",ZObjFromMethod("strval",&bignum__strval,bignumKlass));
    bignumKlass->members.emplace("increment",ZObjFromMethod("increment",&bignum__increment,bignumKlass));
    bignumKlass->members.emplace("__del__",ZObjFromMethod("__del__",&bignum__destroy,bignumKlass));
   
    //add class to module
    d->members.emplace("bignum",ZObjFromKlass(bignumKlass));
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
    bignum* a = (bignum*)self->members[".ptr"].ptr;
    bignum* b = (bignum*)rhs->members[".ptr"].ptr;
    bignum* c = new bignum((*a)+(*b));
    KlassObject* ret = vm_allocKlassObject();
    ret->klass = bignumKlass;
    ret->members = bignumKlass->members;
    ret->members[".ptr"] = ZObjFromPtr(c);
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
    
    bignum* a = (bignum*)self->members[".ptr"].ptr;
    bignum* b = (bignum*)rhs->members[".ptr"].ptr;
    bignum* c = new bignum((*a)-(*b));
    KlassObject* ret = vm_allocKlassObject();
    ret->klass = bignumKlass;
    ret->members = bignumKlass->members;
    ret->members[".ptr"] = ZObjFromPtr(c);
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
    bignum* a = (bignum*)self->members[".ptr"].ptr;
    bignum* b = (bignum*)rhs->members[".ptr"].ptr;
    bignum* c = new bignum((*a)/(*b));
    KlassObject* ret = vm_allocKlassObject();
    ret->klass = bignumKlass;
    ret->members = bignumKlass->members;
    ret->members[".ptr"] = ZObjFromPtr(c);
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
    
    bignum* a = (bignum*)self->members[".ptr"].ptr;
    bignum* b = (bignum*)rhs->members[".ptr"].ptr;
    bignum* c = new bignum((*a)*(*b));
    KlassObject* ret = vm_allocKlassObject();
    ret->klass = bignumKlass;
    ret->members = bignumKlass->members;
    ret->members[".ptr"] = ZObjFromPtr(c);
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
    
    bignum* a = (bignum*)self->members[".ptr"].ptr;
    bignum* b = (bignum*)rhs->members[".ptr"].ptr;
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
    
    bignum* a = (bignum*)self->members[".ptr"].ptr;
    bignum* b = (bignum*)rhs->members[".ptr"].ptr;
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
    
    bignum* a = (bignum*)self->members[".ptr"].ptr;
    bignum* b = (bignum*)rhs->members[".ptr"].ptr;
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
    
    bignum* a = (bignum*)self->members[".ptr"].ptr;
    bignum* b = (bignum*)rhs->members[".ptr"].ptr;
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
    
    bignum* a = (bignum*)self->members[".ptr"].ptr;
    bignum* b = (bignum*)rhs->members[".ptr"].ptr;
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
    
    bignum* a = (bignum*)self->members[".ptr"].ptr;
    bignum* b = (bignum*)rhs->members[".ptr"].ptr;
    ZObject rr;
    rr.type = Z_BOOL;
    rr.i = (*a > *b || *a == *b);
    return rr;
  }
  ZObject bignum__destroy(ZObject* args,int n)
  {
    KlassObject* d = (KlassObject*)args[0].ptr;
    if(d->members[".ptr"].type!=Z_POINTER)
      return nil;  
    bignum* a = (bignum*)d->members[".ptr"].ptr;
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
    bignum a = *(bignum*)self->members[".ptr"].ptr;
    return ZObjFromStr(a.str());
  }
  ZObject bignum__increment(ZObject* args,int32_t n)
  {
     if(n!=1)
      return Z_Err(ArgumentError,"1 argument needed");
    if(args[0].type!=Z_OBJ || ((KlassObject*)args[0].ptr)->klass!=bignumKlass)
      return Z_Err(TypeError,"Error bignum object needed!");
    KlassObject* self = (KlassObject*)args[0].ptr;
    bignum& a = *(bignum*)self->members[".ptr"].ptr;
    a.increment();
    return nil;
  }
}
