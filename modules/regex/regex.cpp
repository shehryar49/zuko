#include "regex.h"
#include <regex>
ZObject init()
{
    Module* d = vm_allocModule();
    d->name = "regex";
    d->members.emplace("match",ZObjFromFunction("regex.match",&match));
    d->members.emplace("search",ZObjFromFunction("regex.search",&search));
    d->members.emplace("replace",ZObjFromFunction("regex.replace",&replace));
    return ZObjFromModule(d);
}
ZObject match(ZObject* args,int n)
{
  if(n!=2)
  {
    return Z_Err(ArgumentError,"2 arguments needed!");
    
  }
  if(args[0].type!='s' || args[1].type!='s')
  {
    return Z_Err(TypeError,"2 string arguments needed!");
    
  }
  ZObject rr;
  rr.type = 'b';
  try
  {
    rr.i = (bool)regex_match(*(string*)args[0].ptr,regex(*(string*)args[1].ptr));
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
  {
    return Z_Err(ArgumentError,"2 arguments needed!");
    
  }
  if(args[0].type!='s' || args[1].type!='s')
  {
    return Z_Err(TypeError,"2 string arguments needed!");
    
  }
  smatch m;
  ZList* parts = vm_allocList();
  string& A = *(string*)args[0].ptr;
  std::regex rgx(AS_STR(args[1]));
  try
  {
    while (regex_search(*(string*)args[0].ptr, m, rgx))
    {
        ZList* match = vm_allocList();
        for(auto x: m)
          match->push_back(ZObjFromStr(x));
        parts->push_back(ZObjFromList(match));
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
    return ZObjFromStr(regex_replace(*(string*)args[0].ptr, regex(*(string*)args[1].ptr),*(string*) args[2].ptr));
  }
  catch(std::regex_error& e)
  {
    return Z_Err(ValueError,e.what());
  }
}
