#include "../include/PltObject.h"

extern "C"
{
    PltObject init();
    //Functions
    PltObject FLOOR(PltObject*,int);
    PltObject CEIL(PltObject*,int);
    PltObject ROUND(PltObject*,int);
    PltObject SIN(PltObject*,int);
    PltObject COS(PltObject*,int);
    PltObject TAN(PltObject*,int);
    PltObject ASIN(PltObject*,int);
    PltObject ACOS(PltObject*,int);
    PltObject ATAN(PltObject*,int);
    PltObject SINH(PltObject*,int);
    PltObject COSH(PltObject*,int);
    PltObject TANH(PltObject*,int);
    PltObject ASINH(PltObject*,int);
    PltObject ACOSH(PltObject*,int);
    PltObject ATANH(PltObject*,int);
    PltObject LOG(PltObject*,int);
    PltObject LOG10(PltObject*,int);
    PltObject SQRT(PltObject*,int);
    PltObject TRUNC(PltObject*,int);
    PltObject RADIANS(PltObject*,int);

}