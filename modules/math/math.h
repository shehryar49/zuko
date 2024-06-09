#include "zapi.h"
#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif
extern "C"
{
    EXPORT zobject init();
    //Functions
    EXPORT zobject FLOOR(zobject*,int);
    EXPORT zobject CEIL(zobject*,int);
    EXPORT zobject ROUND(zobject*,int);
    EXPORT zobject SIN(zobject*,int);
    EXPORT zobject COS(zobject*,int);
    EXPORT zobject TAN(zobject*,int);
    EXPORT zobject ASIN(zobject*,int);
    EXPORT zobject ACOS(zobject*,int);
    EXPORT zobject ATAN(zobject*,int);
    EXPORT zobject SINH(zobject*,int);
    EXPORT zobject COSH(zobject*,int);
    EXPORT zobject TANH(zobject*,int);
    EXPORT zobject ASINH(zobject*,int);
    EXPORT zobject ACOSH(zobject*,int);
    EXPORT zobject ATANH(zobject*,int);
    EXPORT zobject LOG(zobject*,int);
    EXPORT zobject LOG10(zobject*,int);
    EXPORT zobject SQRT(zobject*,int);
    EXPORT zobject TRUNC(zobject*,int);
    EXPORT zobject RADIANS(zobject*,int);

}