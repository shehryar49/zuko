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
    return Plt_Err(ARGUMENT_ERROR,"2 arguments needed!");
    
  }
  if(args[0].type!='s' || args[1].type!='s')
  {
    return Plt_Err(TYPE_ERROR,"2 string arguments needed!");
    
  }
  PltObject rr;
  rr.type = 'b';
  try
  {
    rr.i = (bool)regex_match(*(string*)args[0].ptr,regex(*(string*)args[1].ptr));
  }
  catch(std::regex_error& e)
  {
    return Plt_Err(VALUE_ERROR,e.what());
  }
  return rr;
}
PltObject search(PltObject* args,int n)
{
  if(n!=2)
  {
    return Plt_Err(ARGUMENT_ERROR,"2 arguments needed!");
    
  }
  if(args[0].type!='s' || args[1].type!='s')
  {
    return Plt_Err(TYPE_ERROR,"2 string arguments needed!");
    
  }
  smatch m;
  PltList* parts = vm_allocList();
  string& A = *(string*)args[0].ptr;
  try
  {
    while (regex_search(*(string*)args[0].ptr, m, regex(*(string*)args[1].ptr)))
    {
        parts->push_back(PObjFromStr(m[0]));
        A = m.suffix();
    }
  }
  catch(std::regex_error& e)
  {
    return Plt_Err(VALUE_ERROR,e.what());
  }
  return PObjFromList(parts);
}
PltObject replace(PltObject* args,int n)
{
  if(n!=3)
  {
    return Plt_Err(ARGUMENT_ERROR,"3 arguments needed!");
    
  }
  if(args[0].type!='s' || args[1].type!='s' || args[2].type!='s')
  {
    return Plt_Err(TYPE_ERROR,"3 string arguments needed!");
    
  }
  try
  {
    return PObjFromStr(regex_replace(*(string*)args[0].ptr, regex(*(string*)args[1].ptr),*(string*) args[2].ptr));
  }
  catch(std::regex_error& e)
  {
    return Plt_Err(VALUE_ERROR,e.what());
  }
}
