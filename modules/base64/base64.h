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
    ZObject init();
    
    ZObject ENCODE(ZObject*,int32_t);
    ZObject DECODE(ZObject*,int32_t);
}
#endif