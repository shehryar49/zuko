#include "klass.h"
#include "nativefun.h"


void Klass_addMember(Klass* k,const char* name,ZObject val)
{
  StrMap_emplace(&(k->members),name,val);
}
void Klass_setMember(Klass* k,const char* name,ZObject val)
{
  StrMap_set(&(k->members),name,val);
}
