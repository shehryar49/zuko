#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "cgi.h"
#include "zapi.h"
#include "multipart.h"
#include "cgi-utils.h"

#ifdef _WIN32
  #include<io.h>
  #include <fcntl.h>
#endif
using namespace std;

zclass* part_class;

zdict* GET()
{
    char* q = getenv("QUERY_STRING");
    if(!q)
        return nullptr;
    zdict* m = vm_alloc_zdict();
    if(*q == 0)
        return m;
    vector<string> pairs = split(q,"&");
    for(auto pair: pairs)
    {
        vector<string> eq = split(pair,"=");
        if(eq.size()!=2)
            return nullptr;
        eq[0] = url_decode(eq[0]);
        eq[1] = url_decode(eq[1]);
        zobject key = zobj_from_str(eq[0].c_str());
        zobject val = zobj_from_str(eq[1].c_str());
        zdict_emplace(m,key,val);
    }
    return m;
}
zdict* POST()
{
    char* q = getenv("CONTENT_LENGTH");
    char* t = getenv("CONTENT_TYPE");
    if(!q || !t)
        return nullptr;
    string type = t;
    size_t len = atoll(q);
    char* s = new char[len+1];
    size_t read = fread(s,sizeof(char),len,stdin);
    s[read] = 0;
    
    if(type == "application/x-www-form-urlencoded")
    {
        zdict* m = vm_alloc_zdict();
        if(len == 0)
            return m;
        vector<string> pairs = split(s,"&");
        for(auto pair: pairs)
        {
            vector<string> eq = split(pair,"=");
            if(eq.size()!=2)
                return nullptr;
            eq[0] = url_decode(eq[0]);
            eq[1] = url_decode(eq[1]);
            zobject key = zobj_from_str(eq[0].c_str());
            zobject val = zobj_from_str(eq[1].c_str());
            zdict_emplace(m,key,val);
        }
        return m;
    }
    else if(strncmp(type.c_str(),"multipart/form-data",19) == 0) // multipart form
    {
        vector<string> parts = split(type,"; ");
        if(parts.size() != 2 || parts[0] != "multipart/form-data")
          return nullptr;
        vector<string> boundary_parts = split(parts[1],"=");
        if(boundary_parts.size()!=2 || boundary_parts[0]!="boundary")
          return nullptr;
        std::string boundary = boundary_parts[1];
        strip_spaces(boundary);
        boundary = "--" + boundary; //IMPORTANT!
        multipart_parser parser(s,len,boundary);
        try
        {
          return parser.parse();
        }
        catch(const parse_error& err)
        {
          return nullptr;
        }
    }
    else
    {
        return nullptr;
    }
}
zobject formdata(zobject* args,int32_t n)
{
    if(n!=0)
        return z_err(ArgumentError,"0 arguments needed!");

    char* method = getenv("REQUEST_METHOD");
    if(!method)
        return z_err(Error,"Environment variable REQUEST_METHOD not found!");
    
    if(string(method)=="GET")
    {
        zdict* d = GET();
        if(!d)
            return z_err(Error,"Error parsing request(bad or unsupported format)");
        return zobj_from_dict(d);   
    }
    else if(string(method)=="POST")
    {
        zdict* d = POST();
        if(!d)
            return z_err(Error,"Error parsing request(bad or unsupported format)");
        return zobj_from_dict(d);
    }
    else
    {
        string errmsg = (string)"Unknown method " + method;
        return z_err(ValueError,errmsg.c_str());
    }
}
zobject init()
{
    #ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);//we dont want CRLF replaced by LF when
    //reading from stdin
    #endif

    zmodule* cgi_module = vm_alloc_zmodule("cgi");

    part_class = vm_alloc_zclass("cgi.part");

    zclass_addmember(part_class,"filename",zobj_nil());
    zclass_addmember(part_class,"type",zobj_nil());
    zclass_addmember(part_class,"bytes",zobj_nil());

    zmodule_add_member(cgi_module,"formdata",zobj_from_method("cgi.formdata",&formdata,part_class));
    zmodule_add_member(cgi_module,"cookies",zobj_from_function("cgi.cookies",&cookies));
    //Function formdata is not a member of part class but we want the class  not to be
    //garbage collected when formdata function object is reachable 

    zmodule_add_member(cgi_module,"part",zobj_from_class(part_class));

    return zobj_from_module(cgi_module);
}
zobject cookies(zobject* args,int32_t n)
{
  if(n!=0)
    return z_err(ArgumentError,"0 arguments needed!");
  char* cookie = getenv("HTTP_COOKIE");
  if(!cookie)
    return z_err(Error,"Environment variable HTTP_COOKIE not set. Make sure to run this program in cgi-bin");
  string data = cookie;
  vector<string> parts = split(data,"; ");
  zdict* dict = vm_alloc_zdict();
  for(auto e: parts)
  {
    vector<string> eq = split(e,"=");
    if(eq.size()!=2)
      return z_err(ValueError,"Bad or unsupported format");
    zdict_emplace(dict,zobj_from_str(eq[0].c_str()),zobj_from_str(eq[1].c_str()));
  }
  return zobj_from_dict(dict);
}

void unload()
{
  //Nothing here
}
