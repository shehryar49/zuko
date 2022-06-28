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
           if(r==10)
            res+="a";
           else if(r==11)
            res+="b";
           else if(r==12)
            res+="c";
           else if(r==13)
            res+="d";
           else if(r==14)
            res+="e";
           else if(r==15)
            res+="f";

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
    int k = 0;
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
string addlnbreaks(string s)
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
                lexErr("SyntaxError","Unknown escape character: \\"+s.substr(k,1),false);
               
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
       lexErr("SyntaxError","Error string contains non terminated escape chars",false);
   }
	return r;
}

#endif
