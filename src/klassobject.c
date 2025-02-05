#include "klassobject.h"

zobject zclassobj_get(zclass_object* ko,const char* name)
{
    zobject val;
    val.type = Z_NIL;
    strmap_get(&(ko->members),name,&val);
    return val;
}
void zclassobj_set(zclass_object* ko,const char* name,zobject val)
{
    strmap_set(&(ko->members),name,val);
}
