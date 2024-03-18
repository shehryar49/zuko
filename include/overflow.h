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
#include <cstdint>
#include <math.h>

bool exponen_overflows(int32_t a,int32_t b);
bool exponen_overflows(int64_t a,int64_t b);
bool exponen_overflows(double a,double b);
/*
Addition
*/


bool addition_overflows(int32_t a,int32_t x);
bool addition_overflows(int64_t a,int64_t x);
bool addition_overflows(double a,double x);
/*
Subtraction
*/
bool subtraction_overflows(int32_t a,int32_t x);
bool subtraction_overflows(int64_t a,int64_t x);
bool subtraction_overflows(double a,double x);
/*
Multiplication
*/
bool multiplication_overflows(int32_t a,int32_t b);
bool multiplication_overflows(int64_t a,int64_t b);
bool multiplication_overflows(double a,double b);
bool division_overflows(int32_t op1, int32_t op2);
bool division_overflows(int64_t op1,int64_t op2);



#endif
