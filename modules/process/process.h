#ifndef PROCESS_PLT_H_
#define PROCESS_PLT_H_
#include "zapi.h"

extern "C"
{
    ZObject init(ZObject*);
    ZObject FORK(ZObject*,int32_t);
    ZObject GETPID(ZObject*,int32_t);  
}
#endif