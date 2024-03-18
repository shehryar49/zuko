#include "convo.h"

string str(double f)
{
	string x = to_string(f);
  
	while(x.length()>=1 && x[x.length()-1]=='0')//remove zeros at end
	    x.pop_back();
    if(x.length()>=1 && x[x.length()-1]=='.')
        x+='0';
    return x;
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
