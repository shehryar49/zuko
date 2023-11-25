#ifndef PLT_JSON_H_
#define PLT_JSON_H_
#include "ZObject.h"
#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif
extern "C"
{
    EXPORT ZObject init();
    EXPORT ZObject loads(ZObject*,int32_t);  
    EXPORT ZObject dumps(ZObject*,int32_t);
    EXPORT void unload();  
      
}
#endif