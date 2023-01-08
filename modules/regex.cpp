#include "regex.h"
#include <regex>
void init(PltObject* rr)
{
    Module* d = vm_allocModule();
    d->name = "regex";
    d->members.emplace("match",PltObjectFromFunction("regex.match",&match));
    d->members.emplace("search",PltObjectFromFunction("regex.search",&search));
    d->members.emplace("replace",PltObjectFromFunction("regex.replace",&replace));
    rr->type = PLT_MODULE;
    rr->ptr = (void*)d;
}
void match(PltObject* args,int n,PltObject* rr)
{
  if(n!=2)
  {
    *rr = Plt_Err(ARGUMENT_ERROR,"2 arguments needed!");
    return;
  }
  if(args[0].type!='s' || args[1].type!='s')
  {
    *rr = Plt_Err(TYPE_ERROR,"2 string arguments needed!");
    return;
  }
  rr->type = 'b';
  rr->i = (bool)regex_match(*(string*)args[0].ptr,regex(*(string*)args[1].ptr));

}
void search(PltObject* args,int n,PltObject* rr)
{
  if(n!=2)
  {
    *rr = Plt_Err(ARGUMENT_ERROR,"2 arguments needed!");
    return;
  }
  if(args[0].type!='s' || args[1].type!='s')
  {
    *rr = Plt_Err(TYPE_ERROR,"2 string arguments needed!");
    return;
  }
  smatch m;
  PltList* parts = vm_allocList();
  string& A = *(string*)args[0].ptr;
  while (regex_search(*(string*)args[0].ptr, m, regex(*(string*)args[1].ptr)))
  {
       parts->push_back(PltObjectFromString(m[0]));
       A = m.suffix();
   }
  rr->type = 'j';
  rr->ptr = (void*)parts;
}
void replace(PltObject* args,int n,PltObject* rr)
{
  if(n!=3)
  {
    *rr = Plt_Err(ARGUMENT_ERROR,"3 arguments needed!");
    return;
  }
  if(args[0].type!='s' || args[1].type!='s' || args[2].type!='s')
  {
    *rr = Plt_Err(TYPE_ERROR,"3 string arguments needed!");
    return;
  }
  *rr = PltObjectFromString(regex_replace(*(string*)args[0].ptr, regex(*(string*)args[1].ptr),*(string*) args[2].ptr));
}
