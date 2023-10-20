#include "regex.h"
#include <regex>
PltObject init()
{
    Module* d = vm_allocModule();
    d->name = "regex";
    d->members.emplace("match",PObjFromFunction("regex.match",&match));
    d->members.emplace("search",PObjFromFunction("regex.search",&search));
    d->members.emplace("replace",PObjFromFunction("regex.replace",&replace));
    return PObjFromModule(d);
}
PltObject match(PltObject* args,int n)
{
  if(n!=2)
  {
    return Plt_Err(ArgumentError,"2 arguments needed!");
    
  }
  if(args[0].type!='s' || args[1].type!='s')
  {
    return Plt_Err(TypeError,"2 string arguments needed!");
    
  }
  PltObject rr;
  rr.type = 'b';
  try
  {
    rr.i = (bool)regex_match(*(string*)args[0].ptr,regex(*(string*)args[1].ptr));
  }
  catch(std::regex_error& e)
  {
    return Plt_Err(ValueError,e.what());
  }
  return rr;
}
PltObject search(PltObject* args,int n)
{
  if(n!=2)
  {
    return Plt_Err(ArgumentError,"2 arguments needed!");
    
  }
  if(args[0].type!='s' || args[1].type!='s')
  {
    return Plt_Err(TypeError,"2 string arguments needed!");
    
  }
  smatch m;
  PltList* parts = vm_allocList();
  string& A = *(string*)args[0].ptr;
  std::regex rgx(AS_STR(args[1]));
  try
  {
    while (regex_search(*(string*)args[0].ptr, m, rgx))
    {
        PltList* match = vm_allocList();
        for(auto x: m)
          match->push_back(PObjFromStr(x));
        parts->push_back(PObjFromList(match));
        A = m.suffix();
    }
  }
  catch(std::regex_error& e)
  {
    return Plt_Err(ValueError,e.what());
  }
  return PObjFromList(parts);
}
PltObject replace(PltObject* args,int n)
{
  if(n!=3)
  {
    return Plt_Err(ArgumentError,"3 arguments needed!");
    
  }
  if(args[0].type!='s' || args[1].type!='s' || args[2].type!='s')
  {
    return Plt_Err(TypeError,"3 string arguments needed!");
    
  }
  try
  {
    return PObjFromStr(regex_replace(*(string*)args[0].ptr, regex(*(string*)args[1].ptr),*(string*) args[2].ptr));
  }
  catch(std::regex_error& e)
  {
    return Plt_Err(ValueError,e.what());
  }
}
