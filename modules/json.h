#ifndef PLT_JSON_H_
#define PLT_JSON_H_
#include "PltObject.h"
extern "C"
{
    PltObject init();
    PltObject loads(PltObject*,int32_t);    
}
#endif