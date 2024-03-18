#include "klassobject.h"

ZObject KlassObj_getMember(KlassObject* ko,const char* name)
{
  ZObject val;
  val.type = Z_NIL;
  StrMap_get(&(ko->members),name,&val);
  return val;
}
void KlassObj_setMember(KlassObject* ko,const char* name,ZObject val)
{
  StrMap_set(&(ko->members),name,val);
}