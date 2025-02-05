#include "module.h"
#include "nativefun.h"
#include "zobject.h"


void zmodule_add_class(zmodule* m,const char* name,zclass* p)
{
    zobject val = zobj_from_class(p);  
    strmap_emplace(&(m->members),name,val);
}
void zmodule_add_member(zmodule* m,const char* name,zobject val)
{
    strmap_emplace(&(m->members),name,val);
}

