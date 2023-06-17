#ifndef PLT_JSON_H_
#define PLT_JSON_H_
#include "PltObject.h"
#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif
extern "C"
{
    EXPORT PltObject init();
    EXPORT PltObject loads(PltObject*,int32_t);    
}
#endif