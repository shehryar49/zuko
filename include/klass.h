#ifndef ZUKO_KLASS_H_
#define ZUKO_KLASS_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "strmap.h"

typedef struct zclass
{
  const char* name;
  strmap members;
}zclass;


void zclass_addmember(zclass* k,const char* name,zobject val);
void zclass_setmember(zclass* k,const char* name,zobject val);

#ifdef __cplusplus
}
#endif

#endif
