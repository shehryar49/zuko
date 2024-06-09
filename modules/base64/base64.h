#ifndef ZUKO_BASE64_H_
#define ZUKO_BASE64_H_
#include "zapi.h"
#ifdef _WIN32
  #define EXPORT __declspec(dllexport)
#else
  #define EXPORT
#endif

extern "C"
{
    zobject init();
    
    zobject ENCODE(zobject*,int32_t);
    zobject DECODE(zobject*,int32_t);
}
#endif