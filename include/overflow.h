/*MIT License

Copyright (c) 2022 Shahryar Ahmad 

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/
#ifndef CRASH_OVERFLOW_H_
#define CRASH_OVERFLOW_H_
#include <limits.h>
#include <cfloat>
bool exponen_overflows(int32_t a,int32_t b)
{
    double c = pow(a,b);
    if(c==HUGE_VAL)
      return true;
    if(c>(double)INT_MAX || c<(double)INT_MIN)
      return true;
    return false;
}

bool exponen_overflows(int64_t a,int64_t b)
{
     double c = pow(a,b);
    if(c==HUGE_VAL || c==-HUGE_VAL)
      return true;
    if(c>(double)LLONG_MAX || c<(double)LLONG_MIN)
      return true;
    return false;
}

bool exponen_overflows(double a,double b)
{
    double d = pow(a,b);
    if(d>= -DBL_MAX && d<=DBL_MAX)
    {
        return false;
    }
    return true;
}
/*
Addition
*/


inline bool addition_overflows(int32_t a,int32_t x)
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
inline bool addition_overflows(int64_t a,int64_t x)
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
inline bool addition_overflows(double a,double x)
{
  if ((x > 0) && (a > DBL_MAX - x))
  {
	return true;
  }
  if ((x < 0) && (a < -DBL_MAX - x))
  {
    return true;
  }
  return false;
}
/*
Subtraction
*/
inline bool subtraction_overflows(int32_t a,int32_t x)
{
	if ((x < 0) && (a > INT_MAX + x)) return true;
    if ((x > 0) && (a < INT_MIN + x)) return true;
 return false;
}
inline bool subtraction_overflows(int64_t a,int64_t x)
{
	if ((x < 0) && (a > LLONG_MAX + x)) return true;
    if ((x > 0) && (a < LLONG_MIN + x)) return true;
 return false;
}
inline bool subtraction_overflows(double a,double x)
{
  if ((x < 0) && (a > DBL_MAX + x)) return true;
    if ((x > 0) && (a < -DBL_MAX + x)) return true;
 return false;
}
/*
Multiplication
*/
bool multiplication_overflows(int32_t a,int32_t b)
{

     if (a == 0 || b == 0)
        return false;
    volatile int result = a * b;
    if (a == result / b)
        return false;
    else
        return true;
}
bool multiplication_overflows(int64_t a,int64_t b)
{
  if (a == 0 || b == 0)
        return false;

    volatile long long result = a * b;
    if (a == result / b)
        return false;
    else
        return true;
}
bool multiplication_overflows(double a,double b)
{
  if (a == 0 || b == 0)
        return false;
    double result = (double)a*b;
    if(result>DBL_MAX || result< -DBL_MAX)
      return true;
    return false;
}
inline bool division_overflows(int32_t op1, int32_t op2) {

  if ( op1 == INT_MIN && op2 == -1 )  {
    return true;
  }
  return false;
}
inline bool division_overflows(int64_t op1,int64_t op2) {

  if ( op1 == LLONG_MIN && op2 == -1 )  {
    return true;
  }
  return false;
}
inline bool division_overflows(double op1,double op2) 
{
  return false;
}


#endif
