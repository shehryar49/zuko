#include "regex.h"
#include <regex>
using namespace std;
ZObject init()
{
    Module* d = vm_allocModule();
    d->name = "regex";
    Module_addNativeFun(d,"match",&match);
    Module_addNativeFun(d,"search",&search);
    Module_addNativeFun(d,"replace",&replace);
    
    return ZObjFromModule(d);
}
ZObject match(ZObject* args,int n)
{
  if(n!=2)
    return Z_Err(ArgumentError,"2 arguments needed!");
  if(args[0].type!='s' || args[1].type!='s')
    return Z_Err(TypeError,"2 string arguments needed!");
  
  ZObject rr;
  rr.type = 'b';
  try
  {
    rr.i = (bool)regex_match(AS_STR(args[0])->val,regex(AS_STR(args[1])->val));
  }
  catch(std::regex_error& e)
  {
    return Z_Err(ValueError,e.what());
  }
  return rr;
}
ZObject search(ZObject* args,int n)
{
  if(n!=2)
    return Z_Err(ArgumentError,"2 arguments needed!");
  if(args[0].type!='s' || args[1].type!='s')
    return Z_Err(TypeError,"2 string arguments needed!");

  smatch m;
  ZList* parts = vm_allocList();
  string A = AS_STR(args[0])->val;
  string B = A;
  std::regex rgx(AS_STR(args[1])->val);
  try
  {
    while (regex_search(A, m, rgx))
    {
        ZList* match = vm_allocList();
        for(auto x: m)
          ZList_push(match,ZObjFromStr(x.str().c_str()));
        ZList_push(parts,ZObjFromList(match));
        A = m.suffix();
    }
  }
  catch(std::regex_error& e)
  {
    return Z_Err(ValueError,e.what());
  }
  return ZObjFromList(parts);
}
ZObject replace(ZObject* args,int n)
{
  if(n!=3)
  {
    return Z_Err(ArgumentError,"3 arguments needed!");
    
  }
  if(args[0].type!='s' || args[1].type!='s' || args[2].type!='s')
  {
    return Z_Err(TypeError,"3 string arguments needed!");
    
  }
  try
  {
    return ZObjFromStr(regex_replace(AS_STR(args[0])->val, regex(AS_STR(args[1])->val),AS_STR(args[2])->val).c_str());
  }
  catch(std::regex_error& e)
  {
    return Z_Err(ValueError,e.what());
  }
}
