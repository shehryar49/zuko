#include "math.h"
#include <cmath>
#include <climits>

using namespace std;

PltObject init()
{

    PltObject nil;
    Module* ModuleObject = vm_allocModule();//allocate a module object

    ModuleObject->name = "math";//module name is math
    //Add functions to the module
    ModuleObject->members.emplace("floor",PObjFromFunction("math.floor",&FLOOR));
    ModuleObject->members.emplace("ceil",PObjFromFunction("math.ceil",&CEIL));
    ModuleObject->members.emplace("round",PObjFromFunction("math.round",&ROUND));
    ModuleObject->members.emplace("sin",PObjFromFunction("math.sin",&SIN));
    ModuleObject->members.emplace("cos",PObjFromFunction("math.cos",&COS));
    ModuleObject->members.emplace("tan",PObjFromFunction("math.tan",&TAN));
    ModuleObject->members.emplace("asin",PObjFromFunction("math.asin",&ASIN));
    ModuleObject->members.emplace("acos",PObjFromFunction("math.acos",&ACOS));
    ModuleObject->members.emplace("atan",PObjFromFunction("math.atan",&ATAN));
    ModuleObject->members.emplace("sinh",PObjFromFunction("math.sinh",&SINH));
    ModuleObject->members.emplace("cosh",PObjFromFunction("math.cosh",&COSH));
    ModuleObject->members.emplace("tanh",PObjFromFunction("math.tanh",&TANH));
    ModuleObject->members.emplace("asinh",PObjFromFunction("math.asinh",&ASINH));
    ModuleObject->members.emplace("acosh",PObjFromFunction("math.acosh",&ACOSH));
    ModuleObject->members.emplace("atanh",PObjFromFunction("math.atanh",&ATANH));
    ModuleObject->members.emplace("sqrt",PObjFromFunction("math.sqrt",&SQRT));
    ModuleObject->members.emplace("trunc",PObjFromFunction("math.trunc",&TRUNC));
    ModuleObject->members.emplace("radians",PObjFromFunction("math.radians",&RADIANS));
    ModuleObject->members.emplace("log",PObjFromFunction("math.log",&LOG));
    ModuleObject->members.emplace("log10",PObjFromFunction("math.log10",&LOG10));
    return PObjFromModule(ModuleObject);
}

PltObject FLOOR(PltObject* args,int n)
{
    //rr stands for return result
    if(n!=1)
        return Plt_Err(ARGUMENT_ERROR,"1 argument needed");
    if(args[0].type=='i' || args[0].type=='l')
        return args[0];
    if(args[0].type!=PLT_FLOAT)
        return Plt_Err(TYPE_ERROR,"Numeric argument needed!");    
    double res = (floor(args[0].f));
    if(res>=LLONG_MIN && res<=LLONG_MAX)
      return PObjFromInt64((long long int)res);
    else
      return PObjFromDouble(res);
}
PltObject CEIL(PltObject* args,int n)
{
    if(n!=1)
    {
        return Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        
    }
    if(args[0].type=='i' || args[0].type=='l')
    {
        return args[0];
        
    }
    if(args[0].type!=PLT_FLOAT)
    {
        return Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        
    }
    double res = (ceil(args[0].f));
    if(res>=LLONG_MIN && res<=LLONG_MAX)
      return PObjFromInt64((long long int)res);
    else
      return PObjFromDouble(res);
}
PltObject TRUNC(PltObject* args,int n)
{
    if(n!=1)
    {
        return Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        
    }
    if(args[0].type=='i' || args[0].type=='l')
    {
        return args[0];
        
    }
    if(args[0].type!=PLT_FLOAT)
    {
        return Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        
    }
    double res = (trunc(args[0].f));
    if(res>=LLONG_MIN && res<=LLONG_MAX)
      return PObjFromInt64((long long int)res);
    else
      return PObjFromDouble(res);
}
PltObject ROUND(PltObject* args,int n)
{
    if(n!=1)
    {
        return Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        
    }
    if(args[0].type=='i' || args[0].type=='l')
    {
        return args[0];
        
    }
    if(args[0].type!=PLT_FLOAT)
    {
        return Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        
    }
    double res = (round(args[0].f));
    if(res>=LLONG_MIN && res<=LLONG_MAX)
      return PObjFromInt64((long long int)res);
    else
      return PObjFromDouble(res);
}
PltObject SQRT(PltObject* args,int n)
{
    if(n!=1)
    {
        return Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        
    }
    if(args[0].type==PLT_FLOAT)
    {
        double res = (sqrt(args[0].f));
        return PObjFromDouble(res);
    }
    else if(args[0].type==PLT_INT)
    {
        double res = (sqrt(args[0].i));
        return PObjFromDouble(res);
    }
    else if(args[0].type==PLT_INT64)
    {
        double res = (sqrt(args[0].l));
        return PObjFromDouble(res);
    }
    else
    {
        return Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        
    }
}
PltObject RADIANS(PltObject* args,int n)
{
    if(n!=1)
    {
        return Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        
    }
    double pi = 2*acos(0.0);
    if(args[0].type==PLT_FLOAT)
    {
        double res = ((args[0].f*pi/180));
        return PObjFromDouble(res);
    }
    else if(args[0].type==PLT_INT)
    {
        double res = ((args[0].i*pi/180));
        return PObjFromDouble(res);
    }
    else if(args[0].type==PLT_INT64)
    {
        double res = ((args[0].l*pi/180));
        return PObjFromDouble(res);
    }
    else
    {
        return Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        
    }
}
PltObject LOG(PltObject* args,int n)
{
    if(n!=1)
    {
        return Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        
    }
    if(args[0].type==PLT_FLOAT)
    {
        if(args[0].f<1)
        {
            return Plt_Err(MATH_ERROR,"Input outside log;'s domain interval [1,inf)");
            
        }
        double res = (log(args[0].f));
        return PObjFromDouble(res);
    }
    else if(args[0].type==PLT_INT)
    {
        if(args[0].i<1)
        {
            return Plt_Err(MATH_ERROR,"Input outside log;'s domain interval [1,inf)");
            
        }
        double res = (log(args[0].i));
        return PObjFromDouble(res);
    }
    else if(args[0].type==PLT_INT64)
    {
        if(args[0].l<1)
        {
            return Plt_Err(MATH_ERROR,"Input outside log;'s domain interval [1,inf)");
            
        }
        double res = (log(args[0].l));
        return PObjFromDouble(res);
    }
    else
    {
        return Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        
    }
}
PltObject LOG10(PltObject* args,int n)
{
    if(n!=1)
    {
        return Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        
    }
    if(args[0].type==PLT_FLOAT)
    {
        if(args[0].f<1)
        {
            return Plt_Err(MATH_ERROR,"Input outside log;'s domain interval [1,inf)");
            
        }
        double res = (log10(args[0].f));
        return PObjFromDouble(res);
    }
    else if(args[0].type==PLT_INT)
    {
        if(args[0].i<1)
        {
            return Plt_Err(MATH_ERROR,"Input outside log;'s domain interval [1,inf)");
            
        }
        double res = (log10(args[0].i));
        return PObjFromDouble(res);
    }
    else if(args[0].type==PLT_INT64)
    {
        if(args[0].l<1)
        {
            return Plt_Err(MATH_ERROR,"Input outside log;'s domain interval [1,inf)");
            
        }
        double res = (log10(args[0].l));
        return PObjFromDouble(res);
    }
    else
    {
        return Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        
    }
}
PltObject SIN(PltObject* args,int n)
{
    if(n!=1)
    {
        return Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        
    }
    if(args[0].type==PLT_FLOAT)
    {
        double res = (sin(args[0].f));
        return PObjFromDouble(res);
    }
    else if(args[0].type==PLT_INT)
    {
        double res = (sin(args[0].i));
        return PObjFromDouble(res);
    }
    else if(args[0].type==PLT_INT64)
    {
        double res = (sin(args[0].l));
        return PObjFromDouble(res);
    }
    else
    {
        return Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        
    }
}
PltObject COS(PltObject* args,int n)
{
    if(n!=1)
    {
        return Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        
    }
    if(args[0].type==PLT_FLOAT)
    {
        double res = (cos(args[0].f));
        return PObjFromDouble(res);
    }
    else if(args[0].type==PLT_INT)
    {
        double res = (cos(args[0].i));
        return PObjFromDouble(res);
    }
    else if(args[0].type==PLT_INT64)
    {
        double res = (cos(args[0].l));
        return PObjFromDouble(res);
    }
    else
    {
        return Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        
    }
}
PltObject TAN(PltObject* args,int n)
{
    if(n!=1)
    {
        return Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        
    }
    if(args[0].type==PLT_FLOAT)
    {
        if(cos(args[0].f)==0)
        {
            return Plt_Err(MATH_ERROR,"Input outside tan;'s domain interval R - {pi/2 + npi}");
            
        }
        double res = (tan(args[0].f));
        return PObjFromDouble(res);
    }
    else if(args[0].type==PLT_INT)
    {
        if(cos(args[0].i)==0)
        {
            return Plt_Err(MATH_ERROR,"Input outside tan;'s domain interval R - {pi/2 + npi}");
            
        }
        double res = (tan(args[0].i));
        return PObjFromDouble(res);
    }
    else if(args[0].type==PLT_INT64)
    {
        if(cos(args[0].l)==0)
        {
            return Plt_Err(MATH_ERROR,"Input outside tan;'s domain interval R - {pi/2 + npi}");
            
        }
        double res = (tan(args[0].l));
        return PObjFromDouble(res);
    }
    else
    {
        return Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        
    }
}
PltObject SINH(PltObject* args,int n)
{
    if(n!=1)
    {
        return Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        
    }
    if(args[0].type==PLT_FLOAT)
    {
        double res = (sinh(args[0].f));
        return PObjFromDouble(res);
    }
    else if(args[0].type==PLT_INT)
    {
        double res = (sinh(args[0].i));
        return PObjFromDouble(res);
    }
    else if(args[0].type==PLT_INT64)
    {
        double res = (sinh(args[0].l));
        return PObjFromDouble(res);
    }
    else
    {
        return Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        
    }
}
PltObject COSH(PltObject* args,int n)
{
    if(n!=1)
    {
        return Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        
    }
    if(args[0].type==PLT_FLOAT)
    {
        double res = (cosh(args[0].f));
        return PObjFromDouble(res);
    }
    else if(args[0].type==PLT_INT)
    {
        double res = (cosh(args[0].i));
        return PObjFromDouble(res);
    }
    else if(args[0].type==PLT_INT64)
    {
        double res = (cosh(args[0].l));
        return PObjFromDouble(res);
    }
    else
    {
        return Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        
    }
}
PltObject TANH(PltObject* args,int n)
{
    if(n!=1)
    {
        return Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        
    }
    if(args[0].type==PLT_FLOAT)
    {
        double res = (tanh(args[0].f));
        return PObjFromDouble(res);
    }
    else if(args[0].type==PLT_INT)
    {
        double res = (tanh(args[0].i));
        return PObjFromDouble(res);
    }
    else if(args[0].type==PLT_INT64)
    {
        double res = (tanh(args[0].l));
        return PObjFromDouble(res);
    }
    else
    {
        return Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        
    }
}
PltObject ASIN(PltObject* args,int n)
{
    if(n!=1)
    {
        return Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        
    }
    if(args[0].type==PLT_FLOAT)
    {
        if(args[0].f < -1 || args[0].f>1)
        {
            return Plt_Err(MATH_ERROR,"Input outside asin;'s domain interval [-1,1]");
            
        }
        double res = (asin(args[0].f));
        return PObjFromDouble(res);
    }
    else if(args[0].type==PLT_INT)
    {
        if(args[0].i < -1 || args[0].i>1)
        {
            return Plt_Err(MATH_ERROR,"Input outside asin;'s domain interval [-1,1]");
            
        }
        double res = (asin(args[0].i));
        return PObjFromDouble(res);
    }
    else if(args[0].type==PLT_INT64)
    {
        if(args[0].l < -1 || args[0].l>1)
        {
            return Plt_Err(MATH_ERROR,"Input outside asin;'s domain interval [-1,1]");
            
        }
        double res = (asin(args[0].l));
        return PObjFromDouble(res);
    }
    else
    {
        return Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        
    }
}
PltObject ACOS(PltObject* args,int n)
{
    if(n!=1)
    {
        return Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        
    }
    if(args[0].type==PLT_FLOAT)
    {
        if(args[0].f < -1 || args[0].f>1)
        {
            return Plt_Err(MATH_ERROR,"Input outside acos;'s domain interval [-1,1]");
            
        }
        double res = (acos(args[0].f));
        return PObjFromDouble(res);
    }
    else if(args[0].type==PLT_INT)
    {
        if(args[0].i < -1 || args[0].i>1)
        {
            return Plt_Err(MATH_ERROR,"Input outside acos;'s domain interval [-1,1]");
            
        }
        double res = (acos(args[0].i));
        return PObjFromDouble(res);
    }
    else if(args[0].type==PLT_INT64)
    {
        if(args[0].l < -1 || args[0].l>1)
        {
            return Plt_Err(MATH_ERROR,"Input outside acos;'s domain interval [-1,1]");
            
        }
        double res = (acos(args[0].l));
        return PObjFromDouble(res);
    }
    else
    {
        return Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        
    }
}
PltObject ATAN(PltObject* args,int n)
{
    if(n!=1)
    {
        return Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        
    }
    if(args[0].type==PLT_FLOAT)
    {
        double res = (atan(args[0].f));
        return PObjFromDouble(res);
    }
    else if(args[0].type==PLT_INT)
    {
        double res = (atan(args[0].i));
        return PObjFromDouble(res);
    }
    else if(args[0].type==PLT_INT64)
    {
        double res = (atan(args[0].l));
        return PObjFromDouble(res);
    }
    else
    {
        return Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        
    }
}
PltObject ASINH(PltObject* args,int n)
{
    if(n!=1)
    {
        return Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        
    }
    if(args[0].type==PLT_FLOAT)
    {
        double res = (asinh(args[0].f));
        return PObjFromDouble(res);
    }
    else if(args[0].type==PLT_INT)
    {
        double res = (asinh(args[0].i));
        return PObjFromDouble(res);
    }
    else if(args[0].type==PLT_INT64)
    {
        double res = (asinh(args[0].l));
        return PObjFromDouble(res);
    }
    else
    {
        return Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        
    }
}
PltObject ACOSH(PltObject* args,int n)
{
    if(n!=1)
    {
        return Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        
    }
    if(args[0].type==PLT_FLOAT)
    {
        if(args[0].f<1)
        {
            return Plt_Err(MATH_ERROR,"Input outside acosh;'s domain interval [1,inf)");
            
        }
        double res = (acosh(args[0].f));
        return PObjFromDouble(res);
    }
    else if(args[0].type==PLT_INT)
    {
        if(args[0].i<1)
        {
            return Plt_Err(MATH_ERROR,"Input outside acosh;'s domain interval [1,inf)");
            
        }
        double res = (acosh(args[0].i));
        return PObjFromDouble(res);
    }
    else if(args[0].type==PLT_INT64)
    {
        if(args[0].l<1)
        {
            return Plt_Err(MATH_ERROR,"Input outside acosh;'s domain interval [1,inf)");
            
        }
        double res = (acosh(args[0].l));
        return PObjFromDouble(res);
    }
    else
    {
        return Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        
    }
}
PltObject ATANH(PltObject* args,int n)
{
    if(n!=1)
    {
        return Plt_Err(ARGUMENT_ERROR,"1 argument needed");
        
    }
    if(args[0].type==PLT_FLOAT)
    {
        if(args[0].f <= -1 || args[0].f>=1)
        {
            return Plt_Err(MATH_ERROR,"Input outside atanh;'s domain interval (-1,1)");
            
        }
        double res = (atanh(args[0].f));
        return PObjFromDouble(res);
    }
    else if(args[0].type==PLT_INT)
    {
        if(args[0].i <= -1 || args[0].i>=1)
        {
            return Plt_Err(MATH_ERROR,"Input outside atanh;'s domain interval (-1,1)");
            
        }
        double res = (atanh(args[0].i));
        return PObjFromDouble(res);
    }
    else if(args[0].type==PLT_INT64)
    {
        if(args[0].l <= -1 || args[0].l>=1)
        {
            return Plt_Err(MATH_ERROR,"Input outside atanh;'s domain interval (-1,1)");
            
        }
        double res = (atanh(args[0].l));
        return PObjFromDouble(res);
    }
    else
    {
        return Plt_Err(TYPE_ERROR,"Numeric argument needed!");
        
    }
}
