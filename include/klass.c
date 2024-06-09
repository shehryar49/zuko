#include "klass.h"
#include "nativefun.h"


void klass_addmember(zclass* k,const char* name,zobject val)
{
  StrMap_emplace(&(k->members),name,val);
}
void klass_setmember(zclass* k,const char* name,zobject val)
{
  StrMap_set(&(k->members),name,val);
}
