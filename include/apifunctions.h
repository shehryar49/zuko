#ifndef APIFUNCTIONS_H_
#define APIFUNCTIONS_H_

#include "zobject.h"
#include "zstr.h"
#include "zbytearray.h"
#include "strmap.h"
#include "klass.h"
#include "klassobject.h"
#include "module.h"
#include "nativefun.h"
#include "coroutineobj.h"
#include "zlist.h"
#include "zdict.h"

#ifdef __cplusplus
extern "C"{
#endif
  typedef zlist*(*fn1)();//alloc_zlist
  typedef zdict*(*fn2)();//alloc_zdict
  typedef zstr*(*fn3)(size_t);//alloc_zstr
  typedef uint8_t*(*fn4)(size_t);//alloc_zbytearr
  typedef zfile*(*fn5)();//alloc_zfile
  typedef zclass*(*fn6)(const char*);//alloc_zclass
  typedef zclass_object*(*fn7)(zclass*);//alloc_zclassobj
  typedef znativefun*(*fn8)();//alloc_znativefun
  typedef zmodule*(*fn9)(const char*);//alloc_zmodule
  typedef zbytearr*(*fn10)();//alloc_zbytearr
  typedef bool(*fn11)(zobject*,zobject*,int,zobject*);//call_object
  typedef void(*fn12)(void*);//mark_important
  typedef void(*fn13)(void*);//unmark_impotant
  typedef struct api_functions
  {
    fn1 a1;//api function 1
    fn2 a2;//and so on
    fn3 a3;
    fn4 a4;
    fn5 a5;
    fn6 a6;
    fn7 a7;
    fn8 a8;
    fn9 a9;
    fn10 a10;
    fn11 a11;
    fn12 a12;
    fn13 a13;
    zclass* k1;
    zclass* k2;
    zclass* k3;
    zclass* k4;
    zclass* k5;
    zclass* k6;
    zclass* k7;
    zclass* k8;
    zclass* k9;
    zclass* k10;
    zclass* k11;
    zclass* k12;
    zclass* k13;
    zclass* k14;
    zclass* k15;
  }api_functions;
#ifdef __cplusplus
}
#endif

#endif
