#ifndef MODULE_H_
#define MODULE_H_
#include "zapi.h"

#ifdef __cplusplus
extern "C"{
#endif

typedef struct Module
{
  const char* name;
  StrMap members;
}Module;

#ifdef __cplusplus
}
#endif

#endif