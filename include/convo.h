#ifndef _CONVO_SIMPLE_H_
#define _CONVO_SIMPLE_H_
//This file contains functions for performing various conversions

#include <stdlib.h>
#include <string>

inline std::string str(int32_t x)
{
	return std::to_string(x);
}
inline std::string str(int64_t l)
{
	return std::to_string(l);
}
inline double Float(std::string s)
{
	return atof(s.c_str());
}
inline int32_t Int(std::string s)
{
  return atoi(s.c_str());
}

std::string str(int32_t);
int32_t Int(std::string);
std::string str(double f);
int64_t toInt64(std::string s);
bool isInt64(std::string s);
bool isnum(std::string s);
std::string replace(std::string,std::string,std::string);
bool isaFloat(std::string s);

#endif
////
