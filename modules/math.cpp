#include "math.h"
#include <cmath>
#include <climits>

using namespace std;

void init(PltObject* rr)
{

    PltObject nil;
    rr->type = PLT_MODULE; //init returns a module object

    Module* ModuleObject = vm_allocModule();//allocate a module object

    ModuleObject->name = "math";//module name is math
    //Add functions to the module
    ModuleObject->members.emplace("floor",PltObjectFromFunction("math.floor",&FLOOR));
    ModuleObject->members.emplace("ceil",PltObjectFromFunction("math.ceil",&CEIL));
    ModuleObject->members.emplace("round",PltObjectFromFunction("math.round",&ROUND));
    ModuleObject->members.emplace("sin",PltObjectFromFunction("math.sin",&SIN));
    ModuleObject->members.emplace("cos",PltObjectFromFunction("math.cos",&COS));
    ModuleObject->members.emplace("tan",PltObjectFromFunction("math.tan",&TAN));
    ModuleObject->members.emplace("asin",PltObjectFromFunction("math.asin",&ASIN));
    ModuleObject->members.emplace("acos",PltObjectFromFunction("math.acos",&ACOS));
    ModuleObject->members.emplace("atan",PltObjectFromFunction("math.atan",&ATAN));
    ModuleObject->members.emplace("sinh",PltObjectFromFunction("math.sinh",&SINH));
    ModuleObject->members.emplace("cosh",PltObjectFromFunction("math.cosh",&COSH));
    ModuleObject->members.emplace("tanh",PltObjectFromFunction("math.tanh",&TANH));
    ModuleObject->members.emplace("asinh",PltObjectFromFunction("math.asinh",&ASINH));
    ModuleObject->members.emplace("acosh",PltObjectFromFunction("math.acosh",&ACOSH));
    ModuleObject->members.emplace("atanh",PltObjectFromFunction("math.atanh",&ATANH));
    ModuleObject->members.emplace("sqrt",PltObjectFromFunction("math.sqrt",&SQRT));
    ModuleObject->members.emplace("trunc",PltObjectFromFunction("math.trunc",&TRUNC));
    ModuleObject->members.emplace("radians",PltObjectFromFunction("math.radians",&RADIANS));
    ModuleObject->members.emplace("log",PltObjectFromFunction("math.log",&LOG));
    ModuleObject->members.emplace("log10",PltObjectFromFunction("math.log10",&LOG10));
    rr->ptr = (void*)ModuleObject;
}

void FLOOR(PltObject* args,int n,PltObject* rr)
{
    //rr stands for return result
    if(n!=1)
    {
        *rr = Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        return;
    }
    if(args[0].type=='i' || args[0].type=='l')
    {
        *rr = args[0];
        return;
    }
    if(args[0].type!=PLT_FLOAT)
    {
        *rr = Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        return;
    }
    double res = (floor(args[0].f));
    if(res>=LLONG_MIN && res<=LLONG_MAX)
      *rr = PltObjectFromInt64((long long int)res);
    else
      *rr = PltObjectFromDouble(res);
}
void CEIL(PltObject* args,int n,PltObject* rr)
{
    if(n!=1)
    {
        *rr = Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        return;
    }
    if(args[0].type=='i' || args[0].type=='l')
    {
        *rr = args[0];
        return;
    }
    if(args[0].type!=PLT_FLOAT)
    {
        *rr = Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        return;
    }
    double res = (ceil(args[0].f));
    if(res>=LLONG_MIN && res<=LLONG_MAX)
      *rr = PltObjectFromInt64((long long int)res);
    else
      *rr = PltObjectFromDouble(res);
}
void TRUNC(PltObject* args,int n,PltObject* rr)
{
    if(n!=1)
    {
        *rr = Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        return;
    }
    if(args[0].type=='i' || args[0].type=='l')
    {
        *rr = args[0];
        return;
    }
    if(args[0].type!=PLT_FLOAT)
    {
        *rr = Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        return;
    }
    double res = (trunc(args[0].f));
    if(res>=LLONG_MIN && res<=LLONG_MAX)
      *rr = PltObjectFromInt64((long long int)res);
    else
      *rr = PltObjectFromDouble(res);
}
void ROUND(PltObject* args,int n,PltObject* rr)
{
    if(n!=1)
    {
        *rr = Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        return;
    }
    if(args[0].type=='i' || args[0].type=='l')
    {
        *rr = args[0];
        return;
    }
    if(args[0].type!=PLT_FLOAT)
    {
        *rr = Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        return;
    }
    double res = (round(args[0].f));
    if(res>=LLONG_MIN && res<=LLONG_MAX)
      *rr = PltObjectFromInt64((long long int)res);
    else
      *rr = PltObjectFromDouble(res);
}
void SQRT(PltObject* args,int n,PltObject* rr)
{
    if(n!=1)
    {
        *rr = Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        return;
    }
    if(args[0].type==PLT_FLOAT)
    {
        double res = (sqrt(args[0].f));
        *rr = PltObjectFromDouble(res);
    }
    else if(args[0].type==PLT_INT)
    {
        double res = (sqrt(args[0].i));
        *rr = PltObjectFromDouble(res);
    }
    else if(args[0].type==PLT_INT64)
    {
        double res = (sqrt(args[0].l));
        *rr = PltObjectFromDouble(res);
    }
    else
    {
        *rr = Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        return;
    }
}
void RADIANS(PltObject* args,int n,PltObject* rr)
{
    if(n!=1)
    {
        *rr = Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        return;
    }
    double pi = 2*acos(0.0);
    if(args[0].type==PLT_FLOAT)
    {
        double res = ((args[0].f*pi/180));
        *rr = PltObjectFromDouble(res);
    }
    else if(args[0].type==PLT_INT)
    {
        double res = ((args[0].i*pi/180));
        *rr = PltObjectFromDouble(res);
    }
    else if(args[0].type==PLT_INT64)
    {
        double res = ((args[0].l*pi/180));
        *rr = PltObjectFromDouble(res);
    }
    else
    {
        *rr = Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        return;
    }
}
void LOG(PltObject* args,int n,PltObject* rr)
{
    if(n!=1)
    {
        *rr = Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        return;
    }
    if(args[0].type==PLT_FLOAT)
    {
        if(args[0].f<1)
        {
            *rr = Plt_Err(MATH_ERROR,"Input outside log's domain interval [1,inf)");
            return;
        }
        double res = (log(args[0].f));
        *rr = PltObjectFromDouble(res);
    }
    else if(args[0].type==PLT_INT)
    {
        if(args[0].i<1)
        {
            *rr = Plt_Err(MATH_ERROR,"Input outside log's domain interval [1,inf)");
            return;
        }
        double res = (log(args[0].i));
        *rr = PltObjectFromDouble(res);
    }
    else if(args[0].type==PLT_INT64)
    {
        if(args[0].l<1)
        {
            *rr = Plt_Err(MATH_ERROR,"Input outside log's domain interval [1,inf)");
            return;
        }
        double res = (log(args[0].l));
        *rr = PltObjectFromDouble(res);
    }
    else
    {
        *rr = Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        return;
    }
}
void LOG10(PltObject* args,int n,PltObject* rr)
{
    if(n!=1)
    {
        *rr = Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        return;
    }
    if(args[0].type==PLT_FLOAT)
    {
        if(args[0].f<1)
        {
            *rr = Plt_Err(MATH_ERROR,"Input outside log's domain interval [1,inf)");
            return;
        }
        double res = (log10(args[0].f));
        *rr = PltObjectFromDouble(res);
    }
    else if(args[0].type==PLT_INT)
    {
        if(args[0].i<1)
        {
            *rr = Plt_Err(MATH_ERROR,"Input outside log's domain interval [1,inf)");
            return;
        }
        double res = (log10(args[0].i));
        *rr = PltObjectFromDouble(res);
    }
    else if(args[0].type==PLT_INT64)
    {
        if(args[0].l<1)
        {
            *rr = Plt_Err(MATH_ERROR,"Input outside log's domain interval [1,inf)");
            return;
        }
        double res = (log10(args[0].l));
        *rr = PltObjectFromDouble(res);
    }
    else
    {
        *rr = Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        return;
    }
}
void SIN(PltObject* args,int n,PltObject* rr)
{
    if(n!=1)
    {
        *rr = Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        return;
    }
    if(args[0].type==PLT_FLOAT)
    {
        double res = (sin(args[0].f));
        *rr = PltObjectFromDouble(res);
    }
    else if(args[0].type==PLT_INT)
    {
        double res = (sin(args[0].i));
        *rr = PltObjectFromDouble(res);
    }
    else if(args[0].type==PLT_INT64)
    {
        double res = (sin(args[0].l));
        *rr = PltObjectFromDouble(res);
    }
    else
    {
        *rr = Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        return;
    }
}
void COS(PltObject* args,int n,PltObject* rr)
{
    if(n!=1)
    {
        *rr = Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        return;
    }
    if(args[0].type==PLT_FLOAT)
    {
        double res = (cos(args[0].f));
        *rr = PltObjectFromDouble(res);
    }
    else if(args[0].type==PLT_INT)
    {
        double res = (cos(args[0].i));
        *rr = PltObjectFromDouble(res);
    }
    else if(args[0].type==PLT_INT64)
    {
        double res = (cos(args[0].l));
        *rr = PltObjectFromDouble(res);
    }
    else
    {
        *rr = Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        return;
    }
}
void TAN(PltObject* args,int n,PltObject* rr)
{
    if(n!=1)
    {
        *rr = Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        return;
    }
    if(args[0].type==PLT_FLOAT)
    {
        if(cos(args[0].f)==0)
        {
            *rr = Plt_Err(MATH_ERROR,"Input outside tan's domain interval R - {pi/2 + npi}");
            return;
        }
        double res = (tan(args[0].f));
        *rr = PltObjectFromDouble(res);
    }
    else if(args[0].type==PLT_INT)
    {
        if(cos(args[0].i)==0)
        {
            *rr = Plt_Err(MATH_ERROR,"Input outside tan's domain interval R - {pi/2 + npi}");
            return;
        }
        double res = (tan(args[0].i));
        *rr = PltObjectFromDouble(res);
    }
    else if(args[0].type==PLT_INT64)
    {
        if(cos(args[0].l)==0)
        {
            *rr = Plt_Err(MATH_ERROR,"Input outside tan's domain interval R - {pi/2 + npi}");
            return;
        }
        double res = (tan(args[0].l));
        *rr = PltObjectFromDouble(res);
    }
    else
    {
        *rr = Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        return;
    }
}
void SINH(PltObject* args,int n,PltObject* rr)
{
    if(n!=1)
    {
        *rr = Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        return;
    }
    if(args[0].type==PLT_FLOAT)
    {
        double res = (sinh(args[0].f));
        *rr = PltObjectFromDouble(res);
    }
    else if(args[0].type==PLT_INT)
    {
        double res = (sinh(args[0].i));
        *rr = PltObjectFromDouble(res);
    }
    else if(args[0].type==PLT_INT64)
    {
        double res = (sinh(args[0].l));
        *rr = PltObjectFromDouble(res);
    }
    else
    {
        *rr = Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        return;
    }
}
void COSH(PltObject* args,int n,PltObject* rr)
{
    if(n!=1)
    {
        *rr = Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        return;
    }
    if(args[0].type==PLT_FLOAT)
    {
        double res = (cosh(args[0].f));
        *rr = PltObjectFromDouble(res);
    }
    else if(args[0].type==PLT_INT)
    {
        double res = (cosh(args[0].i));
        *rr = PltObjectFromDouble(res);
    }
    else if(args[0].type==PLT_INT64)
    {
        double res = (cosh(args[0].l));
        *rr = PltObjectFromDouble(res);
    }
    else
    {
        *rr = Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        return;
    }
}
void TANH(PltObject* args,int n,PltObject* rr)
{
    if(n!=1)
    {
        *rr = Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        return;
    }
    if(args[0].type==PLT_FLOAT)
    {
        double res = (tanh(args[0].f));
        *rr = PltObjectFromDouble(res);
    }
    else if(args[0].type==PLT_INT)
    {
        double res = (tanh(args[0].i));
        *rr = PltObjectFromDouble(res);
    }
    else if(args[0].type==PLT_INT64)
    {
        double res = (tanh(args[0].l));
        *rr = PltObjectFromDouble(res);
    }
    else
    {
        *rr = Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        return;
    }
}
void ASIN(PltObject* args,int n,PltObject* rr)
{
    if(n!=1)
    {
        *rr = Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        return;
    }
    if(args[0].type==PLT_FLOAT)
    {
        if(args[0].f < -1 || args[0].f>1)
        {
            *rr = Plt_Err(MATH_ERROR,"Input outside asin's domain interval [-1,1]");
            return;
        }
        double res = (asin(args[0].f));
        *rr = PltObjectFromDouble(res);
    }
    else if(args[0].type==PLT_INT)
    {
        if(args[0].i < -1 || args[0].i>1)
        {
            *rr = Plt_Err(MATH_ERROR,"Input outside asin's domain interval [-1,1]");
            return;
        }
        double res = (asin(args[0].i));
        *rr = PltObjectFromDouble(res);
    }
    else if(args[0].type==PLT_INT64)
    {
        if(args[0].l < -1 || args[0].l>1)
        {
            *rr = Plt_Err(MATH_ERROR,"Input outside asin's domain interval [-1,1]");
            return;
        }
        double res = (asin(args[0].l));
        *rr = PltObjectFromDouble(res);
    }
    else
    {
        *rr = Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        return;
    }
}
void ACOS(PltObject* args,int n,PltObject* rr)
{
    if(n!=1)
    {
        *rr = Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        return;
    }
    if(args[0].type==PLT_FLOAT)
    {
        if(args[0].f < -1 || args[0].f>1)
        {
            *rr = Plt_Err(MATH_ERROR,"Input outside acos's domain interval [-1,1]");
            return;
        }
        double res = (acos(args[0].f));
        *rr = PltObjectFromDouble(res);
    }
    else if(args[0].type==PLT_INT)
    {
        if(args[0].i < -1 || args[0].i>1)
        {
            *rr = Plt_Err(MATH_ERROR,"Input outside acos's domain interval [-1,1]");
            return;
        }
        double res = (acos(args[0].i));
        *rr = PltObjectFromDouble(res);
    }
    else if(args[0].type==PLT_INT64)
    {
        if(args[0].l < -1 || args[0].l>1)
        {
            *rr = Plt_Err(MATH_ERROR,"Input outside acos's domain interval [-1,1]");
            return;
        }
        double res = (acos(args[0].l));
        *rr = PltObjectFromDouble(res);
    }
    else
    {
        *rr = Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        return;
    }
}
void ATAN(PltObject* args,int n,PltObject* rr)
{
    if(n!=1)
    {
        *rr = Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        return;
    }
    if(args[0].type==PLT_FLOAT)
    {
        double res = (atan(args[0].f));
        *rr = PltObjectFromDouble(res);
    }
    else if(args[0].type==PLT_INT)
    {
        double res = (atan(args[0].i));
        *rr = PltObjectFromDouble(res);
    }
    else if(args[0].type==PLT_INT64)
    {
        double res = (atan(args[0].l));
        *rr = PltObjectFromDouble(res);
    }
    else
    {
        *rr = Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        return;
    }
}
void ASINH(PltObject* args,int n,PltObject* rr)
{
    if(n!=1)
    {
        *rr = Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        return;
    }
    if(args[0].type==PLT_FLOAT)
    {
        double res = (asinh(args[0].f));
        *rr = PltObjectFromDouble(res);
    }
    else if(args[0].type==PLT_INT)
    {
        double res = (asinh(args[0].i));
        *rr = PltObjectFromDouble(res);
    }
    else if(args[0].type==PLT_INT64)
    {
        double res = (asinh(args[0].l));
        *rr = PltObjectFromDouble(res);
    }
    else
    {
        *rr = Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        return;
    }
}
void ACOSH(PltObject* args,int n,PltObject* rr)
{
    if(n!=1)
    {
        *rr = Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        return;
    }
    if(args[0].type==PLT_FLOAT)
    {
        if(args[0].f<1)
        {
            *rr = Plt_Err(MATH_ERROR,"Input outside acosh's domain interval [1,inf)");
            return;
        }
        double res = (acosh(args[0].f));
        *rr = PltObjectFromDouble(res);
    }
    else if(args[0].type==PLT_INT)
    {
        if(args[0].i<1)
        {
            *rr = Plt_Err(MATH_ERROR,"Input outside acosh's domain interval [1,inf)");
            return;
        }
        double res = (acosh(args[0].i));
        *rr = PltObjectFromDouble(res);
    }
    else if(args[0].type==PLT_INT64)
    {
        if(args[0].l<1)
        {
            *rr = Plt_Err(MATH_ERROR,"Input outside acosh's domain interval [1,inf)");
            return;
        }
        double res = (acosh(args[0].l));
        *rr = PltObjectFromDouble(res);
    }
    else
    {
        *rr = Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        return;
    }
}
void ATANH(PltObject* args,int n,PltObject* rr)
{
    if(n!=1)
    {
        *rr = Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        return;
    }
    if(args[0].type==PLT_FLOAT)
    {
        if(args[0].f <= -1 || args[0].f>=1)
        {
            *rr = Plt_Err(MATH_ERROR,"Input outside atanh's domain interval (-1,1)");
            return;
        }
        double res = (atanh(args[0].f));
        *rr = PltObjectFromDouble(res);
    }
    else if(args[0].type==PLT_INT)
    {
        if(args[0].i <= -1 || args[0].i>=1)
        {
            *rr = Plt_Err(MATH_ERROR,"Input outside atanh's domain interval (-1,1)");
            return;
        }
        double res = (atanh(args[0].i));
        *rr = PltObjectFromDouble(res);
    }
    else if(args[0].type==PLT_INT64)
    {
        if(args[0].l <= -1 || args[0].l>=1)
        {
            *rr = Plt_Err(MATH_ERROR,"Input outside atanh's domain interval (-1,1)");
            return;
        }
        double res = (atanh(args[0].l));
        *rr = PltObjectFromDouble(res);
    }
    else
    {
        *rr = Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        return;
    }
}
