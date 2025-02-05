#ifndef MODULE_H_
#define MODULE_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "strmap.h"

typedef struct zmodule
{
    const char* name;
    strmap members;
}zmodule;


void zmodule_add_class(zmodule* m,const char* name,zclass* p);
void zmodule_add_member(zmodule* m,const char* name,zobject val);

#ifdef __cplusplus
}
#endif

#endif
