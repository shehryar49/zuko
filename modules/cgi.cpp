#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "cgi.h"
using namespace std;
Klass* CgiFile;
vector<string> split(string s,const string& x)
{
	unsigned int  k = 0;
	vector<string> list;
	while(s.find(x)!=std::string::npos)
	{
		k = s.find(x);
		list.push_back(s.substr(0,k));
		s = s.substr(k+x.length());
	}
	list.push_back(s);
	return list;
}
int to_decimal(string HEX)
{
    int ans = 0;
    std::reverse(HEX.begin(),HEX.end());
    int x = 1;
    for(auto e: HEX)
    {
        if(e>='0' && e<='9')
          ans+= (e-48)*x;
        else if(isalpha(e))
        {
          e = toupper(e);
          ans+=(e-55)*x;
        }
        x*=16;
    }
    return ans;
}
string url_decode(const string& s)
{
    string res = "";
    int k = 0;
    while(k<s.length())
    {
        if(s[k]=='%')
        {
            string HEX;
            HEX+=s[k+1];
            HEX+=s[k+2];
            k+=2;
            res+= (char)to_decimal(HEX);
        }
        else if(s[k]=='+')
        {
            res+=" ";
        }
        else
        {
            res+=s[k];
        }
        k+=1;
    }
    return res;
}
string substr(int x,int y,string& str)//crash safe substring
{
  string res;
  if(x<0 || x>=str.length() || y<0 || y>=str.length())
    return res;//return empty string
  for(int i=x;i<=y;i++)
    res+=str[i];
  return res;
}
vector<string> splitIgnQuotes(string& s,char x)
{
  string read;
  vector<string> parts;
  bool inquotes = false;
  for(auto e: s)
  {
    if(e == '"')
    {
     inquotes = !inquotes;
     read+='"';
    }
    else if(e == x)
    {
      parts.push_back(read);
      read="";
    }
    else
      read+=e;
  }
  parts.push_back(read);
  return parts;
}
Dictionary* parse_multipart(char* data,int len,const string& boundary,bool& hadError)
{
  hadError = true;
  if(len<=0)
    return nullptr;
  int k = 0;
  string b;
  Dictionary* d = vm_allocDict();
  while (k < len && data[k]!='\r')
  {
    b+=data[k];
    k+=1;
  }
  if(b!=boundary)
  {
    if(b.substr(2)!=boundary)
      return nullptr;
  }
  k+=2;//skip newline character
  char ch;
  std::unordered_map<string,string> headers;
  string headername;
  string read;
  string aux ;
  b = "\r\n"+(string)"--"+boundary;
  while(k<len)
  {
    headername = "";
    read = "";
	//read a header
    bool inquotes=false;
    while(k<len && data[k]!='\r')
    {
      ch = data[k];
      if(ch == ' ' && !inquotes)
        ;//ignore
      else if(ch == ':' && !inquotes)
      {
        if(headername != "")
          return nullptr;
        headername = read;
        read = "";
      }
      else if(ch == ';')
      {
        if(headername=="")
          return nullptr;
        read+=ch;
      }
      else if(isalpha(ch) && !inquotes)
        read+=tolower(ch);
      else if(ch == '"')
      {
        read+=ch;
        inquotes = !inquotes;
      }
      else
        read+=ch;
      ++k;
    }
    if(inquotes)
      return nullptr;
    headers.emplace(headername,read);
    k+=2;//skip CRLF
    string content;
    if(headername == "")//empty line,start reading content
    {
      while(k<len)
      {
        if(b[0] == data[k] && k+b.length()-1 < len)
        {
            //match boundary
            k+=1;
            int p = k-1;
            bool match = true;
            for(int l=1;l<b.length();l++,k++)
            {
                if(data[k] != b[l])
                {
                    match = false;
                    break;
                }
            }
            if(!match)
            {
                k = p;
                content+=data[k];
                k++;
                continue;
            }
            if(k+1<len && data[k]=='-' && data[k+1]=='-')//firefox adds "--" at last boundary
              k+=2;
            if(k+1<len && (data[k]!='\r' || data[k+1]!='\n'))//firefox adds "--" at last boundary
              return nullptr;
            k+=2;//skip CRLF
            break;//boundary was found end content
        }
        else
          content+=data[k];
        k+=1;
      }
      if(headers.find("content-disposition") == headers.end())
        return nullptr;
      aux = headers["content-disposition"];
      if(substr(0,9,aux)!="form-data;")
        return nullptr;
      vector<string> parts = splitIgnQuotes(aux,';');
      string name;
      string filename;
      for(auto part: parts)
      {
        if(substr(0,4,part)=="name=" && name=="")
        {
           aux = part.substr(5);
           if(aux.length()<3 || aux[0]!='"' || aux.back()!='"')
             return nullptr;
           aux.pop_back();
           name = aux.substr(1);
        }
        else if(part == "form-data")
        ;
        else if(substr(0,8,part)=="filename=" && filename=="")
        {
           aux = part.substr(9);
           if(aux.length()<3 || aux[0]!='"' || aux.back()!='"')
             return nullptr;
           aux.pop_back();
           filename = aux.substr(1);
           
        }
        else
          return nullptr;
      }
      if(filename =="")
        d->emplace(PObjFromStr(name),PObjFromStr(content));
      else
      {
        //file content uploaded
        KlassInstance* ki = vm_allocKlassInstance();
        ki->klass = CgiFile;
        ki->members.emplace("filename",PObjFromStr(filename));
        if(headers.find("content-type")!=headers.end())
        {
          aux = headers["content-type"];
          ki->members.emplace("type",PObjFromStr(aux));
        }
        auto btArr = vm_allocByteArray();
        btArr->resize(content.length());
        memcpy(&btArr->at(0),&content[0],content.length());
        PltObject rr;
        rr.type = PLT_BYTEARR;
        rr.ptr = (void*)btArr;
        ki->members.emplace("content",rr);
        d->emplace(PObjFromStr(name),PObjFromKlassInst(ki));
      }
        
      headers.clear();
    }
  }
  hadError = false;
  return d;
}
Dictionary* GET()
{
       char* q = getenv("QUERY_STRING");
       if(!q)
         return nullptr;
       string query = q;
       Dictionary* m = vm_allocDict();
       if(query == "")
         return m;
       vector<string> pairs = split(q,"&");
       for(auto pair: pairs)
       {
           vector<string> eq = split(pair,"=");
           if(eq.size()!=2)
             return nullptr;
           eq[0] = url_decode(eq[0]);
           eq[1] = url_decode(eq[1]);
           m->emplace(PObjFromStr(eq[0]),PObjFromStr(eq[1]));
       }
       return m;
}
bool caseInsensitiveCmp(string a,string b)
{
  if(a.length() != b.length())
    return false;
  for(size_t i = 0;i<a.length();i++)
  {
    if(tolower(a[i])!=tolower(b[i]))
      return false;
  }
  return true;
}
void consumeSpace(char* data,int len,int k=0)
{
  while(k<len && data[k]==' ')
    k++;
  
}
void consumeSpace(const string& data,int& k)
{
  while(k<data.length() && data[k]==' ')
    k++;
  
}
Dictionary* POST(bool& err)
{

       char* q = getenv("CONTENT_LENGTH");
       char* t = getenv("CONTENT_TYPE");
       if(!q || !t)
       {
           err = true;
           return nullptr;
       }
       string type = t;
       int len = atoi(q);
       char* s = new char[len+1];
       fread(s,sizeof(char),len,stdin);
       s[len] = 0;
       string data ;
       if(caseInsensitiveCmp(substr(0,18,type),"multipart/form-data")) // multipart form
       {
        int k=19;
        consumeSpace(type,k);
        if(k>=type.length() || data[k]!=';')
          k++;
        consumeSpace(type,k);
        string boundary;
        if(caseInsensitiveCmp(substr(k,k+8,type),"boundary="))
        {
          boundary = type.substr(k+9);
          if(boundary.length() < 1)
          {
            err=true;
            return nullptr;
          }
        }
        else
        {
          err=true;
          return nullptr;
        }
        return parse_multipart(s,len,boundary,err);
       }
       else if(type=="application/x-www-form-urlencoded")
       {
         data = s;
         delete[] s;
         Dictionary* m = vm_allocDict();
         if(data == "")
           return m;
         vector<string> pairs = split(data,"&");
         for(auto pair: pairs)
         {
             vector<string> eq = split(pair,"=");
             if(eq.size()!=2)
               return nullptr;
             eq[0] = url_decode(eq[0]);
             eq[1] = url_decode(eq[1]);
             m->emplace(PObjFromStr(eq[0]),PObjFromStr(eq[1]));
         }
         return m;
       }
       else
       {
         err = true;
         return nullptr;
       }

}
PltObject FormData(PltObject* args,int n)
{
    if(n!=0)
        return Plt_Err(ARGUMENT_ERROR,"0 arguments needed!");
    char* method = getenv("REQUEST_METHOD");
    if(!method)
        return Plt_Err(UNKNOWN_ERROR,"Environment variable REQUEST_METHOD not found!");
    if(string(method)=="GET")
    {
       Dictionary* d = vm_allocDict();
       d = GET();
       if(!d)
          return Plt_Err(UNKNOWN_ERROR,"Error parsing request(bad or unsupported format)");
       return PObjFromDict(d);
       
    }
    else if(string(method)=="POST")
    {
       bool hadErr=false;
       Dictionary* d = POST(hadErr);
       if(!d || hadErr)
          return Plt_Err(UNKNOWN_ERROR,"Error parsing request(bad or unsupported format)");
       return PObjFromDict(d);
    }
    else
      return Plt_Err(VALUE_ERROR,"Unknown method "+(string)method);
}
PltObject nil;
PltObject init()
{
    nil.type = 'n';
    Module* d = vm_allocModule();
    CgiFile = vm_allocKlass();
    CgiFile->members[(string)"filename"]  = nil;
    CgiFile->members[(string)"content"] = nil;
    CgiFile->members[(string)"contentType"] = nil;
    CgiFile->name = "cgi.File";
    d->members.emplace("FormData",PObjFromMethod("cgi.FormData",&FormData,CgiFile));
    d->members.emplace("cookies",PObjFromFunction("cgi.cookies",&cookies));
    
    //Function FormData is not a member of CgiFile class but we want the class  not to be
    //garbage collected when FormData function object is reachable 
    d->members.emplace("File",PObjFromKlass(CgiFile));
    return PObjFromModule(d);
}

PltObject cookies(PltObject* args,int n)
{
  if(n!=0)
    return Plt_Err(ARGUMENT_ERROR,"0 arguments needed!");
  char* cookie = getenv("HTTP_COOKIE");
  if(!cookie)
    return Plt_Err(VALUE_ERROR,"No cookies!");
  string data = cookie;
  vector<string> parts = split(data,"; ");
  Dictionary* dict = vm_allocDict();
  for(auto e: parts)
  {
    vector<string> eq = split(e,"=");
    if(eq.size()!=2)
      return Plt_Err(VALUE_ERROR,"Bad or unsupported format");
    dict->emplace(PObjFromStr(eq[0]),PObjFromStr(eq[1]));
  }
  return PObjFromDict(dict);
}
extern "C" void unload()
{
}