#include "math.h"
#include <cmath>
#include <climits>

using namespace std;
zobject nil;
zobject init()
{

    nil.type = Z_NIL;
    zmodule* moduleObject = vm_allocModule();//allocate a module object

    moduleObject->name = "math";//module name is math
    //Add functions to the module
    Module_addNativeFun(moduleObject,"floor",&FLOOR);
    Module_addNativeFun(moduleObject,"ceil",&CEIL);
    Module_addNativeFun(moduleObject,"round",&ROUND);
    Module_addNativeFun(moduleObject,"sin",&SIN);
    Module_addNativeFun(moduleObject,"cos",&COS);
    Module_addNativeFun(moduleObject,"tan",&TAN);
    Module_addNativeFun(moduleObject,"asin",&ASIN);
    Module_addNativeFun(moduleObject,"acos",&ACOS);
    Module_addNativeFun(moduleObject,"atan",&ATAN);
    Module_addNativeFun(moduleObject,"sinh",&SINH);
    Module_addNativeFun(moduleObject,"cosh",&COSH);
    Module_addNativeFun(moduleObject,"tanh",&TANH);
    Module_addNativeFun(moduleObject,"asinh",&ASINH);
    Module_addNativeFun(moduleObject,"acosh",&ACOSH);
    Module_addNativeFun(moduleObject,"atanh",&ATANH);
    Module_addNativeFun(moduleObject,"sqrt",&SQRT);
    Module_addNativeFun(moduleObject,"trunc",&TRUNC);
    Module_addNativeFun(moduleObject,"radians",&RADIANS);
    Module_addNativeFun(moduleObject,"log",&LOG);
    Module_addNativeFun(moduleObject,"log10",&LOG10);
    return zobj_from_module(moduleObject);
}

zobject FLOOR(zobject* args,int n)
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
      return zobj_from_int64((long long int)res);
    else
      return zobj_from_double(res);
}
zobject CEIL(zobject* args,int n)
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
      return zobj_from_int64((long long int)res);
    else
      return zobj_from_double(res);
}
zobject TRUNC(zobject* args,int n)
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
      return zobj_from_int64((long long int)res);
    else
      return zobj_from_double(res);
}
zobject ROUND(zobject* args,int n)
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
      return zobj_from_int64((long long int)res);
    else
      return zobj_from_double(res);
}
zobject SQRT(zobject* args,int n)
{
    if(n!=1)
    {
        return Z_Err(ArgumentError,"1 argument needed");
        
    }
    if(args[0].type==Z_FLOAT)
    {
        double res = (sqrt(args[0].f));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT)
    {
        double res = (sqrt(args[0].i));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT64)
    {
        double res = (sqrt(args[0].l));
        return zobj_from_double(res);
    }
    else
    {
        return Z_Err(TypeError,"Numeric argument needed!");
        
    }
}
zobject RADIANS(zobject* args,int n)
{
    if(n!=1)
    {
        return Z_Err(ArgumentError,"1 argument needed");
        
    }
    double pi = 2*acos(0.0);
    if(args[0].type==Z_FLOAT)
    {
        double res = ((args[0].f*pi/180));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT)
    {
        double res = ((args[0].i*pi/180));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT64)
    {
        double res = ((args[0].l*pi/180));
        return zobj_from_double(res);
    }
    else
    {
        return Z_Err(TypeError,"Numeric argument needed!");
        
    }
}
zobject LOG(zobject* args,int n)
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
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT)
    {
        if(args[0].i<1)
        {
            return Z_Err(MathError,"Input outside log;'s domain interval [1,inf)");
            
        }
        double res = (log(args[0].i));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT64)
    {
        if(args[0].l<1)
        {
            return Z_Err(MathError,"Input outside log;'s domain interval [1,inf)");
            
        }
        double res = (log(args[0].l));
        return zobj_from_double(res);
    }
    else
    {
        return Z_Err(TypeError,"Numeric argument needed!");
        
    }
}
zobject LOG10(zobject* args,int n)
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
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT)
    {
        if(args[0].i<1)
        {
            return Z_Err(MathError,"Input outside log;'s domain interval [1,inf)");
            
        }
        double res = (log10(args[0].i));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT64)
    {
        if(args[0].l<1)
        {
            return Z_Err(MathError,"Input outside log;'s domain interval [1,inf)");
            
        }
        double res = (log10(args[0].l));
        return zobj_from_double(res);
    }
    else
    {
        return Z_Err(TypeError,"Numeric argument needed!");
        
    }
}
zobject SIN(zobject* args,int n)
{
    if(n!=1)
    {
        return Z_Err(ArgumentError,"1 argument needed");
        
    }
    if(args[0].type==Z_FLOAT)
    {
        double res = (sin(args[0].f));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT)
    {
        double res = (sin(args[0].i));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT64)
    {
        double res = (sin(args[0].l));
        return zobj_from_double(res);
    }
    else
    {
        return Z_Err(TypeError,"Numeric argument needed!");
        
    }
}
zobject COS(zobject* args,int n)
{
    if(n!=1)
    {
        return Z_Err(ArgumentError,"1 argument needed");
        
    }
    if(args[0].type==Z_FLOAT)
    {
        double res = (cos(args[0].f));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT)
    {
        double res = (cos(args[0].i));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT64)
    {
        double res = (cos(args[0].l));
        return zobj_from_double(res);
    }
    else
    {
        return Z_Err(TypeError,"Numeric argument needed!");
        
    }
}
zobject TAN(zobject* args,int n)
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
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT)
    {
        if(cos(args[0].i)==0)
        {
            return Z_Err(MathError,"Input outside tan;'s domain interval R - {pi/2 + npi}");
            
        }
        double res = (tan(args[0].i));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT64)
    {
        if(cos(args[0].l)==0)
        {
            return Z_Err(MathError,"Input outside tan;'s domain interval R - {pi/2 + npi}");
            
        }
        double res = (tan(args[0].l));
        return zobj_from_double(res);
    }
    else
    {
        return Z_Err(TypeError,"Numeric argument needed!");
        
    }
}
zobject SINH(zobject* args,int n)
{
    if(n!=1)
    {
        return Z_Err(ArgumentError,"1 argument needed");
        
    }
    if(args[0].type==Z_FLOAT)
    {
        double res = (sinh(args[0].f));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT)
    {
        double res = (sinh(args[0].i));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT64)
    {
        double res = (sinh(args[0].l));
        return zobj_from_double(res);
    }
    else
    {
        return Z_Err(TypeError,"Numeric argument needed!");
        
    }
}
zobject COSH(zobject* args,int n)
{
    if(n!=1)
    {
        return Z_Err(ArgumentError,"1 argument needed");
        
    }
    if(args[0].type==Z_FLOAT)
    {
        double res = (cosh(args[0].f));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT)
    {
        double res = (cosh(args[0].i));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT64)
    {
        double res = (cosh(args[0].l));
        return zobj_from_double(res);
    }
    else
    {
        return Z_Err(TypeError,"Numeric argument needed!");
        
    }
}
zobject TANH(zobject* args,int n)
{
    if(n!=1)
    {
        return Z_Err(ArgumentError,"1 argument needed");
        
    }
    if(args[0].type==Z_FLOAT)
    {
        double res = (tanh(args[0].f));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT)
    {
        double res = (tanh(args[0].i));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT64)
    {
        double res = (tanh(args[0].l));
        return zobj_from_double(res);
    }
    else
    {
        return Z_Err(TypeError,"Numeric argument needed!");
        
    }
}
zobject ASIN(zobject* args,int n)
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
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT)
    {
        if(args[0].i < -1 || args[0].i>1)
        {
            return Z_Err(MathError,"Input outside asin;'s domain interval [-1,1]");
            
        }
        double res = (asin(args[0].i));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT64)
    {
        if(args[0].l < -1 || args[0].l>1)
        {
            return Z_Err(MathError,"Input outside asin;'s domain interval [-1,1]");
            
        }
        double res = (asin(args[0].l));
        return zobj_from_double(res);
    }
    else
    {
        return Z_Err(TypeError,"Numeric argument needed!");
        
    }
}
zobject ACOS(zobject* args,int n)
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
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT)
    {
        if(args[0].i < -1 || args[0].i>1)
        {
            return Z_Err(MathError,"Input outside acos;'s domain interval [-1,1]");
            
        }
        double res = (acos(args[0].i));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT64)
    {
        if(args[0].l < -1 || args[0].l>1)
        {
            return Z_Err(MathError,"Input outside acos;'s domain interval [-1,1]");
            
        }
        double res = (acos(args[0].l));
        return zobj_from_double(res);
    }
    else
    {
        return Z_Err(TypeError,"Numeric argument needed!");
        
    }
}
zobject ATAN(zobject* args,int n)
{
    if(n!=1)
    {
        return Z_Err(ArgumentError,"1 argument needed");
        
    }
    if(args[0].type==Z_FLOAT)
    {
        double res = (atan(args[0].f));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT)
    {
        double res = (atan(args[0].i));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT64)
    {
        double res = (atan(args[0].l));
        return zobj_from_double(res);
    }
    else
    {
        return Z_Err(TypeError,"Numeric argument needed!");
        
    }
}
zobject ASINH(zobject* args,int n)
{
    if(n!=1)
    {
        return Z_Err(ArgumentError,"1 argument needed");
        
    }
    if(args[0].type==Z_FLOAT)
    {
        double res = (asinh(args[0].f));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT)
    {
        double res = (asinh(args[0].i));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT64)
    {
        double res = (asinh(args[0].l));
        return zobj_from_double(res);
    }
    else
    {
        return Z_Err(TypeError,"Numeric argument needed!");
        
    }
}
zobject ACOSH(zobject* args,int n)
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
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT)
    {
        if(args[0].i<1)
        {
            return Z_Err(MathError,"Input outside acosh;'s domain interval [1,inf)");
            
        }
        double res = (acosh(args[0].i));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT64)
    {
        if(args[0].l<1)
        {
            return Z_Err(MathError,"Input outside acosh;'s domain interval [1,inf)");
            
        }
        double res = (acosh(args[0].l));
        return zobj_from_double(res);
    }
    else
    {
        return Z_Err(TypeError,"Numeric argument needed!");
        
    }
}
zobject ATANH(zobject* args,int n)
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
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT)
    {
        if(args[0].i <= -1 || args[0].i>=1)
        {
            return Z_Err(MathError,"Input outside atanh;'s domain interval (-1,1)");
            
        }
        double res = (atanh(args[0].i));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT64)
    {
        if(args[0].l <= -1 || args[0].l>=1)
        {
            return Z_Err(MathError,"Input outside atanh;'s domain interval (-1,1)");
            
        }
        double res = (atanh(args[0].l));
        return zobj_from_double(res);
    }
    else
    {
        return Z_Err(TypeError,"Numeric argument needed!");
        
    }
}
