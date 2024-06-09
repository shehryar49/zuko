#ifndef PROCESS_PLT_H_
#define PROCESS_PLT_H_
#include "zapi.h"

extern "C"
{
    zobject init(zobject*);
    zobject FORK(zobject*,int32_t);
    zobject GETPID(zobject*,int32_t);  
}
#endif