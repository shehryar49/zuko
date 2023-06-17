#ifndef _CONVO_SIMPLE_H_
#define _CONVO_SIMPLE_H_
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <sstream>
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
string str(double f)
{
	string x = to_string(f);
  
	while(x.length()>=1 && x[x.length()-1]=='0')//remove zeros at end
	    x.pop_back();
    if(x.length()>=1 && x[x.length()-1]=='.')
        x+='0';
    return x;
}
inline string str(int64_t l)
{
	return to_string(l);
}
inline double Float(string s)
{
	return atof(s.c_str());
}
int64_t toInt64(string s)
{
	size_t k = 0;
	int64_t l = 0;
	char c;
	short sign = 1;
	if(s[0]=='-')
	{
		sign = -1;
		s = s.substr(1,s.length()-1);
	}
	while(k<s.length())
	{
		c = s[k];
		l = (l*10)+(c-48);
		k+=1;
	}
	return l*sign;
}

inline int32_t Int(string s)
{
  return atoi(s.c_str());
}

bool isInt64(string s)
{
    while(s.length()>1 && s[0]=='0')
      s = s.substr(1);
	return (str(toInt64(s.c_str()))==s);
}
bool isnum(string s)
{
    while(s.length()>1 && s[0]=='0')
      s = s.substr(1);
	return (str(atoi(s.c_str()))==s);
}
string replace(string,string,string);
bool isaFloat(string s)
{
    if(s=="" || s.find('.')==std::string::npos)return false;
    if(s.length() <=8)//8 because the decimal point is also counted
    {
        s = replace(".","",s);
        if(std::all_of(s.begin(), s.end(), ::isdigit))
        {
            return true;
        }
        return false;
    }
    return false;
}

#endif
////
