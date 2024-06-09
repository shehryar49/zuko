#include "klassobject.h"

zobject zclassobject_get(zclass_object* ko,const char* name)
{
  zobject val;
  val.type = Z_NIL;
  StrMap_get(&(ko->members),name,&val);
  return val;
}
void zclassobj_set(zclass_object* ko,const char* name,zobject val)
{
  StrMap_set(&(ko->members),name,val);
}