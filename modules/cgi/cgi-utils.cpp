#include <string>
#include <vector>

using namespace std;

std::string lowercase(const std::string& str)
{
    std::string result;
    for(auto ch: str)
        result += tolower(ch);
    return result;
}

void strip_spaces(std::string& str)
{
  while(str.length() > 0 && str[0] == ' ')
    str.erase(0);
  while(str.length() > 0 && str.back() == ' ' )
    str.pop_back();
}

vector<string> split(string s,const string& x)
{
	size_t  k = 0;
	vector<string> list;
	while( (k = s.find(x) ) != std::string::npos)
	{
		list.push_back(s.substr(0,k));
		s = s.substr(k+x.length());
	}
	list.push_back(s);
	return list;
}

int hexdigit_to_decimal(char ch)
{
  if(ch >= 'A' && ch <='F')
    return ch - 'A' + 10;
  else if(ch >= 'a' && ch <='f')
    return ch - 'a' + 10;
  else if(ch >= '0' && ch <= '9')
    return ch - '0';
  return 69;
}
bool is_hex_digit(char ch)
{
    return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9');
}
string url_decode(const string& s)
{
    string res = "";
    size_t k = 0;
    size_t len = s.length();
    while(k < len)
    {
        if(s[k]=='%' && k+2 < len && is_hex_digit(s[k+1]) && is_hex_digit(s[k+2]))
        {
            res += (char)( hexdigit_to_decimal(s[k+1])*16 + hexdigit_to_decimal(s[k+2]) );
            k+=2;
        }
        else if(s[k]=='+')
            res+=" ";
        else
          res+=s[k];
        k+=1;
    }
    return res;
}

