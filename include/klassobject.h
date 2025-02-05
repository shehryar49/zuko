#ifndef ZUKO_KLASSOBJ_H_
#define ZUKO_KLASSOBJ_H_

#ifdef __cplusplus
extern "C"{
#endif

typedef struct zclass zclass;
typedef struct zobject zobject;

#include "zobject.h"
#include "strmap.h"


typedef struct zclass_object
{
  zclass* _klass; //class of the object
  strmap members;
}zclass_object;

zobject zclassobj_get(zclass_object* ko,const char* name);
void zclassobj_set(zclass_object* ko,const char* name,zobject val);

#ifdef __cplusplus
}
#endif


#endif
