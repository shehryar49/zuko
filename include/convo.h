#ifndef _CONVO_SIMPLE_H_
#define _CONVO_SIMPLE_H_
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <algorithm>
#include <math.h>
using namespace std;
/////////

string str(int32_t);
int32_t Int(string);
/////////
using namespace std;
//This file contains functions for performing various conversions
inline string str(int32_t x)
{
	return to_string(x);
}
string str(double f);
inline string str(int64_t l)
{
	return to_string(l);
}
inline double Float(string s)
{
	return atof(s.c_str());
}
int64_t toInt64(string s);

inline int32_t Int(string s)
{
  return atoi(s.c_str());
}

bool isInt64(string s);
bool isnum(string s);
string replace(string,string,string);
bool isaFloat(string s);
#endif
////
