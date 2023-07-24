#include "PltObject.h"
#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif
extern "C"
{
    EXPORT PltObject init();
    //Functions
    EXPORT PltObject FLOOR(PltObject*,int);
    EXPORT PltObject CEIL(PltObject*,int);
    EXPORT PltObject ROUND(PltObject*,int);
    EXPORT PltObject SIN(PltObject*,int);
    EXPORT PltObject COS(PltObject*,int);
    EXPORT PltObject TAN(PltObject*,int);
    EXPORT PltObject ASIN(PltObject*,int);
    EXPORT PltObject ACOS(PltObject*,int);
    EXPORT PltObject ATAN(PltObject*,int);
    EXPORT PltObject SINH(PltObject*,int);
    EXPORT PltObject COSH(PltObject*,int);
    EXPORT PltObject TANH(PltObject*,int);
    EXPORT PltObject ASINH(PltObject*,int);
    EXPORT PltObject ACOSH(PltObject*,int);
    EXPORT PltObject ATANH(PltObject*,int);
    EXPORT PltObject LOG(PltObject*,int);
    EXPORT PltObject LOG10(PltObject*,int);
    EXPORT PltObject SQRT(PltObject*,int);
    EXPORT PltObject TRUNC(PltObject*,int);
    EXPORT PltObject RADIANS(PltObject*,int);

}