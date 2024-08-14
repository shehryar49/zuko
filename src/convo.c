#include "convo.h"
#include <ctype.h>


char* double_to_str(double f)
{
	char buffer[100];
	snprintf(buffer,100,"%f",f);
    int k = strlen(buffer) - 1;
    
    while(buffer[k] == '0' && buffer[k-1] != '.')
        buffer[k--] = 0;
	return clone_str(buffer);
}
int64_t str_to_int64(const char* s)
{
	size_t k = 0;
	int64_t l = 0;
	char c;
	short sign = 1;
	if(s[0]=='-')
	{
		sign = -1;
		s++;
	}
	
	while((c = s[k]))
	{
		l = (l*10)+(c-48);
		k+=1;
	}
	return l*sign;
}

bool is_int64(const char* s)
{
    while(s[0]=='0')
      s++;
	int64_t tmp = str_to_int64(s);
	char str[50];
	snprintf(str,50,"%" PRId64,tmp);
	return strcmp(str,s) == 0;
}
bool isnum(const char* s)
{
    while(s[0]=='0' && s[1]!=0)
      s++;
	
	char str[50];
	int32_t tmp = str_to_int32(s);
	snprintf(str,50,"%" PRId32,tmp);
	return strcmp(str,s) == 0;
}

bool isdouble(const char* s)
{
	size_t len = strlen(s);
	if(len == 0)
		return false;

	size_t k = 0;
	bool found_period = false;

	while(k < len)
	{
		if(s[k] == '.' && k!=0 && k!=len-1 && !found_period)
			found_period = true;
		else if(isdigit(s[k]))
		;
		else
			return false;
        k+=1;
    }
    return true;
}

char buffer[50];

char* int32_to_str(int32_t x)
{
	snprintf(buffer,50,"%" PRId32,x);
	return clone_str(buffer);
}
char* int64_to_str(int64_t l)
{
	snprintf(buffer,50,"%" PRId64,l);
	return clone_str(buffer);
}
double str_to_double(const char* s)
{
	return atof(s);
}
int32_t str_to_int32(const char* s)
{
  return atoi(s);
}
