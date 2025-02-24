#ifndef ZUKO_JSON_H_
#define ZUKO_JSON_H_
#include "zapi.h"
#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif
extern "C"
{
    EXPORT zobject init();
    EXPORT zobject loads(zobject*,int32_t);  
    EXPORT zobject dumps(zobject*,int32_t);
    EXPORT void unload();  
      
}
#endif
