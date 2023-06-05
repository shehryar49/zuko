#ifndef PROCESS_PLT_H_
#define PROCESS_PLT_H_
#include "PltObject.h"

extern "C"
{
    PltObject init(PltObject*);
    PltObject FORK(PltObject*,int32_t);
    PltObject GETPID(PltObject*,int32_t);  
}
#endif