#ifndef PLT_UTILITY_H_
#define PLT_UTILITY_H_
#include <string>
using namespace std;

string IntToHex(int i)
{
    if(i==0)
        return "0x00";
    int r;
    string res = "";

    while(i!=0)
    {
        r = i%16;
        if(r<10)
        {
          res+=str((int)r);
        }
        else
        {
           res+= (char) (r+87);
        }
        i = i/16;
    }
    std::reverse(res.begin(),res.end());
    if(res.length()==1)
    {
        res = "0"+res;
    }
    res = "0x"+res;
    return res;
}

unsigned char tobyte(string s)
{

    std::reverse(s.begin(),s.end());
    size_t k = 0;
    int p = 0;
    unsigned char b = 0;
    while(k<s.length())
    {
        string a;
        a+=s[k];
        if(!isdigit(s[k]))
        {
            if(a=="a")
            {
                a = "10";
            }
            else if(a=="b")
                a = "11";
            else if(a=="c")
                a = "12";
            else if(a=="d")
                a = "13";
            else if(a=="e")
                a = "14";
            else if(a=="f")
               a = "15";
            else
            {
                printf("Invalid Byte literal\n");
                exit(0);
            }
        }
        b+= Int(a)*pow(16,p);
        k+=1;
        p+=1;
    }
    return b;
}
int32_t hexToInt32(const string& s)
{
    int32_t res = 0;
    int32_t p = 1;
    for(int32_t i=s.length()-1;i>=0;i--)
    {
        if(s[i] >= '0' && s[i]<='9')
        {
            res+= (s[i]-48) * p;
        }
        else if(s[i] >= 'a' && s[i]<='z')
        {
            res+= (s[i]-87) * p;
        }
        p<<=4;
    }
    return res;
}
int64_t hexToInt64(const string& s)
{
    int64_t res = 0;
    int64_t p = 1;
    for(int32_t i=s.length()-1;i>=0;i--)
    {
        if(s[i] >= '0' && s[i]<='9')
        {
            res+= (s[i]-48) * p;
        }
        else if(s[i] >= 'a' && s[i]<='z')
        {
            res+= (s[i]-87) * p;
        }
        
        p<<=4;
    }
    return res;
}
string addlnbreaks(string s,bool& hadErr)
{

    unsigned int k = 0;
    bool escaped = false;//check if last char was
    string r = "";
    while(k<s.length())
    {
        if(s[k]=='\\')
        {
            if(escaped)
            {
              escaped = false;
              r+="\\";
            }
            else
              {
              escaped = true;
              }
        }
        else if(escaped)
        {
            if(s[k]=='n')
            {
                r+='\n';
            }
            else if(s[k]=='r')
            {
                r+='\r';
            }
            else if(s[k]=='t')
            {
                r+='\t';
            }
            else if(s[k]=='v')
            {
                r+='\v';
            }
            else if(s[k]=='b')
            {
                r+='\b';
            }
            else if(s[k]=='a')
            {
                r+='\a';
            }
            else if(s[k]=='"')
            {
                r+='"';
            }
            else
            {
                hadErr = true;
                return "Unknown escape character: \\"+s.substr(k,1);
               
            }
            escaped = false;
        }
        else if(!escaped)
        {
            r+=s[k];
        }
        k+=1;
    }
   if(escaped)
   {
       hadErr = true;
       return "Error string contains non terminated escape chars";
   }
	return r;
}

#endif
