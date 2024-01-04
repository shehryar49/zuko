#ifndef NATIVEFUN_H_
#define NATIVEFUN_H_
#include "zapi.h"

typedef ZObject(*NativeFunPtr)(ZObject*,int);
struct NativeFunction
{
  Klass* klass;//address of class the function is member of (if any NULL otherwise)
  NativeFunPtr addr;
  const char* name;//name of function (used when printing errors)
  NativeFunction& operator=(const NativeFunction&) = delete;
};
#endif