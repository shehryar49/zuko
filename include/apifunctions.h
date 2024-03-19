#ifndef APIFUNCTIONS_H_
#define APIFUNCTIONS_H_

#include "zobject.h"
#include "zstr.h"
#include "zbytearray.h"
#include "strmap.h"
#include "funobject.h"
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
typedef ZList*(*fn1)();//allocList
typedef ZDict*(*fn2)();//allocZDict
typedef ZStr*(*fn3)(size_t);//allocString
typedef void*(*fn4)();//allocMutString
typedef zfile*(*fn5)();//allocFileObject
typedef Klass*(*fn6)();//allocKlass
typedef KlassObject*(*fn7)(Klass*);//allocKlassObject
typedef NativeFunction*(*fn8)();//allocNativeFunObj
typedef Module*(*fn9)();//allocModule
typedef ZByteArr*(*fn10)();//allocBytearray
typedef bool(*fn11)(ZObject*,ZObject*,int,ZObject*);//callobject
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
  Klass* k1;
  Klass* k2;
  Klass* k3;
  Klass* k4;
  Klass* k5;
  Klass* k6;
  Klass* k7;
  Klass* k8;
  Klass* k9;
  Klass* k10;
  Klass* k11;
  Klass* k12;
  Klass* k13;
  Klass* k14;
  Klass* k15;
  Klass* k16;
}apiFuncions;
#ifdef __cplusplus
}
#endif

#endif