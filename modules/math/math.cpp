#include "math.h"
#include <cmath>
#include <climits>

using namespace std;
ZObject nil;
ZObject init()
{

    nil.type = Z_NIL;
    Module* ModuleObject = vm_allocModule();//allocate a module object

    ModuleObject->name = "math";//module name is math
    //Add functions to the module
    ModuleObject->addNativeFunction("floor",&FLOOR);
    ModuleObject->addNativeFunction("ceil",&CEIL);
    ModuleObject->addNativeFunction("round",&ROUND);
    ModuleObject->addNativeFunction("sin",&SIN);
    ModuleObject->addNativeFunction("cos",&COS);
    ModuleObject->addNativeFunction("tan",&TAN);
    ModuleObject->addNativeFunction("asin",&ASIN);
    ModuleObject->addNativeFunction("acos",&ACOS);
    ModuleObject->addNativeFunction("atan",&ATAN);
    ModuleObject->addNativeFunction("sinh",&SINH);
    ModuleObject->addNativeFunction("cosh",&COSH);
    ModuleObject->addNativeFunction("tanh",&TANH);
    ModuleObject->addNativeFunction("asinh",&ASINH);
    ModuleObject->addNativeFunction("acosh",&ACOSH);
    ModuleObject->addNativeFunction("atanh",&ATANH);
    ModuleObject->addNativeFunction("sqrt",&SQRT);
    ModuleObject->addNativeFunction("trunc",&TRUNC);
    ModuleObject->addNativeFunction("radians",&RADIANS);
    ModuleObject->addNativeFunction("log",&LOG);
    ModuleObject->addNativeFunction("log10",&LOG10);
    return ZObjFromModule(ModuleObject);
}

ZObject FLOOR(ZObject* args,int n)
{
    //rr stands for return result
    if(n!=1)
        return Z_Err(ArgumentError,"1 argument needed");
    if(args[0].type=='i' || args[0].type=='l')
        return args[0];
    if(args[0].type!=Z_FLOAT)
        return Z_Err(TypeError,"Numeric argument needed!");    
    double res = (floor(args[0].f));
    if(res>=LLONG_MIN && res<=LLONG_MAX)
      return ZObjFromInt64((long long int)res);
    else
      return ZObjFromDouble(res);
}
ZObject CEIL(ZObject* args,int n)
{
    if(n!=1)
    {
        return Z_Err(ArgumentError,"1 argument needed");
        
    }
    if(args[0].type=='i' || args[0].type=='l')
    {
        return args[0];
        
    }
    if(args[0].type!=Z_FLOAT)
    {
        return Z_Err(TypeError,"Numeric argument needed!");
        
    }
    double res = (ceil(args[0].f));
    if(res>=LLONG_MIN && res<=LLONG_MAX)
      return ZObjFromInt64((long long int)res);
    else
      return ZObjFromDouble(res);
}
ZObject TRUNC(ZObject* args,int n)
{
    if(n!=1)
    {
        return Z_Err(ArgumentError,"1 argument needed");
        
    }
    if(args[0].type=='i' || args[0].type=='l')
    {
        return args[0];
        
    }
    if(args[0].type!=Z_FLOAT)
    {
        return Z_Err(TypeError,"Numeric argument needed!");
        
    }
    double res = (trunc(args[0].f));
    if(res>=LLONG_MIN && res<=LLONG_MAX)
      return ZObjFromInt64((long long int)res);
    else
      return ZObjFromDouble(res);
}
ZObject ROUND(ZObject* args,int n)
{
    if(n!=1)
    {
        return Z_Err(ArgumentError,"1 argument needed");
        
    }
    if(args[0].type=='i' || args[0].type=='l')
    {
        return args[0];
        
    }
    if(args[0].type!=Z_FLOAT)
    {
        return Z_Err(TypeError,"Numeric argument needed!");
        
    }
    double res = (round(args[0].f));
    if(res>=LLONG_MIN && res<=LLONG_MAX)
      return ZObjFromInt64((long long int)res);
    else
      return ZObjFromDouble(res);
}
ZObject SQRT(ZObject* args,int n)
{
    if(n!=1)
    {
        return Z_Err(ArgumentError,"1 argument needed");
        
    }
    if(args[0].type==Z_FLOAT)
    {
        double res = (sqrt(args[0].f));
        return ZObjFromDouble(res);
    }
    else if(args[0].type==Z_INT)
    {
        double res = (sqrt(args[0].i));
        return ZObjFromDouble(res);
    }
    else if(args[0].type==Z_INT64)
    {
        double res = (sqrt(args[0].l));
        return ZObjFromDouble(res);
    }
    else
    {
        return Z_Err(TypeError,"Numeric argument needed!");
        
    }
}
ZObject RADIANS(ZObject* args,int n)
{
    if(n!=1)
    {
        return Z_Err(ArgumentError,"1 argument needed");
        
    }
    double pi = 2*acos(0.0);
    if(args[0].type==Z_FLOAT)
    {
        double res = ((args[0].f*pi/180));
        return ZObjFromDouble(res);
    }
    else if(args[0].type==Z_INT)
    {
        double res = ((args[0].i*pi/180));
        return ZObjFromDouble(res);
    }
    else if(args[0].type==Z_INT64)
    {
        double res = ((args[0].l*pi/180));
        return ZObjFromDouble(res);
    }
    else
    {
        return Z_Err(TypeError,"Numeric argument needed!");
        
    }
}
ZObject LOG(ZObject* args,int n)
{
    if(n!=1)
    {
        return Z_Err(ArgumentError,"1 argument needed");
        
    }
    if(args[0].type==Z_FLOAT)
    {
        if(args[0].f<1)
        {
            return Z_Err(MathError,"Input outside log;'s domain interval [1,inf)");
            
        }
        double res = (log(args[0].f));
        return ZObjFromDouble(res);
    }
    else if(args[0].type==Z_INT)
    {
        if(args[0].i<1)
        {
            return Z_Err(MathError,"Input outside log;'s domain interval [1,inf)");
            
        }
        double res = (log(args[0].i));
        return ZObjFromDouble(res);
    }
    else if(args[0].type==Z_INT64)
    {
        if(args[0].l<1)
        {
            return Z_Err(MathError,"Input outside log;'s domain interval [1,inf)");
            
        }
        double res = (log(args[0].l));
        return ZObjFromDouble(res);
    }
    else
    {
        return Z_Err(TypeError,"Numeric argument needed!");
        
    }
}
ZObject LOG10(ZObject* args,int n)
{
    if(n!=1)
    {
        return Z_Err(ArgumentError,"1 argument needed");
        
    }
    if(args[0].type==Z_FLOAT)
    {
        if(args[0].f<1)
        {
            return Z_Err(MathError,"Input outside log;'s domain interval [1,inf)");
            
        }
        double res = (log10(args[0].f));
        return ZObjFromDouble(res);
    }
    else if(args[0].type==Z_INT)
    {
        if(args[0].i<1)
        {
            return Z_Err(MathError,"Input outside log;'s domain interval [1,inf)");
            
        }
        double res = (log10(args[0].i));
        return ZObjFromDouble(res);
    }
    else if(args[0].type==Z_INT64)
    {
        if(args[0].l<1)
        {
            return Z_Err(MathError,"Input outside log;'s domain interval [1,inf)");
            
        }
        double res = (log10(args[0].l));
        return ZObjFromDouble(res);
    }
    else
    {
        return Z_Err(TypeError,"Numeric argument needed!");
        
    }
}
ZObject SIN(ZObject* args,int n)
{
    if(n!=1)
    {
        return Z_Err(ArgumentError,"1 argument needed");
        
    }
    if(args[0].type==Z_FLOAT)
    {
        double res = (sin(args[0].f));
        return ZObjFromDouble(res);
    }
    else if(args[0].type==Z_INT)
    {
        double res = (sin(args[0].i));
        return ZObjFromDouble(res);
    }
    else if(args[0].type==Z_INT64)
    {
        double res = (sin(args[0].l));
        return ZObjFromDouble(res);
    }
    else
    {
        return Z_Err(TypeError,"Numeric argument needed!");
        
    }
}
ZObject COS(ZObject* args,int n)
{
    if(n!=1)
    {
        return Z_Err(ArgumentError,"1 argument needed");
        
    }
    if(args[0].type==Z_FLOAT)
    {
        double res = (cos(args[0].f));
        return ZObjFromDouble(res);
    }
    else if(args[0].type==Z_INT)
    {
        double res = (cos(args[0].i));
        return ZObjFromDouble(res);
    }
    else if(args[0].type==Z_INT64)
    {
        double res = (cos(args[0].l));
        return ZObjFromDouble(res);
    }
    else
    {
        return Z_Err(TypeError,"Numeric argument needed!");
        
    }
}
ZObject TAN(ZObject* args,int n)
{
    if(n!=1)
    {
        return Z_Err(ArgumentError,"1 argument needed");
        
    }
    if(args[0].type==Z_FLOAT)
    {
        if(cos(args[0].f)==0)
        {
            return Z_Err(MathError,"Input outside tan;'s domain interval R - {pi/2 + npi}");
            
        }
        double res = (tan(args[0].f));
        return ZObjFromDouble(res);
    }
    else if(args[0].type==Z_INT)
    {
        if(cos(args[0].i)==0)
        {
            return Z_Err(MathError,"Input outside tan;'s domain interval R - {pi/2 + npi}");
            
        }
        double res = (tan(args[0].i));
        return ZObjFromDouble(res);
    }
    else if(args[0].type==Z_INT64)
    {
        if(cos(args[0].l)==0)
        {
            return Z_Err(MathError,"Input outside tan;'s domain interval R - {pi/2 + npi}");
            
        }
        double res = (tan(args[0].l));
        return ZObjFromDouble(res);
    }
    else
    {
        return Z_Err(TypeError,"Numeric argument needed!");
        
    }
}
ZObject SINH(ZObject* args,int n)
{
    if(n!=1)
    {
        return Z_Err(ArgumentError,"1 argument needed");
        
    }
    if(args[0].type==Z_FLOAT)
    {
        double res = (sinh(args[0].f));
        return ZObjFromDouble(res);
    }
    else if(args[0].type==Z_INT)
    {
        double res = (sinh(args[0].i));
        return ZObjFromDouble(res);
    }
    else if(args[0].type==Z_INT64)
    {
        double res = (sinh(args[0].l));
        return ZObjFromDouble(res);
    }
    else
    {
        return Z_Err(TypeError,"Numeric argument needed!");
        
    }
}
ZObject COSH(ZObject* args,int n)
{
    if(n!=1)
    {
        return Z_Err(ArgumentError,"1 argument needed");
        
    }
    if(args[0].type==Z_FLOAT)
    {
        double res = (cosh(args[0].f));
        return ZObjFromDouble(res);
    }
    else if(args[0].type==Z_INT)
    {
        double res = (cosh(args[0].i));
        return ZObjFromDouble(res);
    }
    else if(args[0].type==Z_INT64)
    {
        double res = (cosh(args[0].l));
        return ZObjFromDouble(res);
    }
    else
    {
        return Z_Err(TypeError,"Numeric argument needed!");
        
    }
}
ZObject TANH(ZObject* args,int n)
{
    if(n!=1)
    {
        return Z_Err(ArgumentError,"1 argument needed");
        
    }
    if(args[0].type==Z_FLOAT)
    {
        double res = (tanh(args[0].f));
        return ZObjFromDouble(res);
    }
    else if(args[0].type==Z_INT)
    {
        double res = (tanh(args[0].i));
        return ZObjFromDouble(res);
    }
    else if(args[0].type==Z_INT64)
    {
        double res = (tanh(args[0].l));
        return ZObjFromDouble(res);
    }
    else
    {
        return Z_Err(TypeError,"Numeric argument needed!");
        
    }
}
ZObject ASIN(ZObject* args,int n)
{
    if(n!=1)
    {
        return Z_Err(ArgumentError,"1 argument needed");
        
    }
    if(args[0].type==Z_FLOAT)
    {
        if(args[0].f < -1 || args[0].f>1)
        {
            return Z_Err(MathError,"Input outside asin;'s domain interval [-1,1]");
            
        }
        double res = (asin(args[0].f));
        return ZObjFromDouble(res);
    }
    else if(args[0].type==Z_INT)
    {
        if(args[0].i < -1 || args[0].i>1)
        {
            return Z_Err(MathError,"Input outside asin;'s domain interval [-1,1]");
            
        }
        double res = (asin(args[0].i));
        return ZObjFromDouble(res);
    }
    else if(args[0].type==Z_INT64)
    {
        if(args[0].l < -1 || args[0].l>1)
        {
            return Z_Err(MathError,"Input outside asin;'s domain interval [-1,1]");
            
        }
        double res = (asin(args[0].l));
        return ZObjFromDouble(res);
    }
    else
    {
        return Z_Err(TypeError,"Numeric argument needed!");
        
    }
}
ZObject ACOS(ZObject* args,int n)
{
    if(n!=1)
    {
        return Z_Err(ArgumentError,"1 argument needed");
        
    }
    if(args[0].type==Z_FLOAT)
    {
        if(args[0].f < -1 || args[0].f>1)
        {
            return Z_Err(MathError,"Input outside acos;'s domain interval [-1,1]");
            
        }
        double res = (acos(args[0].f));
        return ZObjFromDouble(res);
    }
    else if(args[0].type==Z_INT)
    {
        if(args[0].i < -1 || args[0].i>1)
        {
            return Z_Err(MathError,"Input outside acos;'s domain interval [-1,1]");
            
        }
        double res = (acos(args[0].i));
        return ZObjFromDouble(res);
    }
    else if(args[0].type==Z_INT64)
    {
        if(args[0].l < -1 || args[0].l>1)
        {
            return Z_Err(MathError,"Input outside acos;'s domain interval [-1,1]");
            
        }
        double res = (acos(args[0].l));
        return ZObjFromDouble(res);
    }
    else
    {
        return Z_Err(TypeError,"Numeric argument needed!");
        
    }
}
ZObject ATAN(ZObject* args,int n)
{
    if(n!=1)
    {
        return Z_Err(ArgumentError,"1 argument needed");
        
    }
    if(args[0].type==Z_FLOAT)
    {
        double res = (atan(args[0].f));
        return ZObjFromDouble(res);
    }
    else if(args[0].type==Z_INT)
    {
        double res = (atan(args[0].i));
        return ZObjFromDouble(res);
    }
    else if(args[0].type==Z_INT64)
    {
        double res = (atan(args[0].l));
        return ZObjFromDouble(res);
    }
    else
    {
        return Z_Err(TypeError,"Numeric argument needed!");
        
    }
}
ZObject ASINH(ZObject* args,int n)
{
    if(n!=1)
    {
        return Z_Err(ArgumentError,"1 argument needed");
        
    }
    if(args[0].type==Z_FLOAT)
    {
        double res = (asinh(args[0].f));
        return ZObjFromDouble(res);
    }
    else if(args[0].type==Z_INT)
    {
        double res = (asinh(args[0].i));
        return ZObjFromDouble(res);
    }
    else if(args[0].type==Z_INT64)
    {
        double res = (asinh(args[0].l));
        return ZObjFromDouble(res);
    }
    else
    {
        return Z_Err(TypeError,"Numeric argument needed!");
        
    }
}
ZObject ACOSH(ZObject* args,int n)
{
    if(n!=1)
    {
        return Z_Err(ArgumentError,"1 argument needed");
        
    }
    if(args[0].type==Z_FLOAT)
    {
        if(args[0].f<1)
        {
            return Z_Err(MathError,"Input outside acosh;'s domain interval [1,inf)");
            
        }
        double res = (acosh(args[0].f));
        return ZObjFromDouble(res);
    }
    else if(args[0].type==Z_INT)
    {
        if(args[0].i<1)
        {
            return Z_Err(MathError,"Input outside acosh;'s domain interval [1,inf)");
            
        }
        double res = (acosh(args[0].i));
        return ZObjFromDouble(res);
    }
    else if(args[0].type==Z_INT64)
    {
        if(args[0].l<1)
        {
            return Z_Err(MathError,"Input outside acosh;'s domain interval [1,inf)");
            
        }
        double res = (acosh(args[0].l));
        return ZObjFromDouble(res);
    }
    else
    {
        return Z_Err(TypeError,"Numeric argument needed!");
        
    }
}
ZObject ATANH(ZObject* args,int n)
{
    if(n!=1)
    {
        return Z_Err(ArgumentError,"1 argument needed");
        
    }
    if(args[0].type==Z_FLOAT)
    {
        if(args[0].f <= -1 || args[0].f>=1)
        {
            return Z_Err(MathError,"Input outside atanh;'s domain interval (-1,1)");
            
        }
        double res = (atanh(args[0].f));
        return ZObjFromDouble(res);
    }
    else if(args[0].type==Z_INT)
    {
        if(args[0].i <= -1 || args[0].i>=1)
        {
            return Z_Err(MathError,"Input outside atanh;'s domain interval (-1,1)");
            
        }
        double res = (atanh(args[0].i));
        return ZObjFromDouble(res);
    }
    else if(args[0].type==Z_INT64)
    {
        if(args[0].l <= -1 || args[0].l>=1)
        {
            return Z_Err(MathError,"Input outside atanh;'s domain interval (-1,1)");
            
        }
        double res = (atanh(args[0].l));
        return ZObjFromDouble(res);
    }
    else
    {
        return Z_Err(TypeError,"Numeric argument needed!");
        
    }
}
