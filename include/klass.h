#ifndef ZUKO_KLASS_H_
#define ZUKO_KLASS_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "strmap.h"

typedef struct Klass
{
  const char* name;
  StrMap members;
  StrMap privateMembers;
}Klass;


void Klass_addMember(Klass* k,const char* name,ZObject val);
void Klass_setMember(Klass* k,const char* name,ZObject val);

#ifdef __cplusplus
}
#endif

#endif