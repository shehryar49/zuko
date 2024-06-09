#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "cgi.h"
#include "klassobject.h"
#include "zobject.h"
#ifdef _WIN32
  #include<io.h>
  #include <fcntl.h>
#endif
using namespace std;

zclass* CgiFile;

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
int hexdigitToDecimal(char ch)
{
  if(ch >= 'A' && ch <='F')
    return ch - 'A' + 10;
  else if(ch >= 'a' && ch <='f')
    return ch - 'a' + 10;
  else if(ch >= '0' && ch <= '9')
    return ch - '0';
  return 69;
}
string url_decode(const string& s)
{
    string res = "";
    int k = 0;
    size_t len = s.length();
    while(k<len)
    {
        if(s[k]=='%' && k+2 < len)
        {
            k+=2;
            res += (char)( hexdigitToDecimal(s[k+1])*16 + hexdigitToDecimal(s[k+2]) );
        }
        else if(s[k]=='+')
            res+=" ";
        else
          res+=s[k];
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

zdict* parse_multipart(char* data,size_t len,const string& boundary,bool& hadError)
{

  hadError = true;
  size_t k = 0;
  string b;
  while (k < len && data[k]!='\r')
    b+=data[k++];
  
  if(b.substr(2)!=boundary)
    return nullptr;
  k+=2;//skip newline character
  
  char ch;
  std::unordered_map<string,string> headers;
  string headername;
  string read;
  string aux ;
  b = "\r\n"+(string)"--"+boundary;
  zdict* d = vm_allocDict();
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
            if(k+1<len && data[k]=='-' && data[k+1]=='-')
              k+=2;
            if(k+1<len && (data[k]!='\r' || data[k+1]!='\n'))
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
        zdict_emplace(d,zobj_from_str(name.c_str()),zobj_from_str(content.c_str()));
      else
      {
        //file content uploaded
        zclass_object* ki = vm_allocklassObject(CgiFile);
        zclassobj_set(ki,"filename",zobj_from_str(filename.c_str()));
        if(headers.find("content-type")!=headers.end())
        {
          aux = headers["content-type"];
          zclassobj_set(ki,"type",zobj_from_str(aux.c_str()));
        }
        auto btArr = vm_allocByteArray();
        zbytearr_resize(btArr,content.length());
        memcpy(btArr->arr,&content[0],content.length());
        zobject rr;
        rr.type = Z_BYTEARR;
        rr.ptr = (void*)btArr;
        zclassobj_set(ki,"content",rr);
        zdict_emplace(d,zobj_from_str(name.c_str()),zobj_from_classobj(ki));
      }
        
      headers.clear();
    }
  }
  hadError = false;
  return d;
}
zdict* GET()
{
       char* q = getenv("QUERY_STRING");
       if(!q)
         return nullptr;
       string query = q;
       zdict* m = vm_allocDict();
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
           zdict_emplace(m,zobj_from_str(eq[0].c_str()),zobj_from_str(eq[1].c_str()));
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
zdict* POST(bool& err)
{

       err = true;
       char* q = getenv("CONTENT_LENGTH");
       char* t = getenv("CONTENT_TYPE");
       if(!q || !t)
         return nullptr;
       string type = t;
       size_t len = atoll(q);
       char* s = new char[len+1];
       fread(s,sizeof(char),len,stdin);
       s[len] = 0;
      
       if(type=="application/x-www-form-urlencoded")
       {
         string data = s;
         zdict* m = vm_allocDict();
         if(data.length() == 0)
         {
           err = false;
           return m;
         }
         vector<string> pairs = split(data,"&");
         for(auto pair: pairs)
         {
             vector<string> eq = split(pair,"=");
             if(eq.size()!=2)
               return nullptr;
             eq[0] = url_decode(eq[0]);
             eq[1] = url_decode(eq[1]);
             zdict_emplace(m,zobj_from_str(eq[0].c_str()),zobj_from_str(eq[1].c_str()));
         }
         err = false;
         return m;
       }
       else if(caseInsensitiveCmp(substr(0,18,type),"multipart/form-data")) // multipart form
       {
        int k=19;
        consumeSpace(type,k);
        if(k>=type.length() || type[k]!=';')
          return nullptr;
        k+=1;
        consumeSpace(type,k);
        if(k >= type.length())
          return nullptr;
        string boundary;
        if(caseInsensitiveCmp(substr(k,k+8,type),"boundary="))
        {
          boundary = type.substr(k+9);
          if(boundary.length() < 1)
            return nullptr;
        }
        else
          return nullptr;
        return parse_multipart(s,len,boundary,err);
       }
       else
       {
         return nullptr;
       }

}
zobject FormData(zobject* args,int n)
{
    if(n!=0)
        return Z_Err(ArgumentError,"0 arguments needed!");
    char* method = getenv("REQUEST_METHOD");
    if(!method)
        return Z_Err(Error,"Environment variable REQUEST_METHOD not found!");
    if(string(method)=="GET")
    {
       zdict* d = vm_allocDict();
       d = GET();
       if(!d)
          return Z_Err(Error,"Error parsing request(bad or unsupported format)");
       return zobj_from_dict(d);
       
    }
    else if(string(method)=="POST")
    {
       bool hadErr=false;
       zdict* d = POST(hadErr);
       if(!d || hadErr)
          return Z_Err(Error,"Error parsing request(bad or unsupported format)");
       return zobj_from_dict(d);
    }
    else
    {
      string errmsg = (string)"Unknown method " + method;
      return Z_Err(ValueError,errmsg.c_str());
    }
}
zobject nil;
zobject init()
{
    #ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);//we dont want CRLF replaced by LF when
    //reading from stdin
    #endif
    nil.type = 'n';
    zmodule* d = vm_allocModule();
    CgiFile = vm_allocklass();
    zclass_addmember(CgiFile,"filename",nil);
    zclass_addmember(CgiFile,"content",nil);
    zclass_addmember(CgiFile,"contentType",nil);
    CgiFile->name = "cgi.File";

    zmodule_add_member(d,"FormData",zobj_from_method("cgi.FormData",&FormData,CgiFile));
    zmodule_add_member(d,"cookies",zobj_from_function("cgi.cookies",&cookies));
    
    //Function FormData is not a member of CgiFile class but we want the class  not to be
    //garbage collected when FormData function object is reachable 
    zmodule_add_member(d,"File",zobj_from_class(CgiFile));
    return zobj_from_module(d);
}

zobject cookies(zobject* args,int n)
{
  if(n!=0)
    return Z_Err(ArgumentError,"0 arguments needed!");
  char* cookie = getenv("HTTP_COOKIE");
  if(!cookie)
    return Z_Err(ValueError,"No cookies!");
  string data = cookie;
  vector<string> parts = split(data,"; ");
  zdict* dict = vm_allocDict();
  for(auto e: parts)
  {
    vector<string> eq = split(e,"=");
    if(eq.size()!=2)
      return Z_Err(ValueError,"Bad or unsupported format");
    zdict_emplace(dict,zobj_from_str(eq[0].c_str()),zobj_from_str(eq[1].c_str()));
  }
  return zobj_from_dict(dict);
}
extern "C" void unload()
{
}
