#ifndef CRASH_OVERFLOW_H_
#define CRASH_OVERFLOW_H_
#include <limits.h>
#include <cfloat>
bool exponen_overflows(int a,int b)
{
    double c = pow(a,b);
    if(c==HUGE_VAL)
      return true;
    if(c>INT_MAX || c<INT_MIN)
      return true;
    return false;
}

bool exponen_overflows(long long int a,long long int b)
{
     double c = pow(a,b);
    if(c==HUGE_VAL)
      return true;
    if(c>LLONG_MAX || c<LLONG_MIN)
      return true;
    return false;
}

bool exponen_overflows(float a,float b)
{
    double d = pow(a,b);
    if(d>= -FLT_MAX && d<=FLT_MAX)
    {
        return false;
    }
    return true;
}
/*
Addition
*/


bool addition_overflows(int a,int x)
{
  if ((x > 0) && (a > INT_MAX - x))
  {
	return true;
  }
  if ((x < 0) && (a < INT_MIN - x))
  {
    return true;
  }
  return false;
}
bool addition_overflows(long long int a,long long int x)
{
  if ((x > 0) && (a > LLONG_MAX - x))
  {
	return true;
  }
  if ((x < 0) && (a < LLONG_MIN - x))
  {
    return true;
  }
  return false;
}
bool addition_overflows(float a,float x)
{
  if ((x > 0) && (a > FLT_MAX - x))
  {
	return true;
  }
  if ((x < 0) && (a < -FLT_MAX - x))
  {
    return true;
  }
  return false;
}
/*
Subtraction
*/
bool subtraction_overflows(int a,int x)
{
	if ((x < 0) && (a > INT_MAX + x)) return true;
    if ((x > 0) && (a < INT_MIN + x)) return true;
 return false;
}
bool subtraction_overflows(long long int a,long long int x)
{
	if ((x < 0) && (a > LLONG_MAX + x)) return true;
    if ((x > 0) && (a < LLONG_MIN + x)) return true;
 return false;
}
bool subtraction_overflows(float a,float x)
{
  if ((x < 0) && (a > FLT_MAX + x)) return true;
    if ((x > 0) && (a < -FLT_MAX + x)) return true;
 return false;
}
/*
Multiplication
*/
bool multiplication_overflows(int a,int b)
{

     if (a == 0 || b == 0)
        return false;
    volatile int result = a * b;
    if (a == result / b)
        return false;
    else
        return true;
}
bool multiplication_overflows(long long int a,long long int b)
{
  if (a == 0 || b == 0)
        return false;

    volatile long long result = a * b;
    if (a == result / b)
        return false;
    else
        return true;
}
bool multiplication_overflows(float a,float b)
{
  if (a == 0 || b == 0)
        return false;
    double result = (double)a*b;
    if(result>FLT_MAX || result< -FLT_MAX)
      return true;
    return false;
}
bool division_overflows(int op1, int op2) {

  if ( op1 == INT_MIN && op2 == -1 )  {
    return true;
  }
  return false;
}
bool division_overflows(long long int op1,long long int op2) {

  if ( op1 == LLONG_MIN && op2 == -1 )  {
    return true;
  }
  return false;
}
bool division_overflows(float op1,float op2) {

  
  return false;
}


#endif
