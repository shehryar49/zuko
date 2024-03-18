#ifndef ZUKO_KLASSOBJ_H_
#define ZUKO_KLASSOBJ_H_


struct Klass;
struct StrMap;
struct ZObject;
#include "strmap.h"

struct StrMap;
typedef struct StrMap StrMap;

typedef struct KlassObject
{
  Klass* klass;
  StrMap members;
  StrMap privateMembers;
}KlassObject;

ZObject KlassObj_getMember(KlassObject* ko,const char* name);
void KlassObj_setMember(KlassObject* ko,const char* name,ZObject val);

#endif