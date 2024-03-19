#ifndef ZUKO_KLASSOBJ_H_
#define ZUKO_KLASSOBJ_H_

#ifdef __cplusplus
extern "C"{
#endif

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

#ifdef __cplusplus
}
#endif


#endif