#ifndef NATIVEFUN_H_
#define NATIVEFUN_H_

#ifdef __cplusplus
extern "C"{
#endif

typedef struct zobject zobject;
typedef struct zclass zclass;

typedef zobject(*NativeFunPtr)(zobject*,int);

typedef struct znativefun
{
  zclass* _klass;//address of class the function is member of (if any NULL otherwise)
  NativeFunPtr addr;
  const char* name;//name of function (used when printing errors)
  const char* signature; //type string if any NULL otherwise
}znativefun;

#ifdef __cplusplus
}
#endif

#endif