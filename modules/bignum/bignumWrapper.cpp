#include "bignum.h"
#include "C:\\plutonium\\PltObject.h"
using namespace std;
#define EXPORT __declspec(dllexport)
PltObject nil;
extern "C"
{

  EXPORT PltObject init();
  EXPORT PltObject bignum__construct(PltObject*,int);
  EXPORT PltObject bignum__add(PltObject*,int);
  EXPORT PltObject bignum__sub(PltObject*,int);
  EXPORT PltObject bignum__div(PltObject*,int);
  EXPORT PltObject bignum__mul(PltObject*,int);
  EXPORT PltObject bignum__smallerthan(PltObject*,int);
  EXPORT PltObject bignum__smallerthaneq(PltObject*,int);
  EXPORT PltObject bignum__greaterthan(PltObject*,int);
  EXPORT PltObject bignum__greaterthaneq(PltObject*,int);
  EXPORT PltObject bignum__eq(PltObject*,int);
  EXPORT PltObject bignum__noteq(PltObject*,int);
  EXPORT PltObject bignum__strval(PltObject*,int);
  EXPORT PltObject bignum__increment(PltObject*,int);
  
  EXPORT PltObject bignum__destroy(PltObject*,int);
  
  Klass* bignumKlass;
  ////////
  //Implementations
  
  PltObject bignum__construct(PltObject* args,int n)
  {
    if(n!=2)
      return Plt_Err(ArgumentError,"2 arguments needed!");
    if(args[0].type!='o' || args[1].type!='s')
      return Plt_Err(TypeError,"Invalid argument types!");
    if(((KlassObject*)args[0].ptr)->klass!=bignumKlass)
       return Plt_Err(TypeError,"First argument must be an object of class bignum");
    bignum* b = new bignum(*((string*)args[1].ptr));
    if(!b)
      return Plt_Err(Error,"Error allocating memory");
    KlassObject* k = (KlassObject*)args[0].ptr;
    k->members[".ptr"]  = PObjFromPtr((void*)b);
    return nil;
  }
  PltObject init()
  {
      nil.type = 'n';
    Module* d  = vm_allocModule();
    d->name = "bignum";
    bignumKlass = vm_allocKlass();
    bignumKlass->name = "bignum";
    bignumKlass->members.emplace("__construct__",PObjFromMethod("__construct__",&bignum__construct,bignumKlass));
    bignumKlass->members.emplace(".ptr",nil);
    bignumKlass->members.emplace("__add__",PObjFromMethod("__add__",&bignum__add,bignumKlass));
    bignumKlass->members.emplace("__sub__",PObjFromMethod("__sub__",&bignum__sub,bignumKlass));
    bignumKlass->members.emplace("__div__",PObjFromMethod("__div__",&bignum__div,bignumKlass));
    bignumKlass->members.emplace("__mul__",PObjFromMethod("__mul__",&bignum__mul,bignumKlass));
    bignumKlass->members.emplace("__smallerthan__",PObjFromMethod("__smallerthan__",&bignum__smallerthan,bignumKlass));
    bignumKlass->members.emplace("__smallerthaneq__",PObjFromMethod("__smallerthaneq__",&bignum__smallerthaneq,bignumKlass));
    bignumKlass->members.emplace("__greaterthan__",PObjFromMethod("__greaterthan__",&bignum__greaterthan,bignumKlass));
    bignumKlass->members.emplace("__greaterthaneq__",PObjFromMethod("__greaterthaneq__",&bignum__greaterthaneq,bignumKlass));
    bignumKlass->members.emplace("__eq__",PObjFromMethod("__eq__",&bignum__eq,bignumKlass));
    bignumKlass->members.emplace("__noteq__",PObjFromMethod("__noteq__",&bignum__noteq,bignumKlass));
    bignumKlass->members.emplace("strval",PObjFromMethod("strval",&bignum__strval,bignumKlass));
    bignumKlass->members.emplace("increment",PObjFromMethod("increment",&bignum__increment,bignumKlass));
    bignumKlass->members.emplace("__del__",PObjFromMethod("__del__",&bignum__destroy,bignumKlass));
   
    //add class to module
    d->members.emplace("bignum",PObjFromKlass(bignumKlass));
    return PObjFromModule(d);
  }
  PltObject bignum__add(PltObject* args,int n)
  {
    if(n!=2)
      return Plt_Err(ValueError,"2 arguments needed");
    if(args[0].type!='o' || args[1].type!='o')
      return  Plt_Err(TypeError,"Error bignum object needed");
    if(((KlassObject*)args[0].ptr)->klass!=bignumKlass ||((KlassObject*)args[0].ptr)->klass!=bignumKlass)
      return  Plt_Err(TypeError,"Error bignum object needed");
    KlassObject* self = (KlassObject*)args[0].ptr;
    KlassObject* rhs = (KlassObject*)args[1].ptr;
    bignum* a = (bignum*)self->members[".ptr"].ptr;
    bignum* b = (bignum*)rhs->members[".ptr"].ptr;
    bignum* c = new bignum((*a)+(*b));
    KlassObject* ret = vm_allocKlassObject();
    ret->klass = bignumKlass;
    ret->members = bignumKlass->members;
    ret->members[".ptr"] = PObjFromPtr(c);
    //store new object in return result
    return PObjFromKlassObj(ret);
  }
  PltObject bignum__sub(PltObject* args,int n)
  {
    if(n!=2)
      return  Plt_Err(ValueError,"2 arguments needed");
    if(args[0].type!='o' || args[1].type!='o')
      return  Plt_Err(TypeError,"Error bignum object needed");
    if(((KlassObject*)args[0].ptr)->klass!=bignumKlass ||((KlassObject*)args[0].ptr)->klass!=bignumKlass)
      return  Plt_Err(TypeError,"Error bignum object needed");
    KlassObject* self = (KlassObject*)args[0].ptr;
    KlassObject* rhs = (KlassObject*)args[1].ptr;
    
    bignum* a = (bignum*)self->members[".ptr"].ptr;
    bignum* b = (bignum*)rhs->members[".ptr"].ptr;
    bignum* c = new bignum((*a)-(*b));
    KlassObject* ret = vm_allocKlassObject();
    ret->klass = bignumKlass;
    ret->members = bignumKlass->members;
    ret->members[".ptr"] = PObjFromPtr(c);
    //store new object in return result
    return PObjFromKlassObj(ret);
  }
  PltObject bignum__div(PltObject* args,int n)
  {
    if(n!=2)
      return Plt_Err(ValueError,"2 arguments needed");
    if(args[0].type!='o' || args[1].type!='o')
      return Plt_Err(TypeError,"Error bignum object needed"); 
    if(((KlassObject*)args[0].ptr)->klass!=bignumKlass ||((KlassObject*)args[0].ptr)->klass!=bignumKlass)
      return Plt_Err(TypeError,"Error bignum object needed"); 
    
    KlassObject* self = (KlassObject*)args[0].ptr;
    KlassObject* rhs = (KlassObject*)args[1].ptr;
    bignum* a = (bignum*)self->members[".ptr"].ptr;
    bignum* b = (bignum*)rhs->members[".ptr"].ptr;
    bignum* c = new bignum((*a)/(*b));
    KlassObject* ret = vm_allocKlassObject();
    ret->klass = bignumKlass;
    ret->members = bignumKlass->members;
    ret->members[".ptr"] = PObjFromPtr(c);
    //store new object in return result
    return PObjFromKlassObj(ret);
  }
  PltObject bignum__mul(PltObject* args,int n)
  {
    if(n!=2)
      return  Plt_Err(ValueError,"2 arguments needed");
    if(args[0].type!='o' || args[1].type!='o')
      return  Plt_Err(TypeError,"Error bignum object needed");
    if(((KlassObject*)args[0].ptr)->klass!=bignumKlass ||((KlassObject*)args[0].ptr)->klass!=bignumKlass)
      return  Plt_Err(TypeError,"Error bignum object needed");
    
    KlassObject* self = (KlassObject*)args[0].ptr;
    KlassObject* rhs = (KlassObject*)args[1].ptr;
    
    bignum* a = (bignum*)self->members[".ptr"].ptr;
    bignum* b = (bignum*)rhs->members[".ptr"].ptr;
    bignum* c = new bignum((*a)*(*b));
    KlassObject* ret = vm_allocKlassObject();
    ret->klass = bignumKlass;
    ret->members = bignumKlass->members;
    ret->members[".ptr"] = PObjFromPtr(c);
    //store new object in return result
    return PObjFromKlassObj(ret);
  }
  PltObject bignum__eq(PltObject* args,int n)
  {
    if(n!=2)
      return  Plt_Err(ValueError,"2 arguments needed");
    if(args[0].type!='o' || args[1].type!='o')
      return  Plt_Err(TypeError,"Error bignum object needed");
    if(((KlassObject*)args[0].ptr)->klass!=bignumKlass ||((KlassObject*)args[0].ptr)->klass!=bignumKlass)
      return  Plt_Err(TypeError,"Error bignum object needed");
    
    KlassObject* self = (KlassObject*)args[0].ptr;
    KlassObject* rhs = (KlassObject*)args[1].ptr;
    
    bignum* a = (bignum*)self->members[".ptr"].ptr;
    bignum* b = (bignum*)rhs->members[".ptr"].ptr;
    PltObject rr;
    rr.type = PLT_BOOL;
    rr.i = *a == *b;
    return rr;
  }
  PltObject bignum__noteq(PltObject* args,int n)
  {
    if(n!=2)
      return  Plt_Err(ValueError,"2 arguments needed");
    if(args[0].type!='o' || args[1].type!='o')
      return  Plt_Err(TypeError,"Error bignum object needed");
    if(((KlassObject*)args[0].ptr)->klass!=bignumKlass ||((KlassObject*)args[0].ptr)->klass!=bignumKlass)
      return  Plt_Err(TypeError,"Error bignum object needed");
    
    KlassObject* self = (KlassObject*)args[0].ptr;
    KlassObject* rhs = (KlassObject*)args[1].ptr;
    
    bignum* a = (bignum*)self->members[".ptr"].ptr;
    bignum* b = (bignum*)rhs->members[".ptr"].ptr;
    PltObject rr;
    rr.type = PLT_BOOL;
    rr.i = !(*a == *b);
    return rr;
  }
  PltObject bignum__smallerthan(PltObject* args,int n)
  {
    if(n!=2)
      return  Plt_Err(ValueError,"2 arguments needed");
    if(args[0].type!='o' || args[1].type!='o')
      return  Plt_Err(TypeError,"Error bignum object needed");
    if(((KlassObject*)args[0].ptr)->klass!=bignumKlass ||((KlassObject*)args[0].ptr)->klass!=bignumKlass)
      return  Plt_Err(TypeError,"Error bignum object needed");
    
    KlassObject* self = (KlassObject*)args[0].ptr;
    KlassObject* rhs = (KlassObject*)args[1].ptr;
    
    bignum* a = (bignum*)self->members[".ptr"].ptr;
    bignum* b = (bignum*)rhs->members[".ptr"].ptr;
    PltObject rr;
    rr.type = PLT_BOOL;
    rr.i = (*a < *b);
    return rr;
  }
  PltObject bignum__greaterthan(PltObject* args,int n)
  {
    if(n!=2)
      return  Plt_Err(ValueError,"2 arguments needed");
    if(args[0].type!='o' || args[1].type!='o')
      return  Plt_Err(TypeError,"Error bignum object needed");
    if(((KlassObject*)args[0].ptr)->klass!=bignumKlass ||((KlassObject*)args[0].ptr)->klass!=bignumKlass)
      return  Plt_Err(TypeError,"Error bignum object needed");
    
    KlassObject* self = (KlassObject*)args[0].ptr;
    KlassObject* rhs = (KlassObject*)args[1].ptr;
    
    bignum* a = (bignum*)self->members[".ptr"].ptr;
    bignum* b = (bignum*)rhs->members[".ptr"].ptr;
    PltObject rr;
    rr.type = PLT_BOOL;
    rr.i = (*a > *b);
    return rr;
  }
  //
  PltObject bignum__smallerthaneq(PltObject* args,int n)
  {
    if(n!=2)
      return  Plt_Err(ValueError,"2 arguments needed");
    if(args[0].type!='o' || args[1].type!='o')
      return  Plt_Err(TypeError,"Error bignum object needed");
    if(((KlassObject*)args[0].ptr)->klass!=bignumKlass ||((KlassObject*)args[0].ptr)->klass!=bignumKlass)
      return  Plt_Err(TypeError,"Error bignum object needed");
    KlassObject* self = (KlassObject*)args[0].ptr;
    KlassObject* rhs = (KlassObject*)args[1].ptr;
    
    bignum* a = (bignum*)self->members[".ptr"].ptr;
    bignum* b = (bignum*)rhs->members[".ptr"].ptr;
    PltObject rr;
    rr.type = PLT_BOOL;
    rr.i = (*a < *b || *a == *b);
    return rr;
  }
  PltObject bignum__greaterthaneq(PltObject* args,int n)
  {
    if(n!=2)
      return  Plt_Err(ValueError,"2 arguments needed");
    if(args[0].type!='o' || args[1].type!='o')
      return  Plt_Err(TypeError,"Error bignum object needed");
    if(((KlassObject*)args[0].ptr)->klass!=bignumKlass ||((KlassObject*)args[0].ptr)->klass!=bignumKlass)
      return  Plt_Err(TypeError,"Error bignum object needed");
    
    KlassObject* self = (KlassObject*)args[0].ptr;
    KlassObject* rhs = (KlassObject*)args[1].ptr;
    
    bignum* a = (bignum*)self->members[".ptr"].ptr;
    bignum* b = (bignum*)rhs->members[".ptr"].ptr;
    PltObject rr;
    rr.type = PLT_BOOL;
    rr.i = (*a > *b || *a == *b);
    return rr;
  }
  PltObject bignum__destroy(PltObject* args,int n)
  {
    KlassObject* d = (KlassObject*)args[0].ptr;
    if(d->members[".ptr"].type!=PLT_POINTER)
      return nil;  
    bignum* a = (bignum*)d->members[".ptr"].ptr;
    delete a;
    return nil;
  }
  PltObject bignum__strval(PltObject* args,int n)
  {
    if(n!=1)
      return Plt_Err(ArgumentError,"1 argument needed");
    if(args[0].type!='o' || ((KlassObject*)args[0].ptr)->klass!=bignumKlass)
      return Plt_Err(TypeError,"Error bignum object needed!");
    KlassObject* self = (KlassObject*)args[0].ptr;
    bignum a = *(bignum*)self->members[".ptr"].ptr;
    return PObjFromStr(a.str());
  }
  PltObject bignum__increment(PltObject* args,int32_t n)
  {
     if(n!=1)
      return Plt_Err(ArgumentError,"1 argument needed");
    if(args[0].type!=PLT_OBJ || ((KlassObject*)args[0].ptr)->klass!=bignumKlass)
      return Plt_Err(TypeError,"Error bignum object needed!");
    KlassObject* self = (KlassObject*)args[0].ptr;
    bignum& a = *(bignum*)self->members[".ptr"].ptr;
    a.increment();
    return nil;
  }
}
