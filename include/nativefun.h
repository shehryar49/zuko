#ifndef NATIVEFUN_H_
#define NATIVEFUN_H_

#ifdef __cplusplus
extern "C"{
#endif

typedef struct ZObject ZObject;
typedef struct Klass Klass;

typedef ZObject(*NativeFunPtr)(ZObject*,int);

typedef struct NativeFunction
{
  Klass* klass;//address of class the function is member of (if any NULL otherwise)
  NativeFunPtr addr;
  const char* name;//name of function (used when printing errors)
  const char* signature; //type string if any NULL otherwise
  #ifdef __cpluscplus
    NativeFunction& operator=(const NativeFunction&) = delete;
    NativeFunction(const NativeFunction&) = delete;
  #endif
}NativeFunction;

#ifdef __cplusplus
}
#endif

#endif