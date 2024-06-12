#include "regex.h"
#include "zapi.h"
#include "zobject.h"
#include <regex>
using namespace std;
zobject init()
{
    zmodule* d = vm_alloc_zmodule();
    d->name = "regex";
    zmodule_add_fun(d,"match",&match);
    zmodule_add_fun(d,"search",&search);
    zmodule_add_fun(d,"replace",&replace);
    
    return zobj_from_module(d);
}
zobject match(zobject* args,int n)
{
  if(n!=2)
    return z_err(ArgumentError,"2 arguments needed!");
  if(args[0].type!='s' || args[1].type!='s')
    return z_err(TypeError,"2 string arguments needed!");
  
  zobject rr;
  rr.type = 'b';
  try
  {
    rr.i = (bool)regex_match(AS_STR(args[0])->val,regex(AS_STR(args[1])->val));
  }
  catch(std::regex_error& e)
  {
    return z_err(ValueError,e.what());
  }
  return rr;
}
zobject search(zobject* args,int n)
{
  if(n!=2)
    return z_err(ArgumentError,"2 arguments needed!");
  if(args[0].type!='s' || args[1].type!='s')
    return z_err(TypeError,"2 string arguments needed!");

  smatch m;
  zlist* parts = vm_alloc_zlist();
  string A = AS_STR(args[0])->val;
  string B = A;
  std::regex rgx(AS_STR(args[1])->val);
  try
  {
    while (regex_search(A, m, rgx))
    {
        zlist* match = vm_alloc_zlist();
        for(auto x: m)
          zlist_push(match,zobj_from_str(x.str().c_str()));
        zlist_push(parts,zobj_from_list(match));
        A = m.suffix();
    }
  }
  catch(std::regex_error& e)
  {
    return z_err(ValueError,e.what());
  }
  return zobj_from_list(parts);
}
zobject replace(zobject* args,int n)
{
  if(n!=3)
  {
    return z_err(ArgumentError,"3 arguments needed!");
    
  }
  if(args[0].type!='s' || args[1].type!='s' || args[2].type!='s')
  {
    return z_err(TypeError,"3 string arguments needed!");
    
  }
  try
  {
    return zobj_from_str(regex_replace(AS_STR(args[0])->val, regex(AS_STR(args[1])->val),AS_STR(args[2])->val).c_str());
  }
  catch(std::regex_error& e)
  {
    return z_err(ValueError,e.what());
  }
}
