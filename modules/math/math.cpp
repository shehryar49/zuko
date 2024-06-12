#include "math.h"
#include <cmath>
#include <climits>

using namespace std;
zobject nil;
zobject init()
{

    nil.type = Z_NIL;
    zmodule* moduleObject = vm_alloc_zmodule();//allocate a module object

    moduleObject->name = "math";//module name is math
    //Add functions to the module
    zmodule_add_fun(moduleObject,"floor",&FLOOR);
    zmodule_add_fun(moduleObject,"ceil",&CEIL);
    zmodule_add_fun(moduleObject,"round",&ROUND);
    zmodule_add_fun(moduleObject,"sin",&SIN);
    zmodule_add_fun(moduleObject,"cos",&COS);
    zmodule_add_fun(moduleObject,"tan",&TAN);
    zmodule_add_fun(moduleObject,"asin",&ASIN);
    zmodule_add_fun(moduleObject,"acos",&ACOS);
    zmodule_add_fun(moduleObject,"atan",&ATAN);
    zmodule_add_fun(moduleObject,"sinh",&SINH);
    zmodule_add_fun(moduleObject,"cosh",&COSH);
    zmodule_add_fun(moduleObject,"tanh",&TANH);
    zmodule_add_fun(moduleObject,"asinh",&ASINH);
    zmodule_add_fun(moduleObject,"acosh",&ACOSH);
    zmodule_add_fun(moduleObject,"atanh",&ATANH);
    zmodule_add_fun(moduleObject,"sqrt",&SQRT);
    zmodule_add_fun(moduleObject,"trunc",&TRUNC);
    zmodule_add_fun(moduleObject,"radians",&RADIANS);
    zmodule_add_fun(moduleObject,"log",&LOG);
    zmodule_add_fun(moduleObject,"log10",&LOG10);
    return zobj_from_module(moduleObject);
}

zobject FLOOR(zobject* args,int n)
{
    //rr stands for return result
    if(n!=1)
        return z_err(ArgumentError,"1 argument needed");
    if(args[0].type=='i' || args[0].type=='l')
        return args[0];
    if(args[0].type!=Z_FLOAT)
        return z_err(TypeError,"Numeric argument needed!");    
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
        return z_err(ArgumentError,"1 argument needed");
        
    }
    if(args[0].type=='i' || args[0].type=='l')
    {
        return args[0];
        
    }
    if(args[0].type!=Z_FLOAT)
    {
        return z_err(TypeError,"Numeric argument needed!");
        
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
        return z_err(ArgumentError,"1 argument needed");
        
    }
    if(args[0].type=='i' || args[0].type=='l')
    {
        return args[0];
        
    }
    if(args[0].type!=Z_FLOAT)
    {
        return z_err(TypeError,"Numeric argument needed!");
        
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
        return z_err(ArgumentError,"1 argument needed");
        
    }
    if(args[0].type=='i' || args[0].type=='l')
    {
        return args[0];
        
    }
    if(args[0].type!=Z_FLOAT)
    {
        return z_err(TypeError,"Numeric argument needed!");
        
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
        return z_err(ArgumentError,"1 argument needed");
        
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
        return z_err(TypeError,"Numeric argument needed!");
        
    }
}
zobject RADIANS(zobject* args,int n)
{
    if(n!=1)
    {
        return z_err(ArgumentError,"1 argument needed");
        
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
        return z_err(TypeError,"Numeric argument needed!");
        
    }
}
zobject LOG(zobject* args,int n)
{
    if(n!=1)
    {
        return z_err(ArgumentError,"1 argument needed");
        
    }
    if(args[0].type==Z_FLOAT)
    {
        if(args[0].f<1)
        {
            return z_err(MathError,"Input outside log;'s domain interval [1,inf)");
            
        }
        double res = (log(args[0].f));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT)
    {
        if(args[0].i<1)
        {
            return z_err(MathError,"Input outside log;'s domain interval [1,inf)");
            
        }
        double res = (log(args[0].i));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT64)
    {
        if(args[0].l<1)
        {
            return z_err(MathError,"Input outside log;'s domain interval [1,inf)");
            
        }
        double res = (log(args[0].l));
        return zobj_from_double(res);
    }
    else
    {
        return z_err(TypeError,"Numeric argument needed!");
        
    }
}
zobject LOG10(zobject* args,int n)
{
    if(n!=1)
    {
        return z_err(ArgumentError,"1 argument needed");
        
    }
    if(args[0].type==Z_FLOAT)
    {
        if(args[0].f<1)
        {
            return z_err(MathError,"Input outside log;'s domain interval [1,inf)");
            
        }
        double res = (log10(args[0].f));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT)
    {
        if(args[0].i<1)
        {
            return z_err(MathError,"Input outside log;'s domain interval [1,inf)");
            
        }
        double res = (log10(args[0].i));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT64)
    {
        if(args[0].l<1)
        {
            return z_err(MathError,"Input outside log;'s domain interval [1,inf)");
            
        }
        double res = (log10(args[0].l));
        return zobj_from_double(res);
    }
    else
    {
        return z_err(TypeError,"Numeric argument needed!");
        
    }
}
zobject SIN(zobject* args,int n)
{
    if(n!=1)
    {
        return z_err(ArgumentError,"1 argument needed");
        
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
        return z_err(TypeError,"Numeric argument needed!");
        
    }
}
zobject COS(zobject* args,int n)
{
    if(n!=1)
    {
        return z_err(ArgumentError,"1 argument needed");
        
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
        return z_err(TypeError,"Numeric argument needed!");
        
    }
}
zobject TAN(zobject* args,int n)
{
    if(n!=1)
    {
        return z_err(ArgumentError,"1 argument needed");
        
    }
    if(args[0].type==Z_FLOAT)
    {
        if(cos(args[0].f)==0)
        {
            return z_err(MathError,"Input outside tan;'s domain interval R - {pi/2 + npi}");
            
        }
        double res = (tan(args[0].f));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT)
    {
        if(cos(args[0].i)==0)
        {
            return z_err(MathError,"Input outside tan;'s domain interval R - {pi/2 + npi}");
            
        }
        double res = (tan(args[0].i));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT64)
    {
        if(cos(args[0].l)==0)
        {
            return z_err(MathError,"Input outside tan;'s domain interval R - {pi/2 + npi}");
            
        }
        double res = (tan(args[0].l));
        return zobj_from_double(res);
    }
    else
    {
        return z_err(TypeError,"Numeric argument needed!");
        
    }
}
zobject SINH(zobject* args,int n)
{
    if(n!=1)
    {
        return z_err(ArgumentError,"1 argument needed");
        
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
        return z_err(TypeError,"Numeric argument needed!");
        
    }
}
zobject COSH(zobject* args,int n)
{
    if(n!=1)
    {
        return z_err(ArgumentError,"1 argument needed");
        
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
        return z_err(TypeError,"Numeric argument needed!");
        
    }
}
zobject TANH(zobject* args,int n)
{
    if(n!=1)
    {
        return z_err(ArgumentError,"1 argument needed");
        
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
        return z_err(TypeError,"Numeric argument needed!");
        
    }
}
zobject ASIN(zobject* args,int n)
{
    if(n!=1)
    {
        return z_err(ArgumentError,"1 argument needed");
        
    }
    if(args[0].type==Z_FLOAT)
    {
        if(args[0].f < -1 || args[0].f>1)
        {
            return z_err(MathError,"Input outside asin;'s domain interval [-1,1]");
            
        }
        double res = (asin(args[0].f));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT)
    {
        if(args[0].i < -1 || args[0].i>1)
        {
            return z_err(MathError,"Input outside asin;'s domain interval [-1,1]");
            
        }
        double res = (asin(args[0].i));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT64)
    {
        if(args[0].l < -1 || args[0].l>1)
        {
            return z_err(MathError,"Input outside asin;'s domain interval [-1,1]");
            
        }
        double res = (asin(args[0].l));
        return zobj_from_double(res);
    }
    else
    {
        return z_err(TypeError,"Numeric argument needed!");
        
    }
}
zobject ACOS(zobject* args,int n)
{
    if(n!=1)
    {
        return z_err(ArgumentError,"1 argument needed");
        
    }
    if(args[0].type==Z_FLOAT)
    {
        if(args[0].f < -1 || args[0].f>1)
        {
            return z_err(MathError,"Input outside acos;'s domain interval [-1,1]");
            
        }
        double res = (acos(args[0].f));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT)
    {
        if(args[0].i < -1 || args[0].i>1)
        {
            return z_err(MathError,"Input outside acos;'s domain interval [-1,1]");
            
        }
        double res = (acos(args[0].i));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT64)
    {
        if(args[0].l < -1 || args[0].l>1)
        {
            return z_err(MathError,"Input outside acos;'s domain interval [-1,1]");
            
        }
        double res = (acos(args[0].l));
        return zobj_from_double(res);
    }
    else
    {
        return z_err(TypeError,"Numeric argument needed!");
        
    }
}
zobject ATAN(zobject* args,int n)
{
    if(n!=1)
    {
        return z_err(ArgumentError,"1 argument needed");
        
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
        return z_err(TypeError,"Numeric argument needed!");
        
    }
}
zobject ASINH(zobject* args,int n)
{
    if(n!=1)
    {
        return z_err(ArgumentError,"1 argument needed");
        
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
        return z_err(TypeError,"Numeric argument needed!");
        
    }
}
zobject ACOSH(zobject* args,int n)
{
    if(n!=1)
    {
        return z_err(ArgumentError,"1 argument needed");
        
    }
    if(args[0].type==Z_FLOAT)
    {
        if(args[0].f<1)
        {
            return z_err(MathError,"Input outside acosh;'s domain interval [1,inf)");
            
        }
        double res = (acosh(args[0].f));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT)
    {
        if(args[0].i<1)
        {
            return z_err(MathError,"Input outside acosh;'s domain interval [1,inf)");
            
        }
        double res = (acosh(args[0].i));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT64)
    {
        if(args[0].l<1)
        {
            return z_err(MathError,"Input outside acosh;'s domain interval [1,inf)");
            
        }
        double res = (acosh(args[0].l));
        return zobj_from_double(res);
    }
    else
    {
        return z_err(TypeError,"Numeric argument needed!");
        
    }
}
zobject ATANH(zobject* args,int n)
{
    if(n!=1)
    {
        return z_err(ArgumentError,"1 argument needed");
        
    }
    if(args[0].type==Z_FLOAT)
    {
        if(args[0].f <= -1 || args[0].f>=1)
        {
            return z_err(MathError,"Input outside atanh;'s domain interval (-1,1)");
            
        }
        double res = (atanh(args[0].f));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT)
    {
        if(args[0].i <= -1 || args[0].i>=1)
        {
            return z_err(MathError,"Input outside atanh;'s domain interval (-1,1)");
            
        }
        double res = (atanh(args[0].i));
        return zobj_from_double(res);
    }
    else if(args[0].type==Z_INT64)
    {
        if(args[0].l <= -1 || args[0].l>=1)
        {
            return z_err(MathError,"Input outside atanh;'s domain interval (-1,1)");
            
        }
        double res = (atanh(args[0].l));
        return zobj_from_double(res);
    }
    else
    {
        return z_err(TypeError,"Numeric argument needed!");
        
    }
}
