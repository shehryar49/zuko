#include "../include/PltObject.h"

extern "C"
{
    void init(PltObject*);
    //Functions
    void FLOOR(PltObject*,int,PltObject*);
    void CEIL(PltObject*,int,PltObject*);
    void ROUND(PltObject*,int,PltObject*);
    void SIN(PltObject*,int,PltObject*);
    void COS(PltObject*,int,PltObject*);
    void TAN(PltObject*,int,PltObject*);
    void ASIN(PltObject*,int,PltObject*);
    void ACOS(PltObject*,int,PltObject*);
    void ATAN(PltObject*,int,PltObject*);
    void SINH(PltObject*,int,PltObject*);
    void COSH(PltObject*,int,PltObject*);
    void TANH(PltObject*,int,PltObject*);
    void ASINH(PltObject*,int,PltObject*);
    void ACOSH(PltObject*,int,PltObject*);
    void ATANH(PltObject*,int,PltObject*);
    void LOG(PltObject*,int,PltObject*);
    void LOG10(PltObject*,int,PltObject*);
    void SQRT(PltObject*,int,PltObject*);
    void TRUNC(PltObject*,int,PltObject*);
    void RADIANS(PltObject*,int,PltObject*);

}