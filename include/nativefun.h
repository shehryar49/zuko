#ifndef NATIVEFUN_H_
#define NATIVEFUN_H_
#include "zapi.h"

typedef ZObject(*NativeFunPtr)(ZObject*,int);
typedef struct NativeFunction
{
  Klass* klass;//address of class the function is member of (if any NULL otherwise)
  NativeFunPtr addr;
  const char* name;//name of function (used when printing errors)
  #ifdef __cpluscplus
    NativeFunction& operator=(const NativeFunction&) = delete;
    NativeFunction(const NativeFunction&) = delete;
  #endif
}NativeFunction;
#endif