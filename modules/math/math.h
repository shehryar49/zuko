#include "ZObject.h"
#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif
extern "C"
{
    EXPORT ZObject init();
    //Functions
    EXPORT ZObject FLOOR(ZObject*,int);
    EXPORT ZObject CEIL(ZObject*,int);
    EXPORT ZObject ROUND(ZObject*,int);
    EXPORT ZObject SIN(ZObject*,int);
    EXPORT ZObject COS(ZObject*,int);
    EXPORT ZObject TAN(ZObject*,int);
    EXPORT ZObject ASIN(ZObject*,int);
    EXPORT ZObject ACOS(ZObject*,int);
    EXPORT ZObject ATAN(ZObject*,int);
    EXPORT ZObject SINH(ZObject*,int);
    EXPORT ZObject COSH(ZObject*,int);
    EXPORT ZObject TANH(ZObject*,int);
    EXPORT ZObject ASINH(ZObject*,int);
    EXPORT ZObject ACOSH(ZObject*,int);
    EXPORT ZObject ATANH(ZObject*,int);
    EXPORT ZObject LOG(ZObject*,int);
    EXPORT ZObject LOG10(ZObject*,int);
    EXPORT ZObject SQRT(ZObject*,int);
    EXPORT ZObject TRUNC(ZObject*,int);
    EXPORT ZObject RADIANS(ZObject*,int);

}