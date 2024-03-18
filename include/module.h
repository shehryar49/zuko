#ifndef MODULE_H_
#define MODULE_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "strmap.h"

typedef struct Module
{
  const char* name;
  StrMap members;
}Module;


void Module_addKlass(Module* m,const char* name,Klass* p);
void Module_addMember(Module* m,const char* name,ZObject val);

#ifdef __cplusplus
}
#endif

#endif