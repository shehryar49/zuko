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
  typedef zlist*(*fn1)();//allocList
  typedef zdict*(*fn2)();//allocZDict
  typedef zstr*(*fn3)(size_t);//allocString
  typedef void*(*fn4)();//unused
  typedef zfile*(*fn5)();//allocFileObject
  typedef zclass*(*fn6)();//allocKlass
  typedef zclass_object*(*fn7)(zclass*);//allocKlassObject
  typedef znativefun*(*fn8)();//allocNativeFunObj
  typedef zmodule*(*fn9)();//allocModule
  typedef zbytearr*(*fn10)();//allocBytearray
  typedef bool(*fn11)(zobject*,zobject*,int,zobject*);//callobject
  typedef void(*fn12)(void*);//markImportant
  typedef void(*fn13)(void*);//unmarkImpotant
  typedef struct apiFuncions
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
    zclass* k16;
  }apiFuncions;
#ifdef __cplusplus
}
#endif

#endif