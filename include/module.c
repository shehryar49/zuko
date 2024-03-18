#include "module.h"
#include "nativefun.h"
#include "zobject.h"


void Module_addKlass(Module* m,const char* name,Klass* p)
{
  ZObject val = ZObjFromKlass(p);  
  StrMap_emplace(&(m->members),name,val);
}
void Module_addMember(Module* m,const char* name,ZObject val)
{
  StrMap_emplace(&(m->members),name,val);
}

