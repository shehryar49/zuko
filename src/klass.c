#include "klass.h"
#include "nativefun.h"


void zclass_addmember(zclass* k,const char* name,zobject val)
{
    strmap_emplace(&(k->members),name,val);
}
void zclass_setmember(zclass* k,const char* name,zobject val)
{
    strmap_set(&(k->members),name,val);
}
