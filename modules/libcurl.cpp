/*
Libcurl Binding for Plutonium(easy interface only)
The original code of libcurl is not modified in anyway. This is just a wrapper around libcurl
and requires libcurl libraries to be linked when compiling.
Written by Shahryar Ahmad
*/
#include "curl/curl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libcurl.h"
using namespace std;
struct MemoryStruct
{
  vector<uint8_t>* memory;//bytearray of memory
};

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  vector<uint8_t>* mem = (vector<uint8_t>*)userp;
  size_t prevsize = mem->size();
  mem->resize(prevsize + realsize);
  memcpy(&(mem->at(prevsize)), contents, realsize);
  return realsize;
}
PltObject wmcallback;
size_t WMCallbackHandler(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  //send newly received memory to callback function
  auto btArr = vm_allocByteArray();
  btArr->resize(realsize);
  memcpy(&btArr->at(0), contents, realsize);
  PltObject rr;
  PltObject p1;
  p1.type = PLT_BYTEARR;
  p1.ptr = (void*)btArr;
  vm_callObject(&wmcallback,&p1,1,&rr);
  return realsize;
}
struct CurlObject //Wrapper around the curl handle
{
  CURL* handle;
  std::string url;
  std::string useragent;
  char* postfields;
};
struct MimePart  //Wrapper around curl_mimepart
{
  curl_mimepart* mimepart;
};
PltObject nil;
Klass* curlklass;
Klass* mimeklass;
Klass* mimepartklass;
PltObject callback;//write function callback
PltObject init()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    PltObject nil;
    Module* d = vm_allocModule();
    //
    curlklass = new Klass;
  
    curlklass->name = "Curl";
    curlklass->members.emplace("__construct__",PObjFromMethod("Curl.__construct__",&curlklass__construct__,curlklass));
    curlklass->members.emplace("perform",PObjFromMethod("Curl.perform",&perform,curlklass));
    curlklass->members.emplace("setopt",PObjFromMethod("Curl.setopt",&setopt,curlklass));
    curlklass->members.emplace("getinfo",PObjFromMethod("Curl.getinfo",&getinfo,curlklass));
    curlklass->members.emplace("__del__",PObjFromMethod("Curl.__del__",&curlklass__del__,curlklass));
    curlklass->members.emplace("escape",PObjFromFunction("Curl.escape",&ESCAPE,curlklass));
    curlklass->members.emplace("unescape",PObjFromFunction("Curl.unescape",&UNESCAPE,curlklass));
    
    //
    mimepartklass = new Klass;
    mimepartklass->name = "mimepart";
    //
    mimeklass = new Klass;
    mimeklass->name = "mime";
    //add methods to object
    mimeklass->members.emplace("__construct__",PObjFromMethod("mime.__construct__",&mime__construct__,mimeklass));
    mimeklass->members.emplace("addpart",PObjFromMethod("mime.addpart",&addpart,mimeklass));
    mimeklass->members.emplace("__del__",PObjFromMethod("mime.__del__",&MIME__del__,mimeklass));
    
    //
    //these 3 classes are required during whole life of module
    //so we manually manage their memory
    //
    d->members.emplace("Curl",PObjFromKlass(curlklass));
    d->members.emplace("mime",PObjFromKlass(mimeklass));
    d->members.emplace(("strerror"),PObjFromFunction("libcurl.strerror",&STRERROR));
    d->members.emplace(("OPT_URL"),PObjFromInt64(CURLOPT_URL));
    d->members.emplace(("OPT_PORT"),PObjFromInt64(CURLOPT_PORT));
    d->members.emplace(("OPT_POSTFIELDS"),PObjFromInt64(CURLOPT_POSTFIELDS));
    d->members.emplace(("OPT_USERAGENT"),PObjFromInt64(CURLOPT_USERAGENT));
    d->members.emplace(("OPT_FOLLOWLOCATION"),PObjFromInt64(CURLOPT_FOLLOWLOCATION));
    d->members.emplace(("OPT_WRITEFUNCTION"),PObjFromInt64(CURLOPT_WRITEFUNCTION));
    d->members.emplace(("OPT_MIMEPOST"),PObjFromInt64(CURLOPT_MIMEPOST));
    d->members.emplace(("OPT_VERBOSE"),PObjFromInt64(CURLOPT_VERBOSE));
    
    d->members.emplace(("CURLE_OK"),PObjFromInt64(CURLE_OK));
    d->members.emplace(("WriteMemory"),PObjFromInt(0));
    d->members.emplace(("INFO_CONTENT_TYPE"),PObjFromInt64((long long int)CURLINFO_CONTENT_TYPE));
    d->members.emplace(("INFO_HTTP_CODE"),PObjFromInt64((long long int)CURLINFO_HTTP_CODE));
    d->members.emplace(("VERSION_NUM"),PObjFromInt(LIBCURL_VERSION_NUM));
    d->members.emplace(("VERSION_MAJOR"),PObjFromInt(LIBCURL_VERSION_MAJOR));
    d->members.emplace(("VERSION_MINOR"),PObjFromInt(LIBCURL_VERSION_MINOR));
    

    return PObjFromModule(d);
}
PltObject curlklass__del__(PltObject* args,int n)//called by the VM so no typechecking required
{
    KlassInstance* d = (KlassInstance*)args[0].ptr;
    if(d->members[".handle"].type == 'n')
      return nil;
    CurlObject* obj = (CurlObject*)(d->members)[".handle"].ptr;
    if(obj->postfields!=NULL)
      delete[] obj->postfields;
    curl_easy_cleanup(obj->handle);
    delete obj;
    (d->members)[".handle"] = nil;
    return nil;
}
PltObject STRERROR(PltObject* args,int n)
{
  if(n!=1)
  {
    return Plt_Err(ARGUMENT_ERROR,"1 argument needed!");
    
  }
  if(args[0].type!='l')
  {
    return Plt_Err(TYPE_ERROR,"Integer argument needed!");
    
  }
  string s = curl_easy_strerror((CURLcode)args[0].l);
  return PObjFromStr(s);
}
PltObject curlklass__construct__(PltObject* args,int n)
{
  if(n!=1)
  {
    return Plt_Err(ARGUMENT_ERROR,"1 arguments needed!");
    
  }
  KlassInstance* curobj = (KlassInstance*)args[0].ptr;
  CURL* curl = curl_easy_init();
  if(!curl)
  {
    return Plt_Err(UNKNOWN_ERROR,"failed");
    
  }
  //Return an object
  
  CurlObject* obj = new CurlObject;
  obj->handle = curl;
  obj->postfields = NULL;
  curobj->members.emplace((".handle"),PObjFromPtr(obj));
  return nil;
 
}
PltObject MIME__del__(PltObject* args,int n)
{
    if(n!=1)
        return Plt_Err(ARGUMENT_ERROR,"1 argument needed!");
    KlassInstance* k = (KlassInstance*)args[0].ptr;
    curl_mime* obj = (curl_mime*)k->members[".handle"].ptr;
    curl_mime_free(obj);
    return nil;
}
PltObject mime__construct__(PltObject* args,int n)
{
  if(n!=2)
  {
    return Plt_Err(ARGUMENT_ERROR,"2 arguments needed!");
    
  }
  if(args[1].type!='o' || ((KlassInstance*)args[1].ptr)->klass!=curlklass)
  {
    return Plt_Err(TYPE_ERROR,"Curl Object needed!");
    
  }
  KlassInstance* d = (KlassInstance*)args[0].ptr;
  KlassInstance* cobj = (KlassInstance*)args[1].ptr;
  CurlObject*  curlobj = (CurlObject*)cobj->members[".handle"].ptr;
  curl_mime* mime = curl_mime_init(curlobj->handle);
  
  //Add the mime handle to mime object
  d->members.emplace((".handle"),PObjFromPtr((void*)mime));
  return nil;
}
PltObject MIME_NAME(PltObject* args,int n)
{
    if(n!=2)
        return Plt_Err(ARGUMENT_ERROR,"2 argument needed!");
    if(args[1].type!='s')
        return Plt_Err(TYPE_ERROR,"Argument 2 must be string!");
    if(args[0].type!='o' || ((KlassInstance*)args[0].ptr)->klass!=mimepartklass)
      return Plt_Err(TYPE_ERROR,"Argument 1 must be a MimePart object");
  KlassInstance* d = (KlassInstance*)args[0].ptr;
  MimePart* obj = (MimePart*)(d->members)[".handle"].ptr;
  string& name = *(string*)args[1].ptr;
  curl_mime_name(obj->mimepart,name.c_str());
  return nil;
}
PltObject MIME_FILENAME(PltObject* args,int n)
{
    if(n!=2)
        return Plt_Err(ARGUMENT_ERROR,"2 argument needed!");
    if(args[1].type!='s')
        return Plt_Err(TYPE_ERROR,"Argument 2 must be string!");
    if(args[0].type!='o' || ((KlassInstance*)args[0].ptr)->klass!=mimepartklass)
      return Plt_Err(TYPE_ERROR,"Argument 1 must be a MimePart object");
  KlassInstance* d = (KlassInstance*)args[0].ptr;
  MimePart* obj = (MimePart*)(d->members)[".handle"].ptr;
  string& name = *(string*)args[1].ptr;
  curl_mime_filename(obj->mimepart,name.c_str());
  return nil;
}
PltObject MIME_CONTENTTYPE(PltObject* args,int n)
{
    if(n!=2)
        return Plt_Err(ARGUMENT_ERROR,"2 argument needed!");
    if(args[1].type!='s')
        return Plt_Err(TYPE_ERROR,"Argument 2 must be string!");
    if(args[0].type!='o' || ((KlassInstance*)args[0].ptr)->klass!=mimepartklass)
      return Plt_Err(TYPE_ERROR,"Argument 1 must be a MimePart object");
  KlassInstance* d = (KlassInstance*)args[0].ptr;
  MimePart* obj = (MimePart*)(d->members)[".handle"].ptr;
  string& name = *(string*)args[1].ptr;
  curl_mime_type(obj->mimepart,name.c_str());
  return nil;
}
PltObject MIME_DATA(PltObject* args,int n)
{
    if(n!=2)
        return Plt_Err(ARGUMENT_ERROR,"1 argument needed!");
    if(args[1].type!='c')
      return Plt_Err(TYPE_ERROR,"Argument 2 must be a Byte array!");
    if(args[0].type!='o' || ((KlassInstance*)args[0].ptr)->klass!=mimepartklass)
      return Plt_Err(TYPE_ERROR,"Argument 1 must be a MimePart object");
    KlassInstance* d = (KlassInstance*)args[0].ptr;
    MimePart* obj = (MimePart*)(d->members)[".handle"].ptr;
    auto l = *(vector<uint8_t>*)args[1].ptr;
    curl_mime_data(obj->mimepart,(const char*)&l[0],l.size());
    return nil;
}

PltObject addpart(PltObject* args,int n)
{
    if(n!=1)
        return Plt_Err(ARGUMENT_ERROR,"1 arguments needed!");
    if(args[0].type!='o' || ((KlassInstance*)args[0].ptr)->klass!=mimeklass)
      return Plt_Err(TYPE_ERROR,"Argument 1 must be a Mime object");
    KlassInstance* d = (KlassInstance*)args[0].ptr;
    curl_mime* obj = (curl_mime*)(d->members)[".handle"].ptr;
    curl_mimepart* mimepart = curl_mime_addpart(obj);
    MimePart* mp = new MimePart;
    mp->mimepart = mimepart;
    //add methods to object
    KlassInstance* part = vm_allocKlassInstance();
    part->klass = mimepartklass;
    part->members.emplace(("data"),PObjFromMethod("mimepart.data",&MIME_DATA,mimepartklass));
    part->members.emplace(("name"),PObjFromMethod("mimepart.name",&MIME_NAME,mimepartklass));
    part->members.emplace(("filename"),PObjFromMethod("mimepart.filename",&MIME_FILENAME,mimepartklass));
    part->members.emplace(("type"),PObjFromMethod("mimepart.type",&MIME_CONTENTTYPE,mimepartklass));
    part->members.emplace(("__del__"),PObjFromMethod("mimepart.__del__",&MIMEPART__del__,mimepartklass));
    part->members.emplace((".handle"),PObjFromPtr((void*)mp));

    return PObjFromKlassInst(part);
}
PltObject MIMEPART__del__(PltObject* args,int n)
{
    if(n!=1)
        return Plt_Err(ARGUMENT_ERROR,"1 argument needed!");
    KlassInstance* k = (KlassInstance*)args[0].ptr;
    MimePart* obj = (MimePart*)k->members[".handle"].ptr;
    delete obj;
    return nil;
}
PltObject setopt(PltObject* args,int n)
{
    if(n!=3)
    {
        return Plt_Err(ARGUMENT_ERROR,"3 arguments needed!");
        
    }
    if(args[1].type!=PLT_INT64)
    {
        return Plt_Err(TYPE_ERROR,"Argument 2 must an Integer!");
        
    }
    if(args[0].type!='o' || ((KlassInstance*)args[0].ptr)->klass!=curlklass)
    {
      return Plt_Err(TYPE_ERROR,"Argument 1 must be a Curl object");
      
    }
    long long int opt = args[1].l;
    if(opt==CURLOPT_URL)
    {
      if(args[2].type!=PLT_STR)
      {
        return Plt_Err(TYPE_ERROR,"URL option requires a string value");
         
      }
      KlassInstance* d = (KlassInstance*)args[0].ptr;
      CurlObject* obj = (CurlObject*)(d->members)[".handle"].ptr;
      obj->url = *(string*)args[2].ptr;
      CURLcode res = curl_easy_setopt(obj->handle,(CURLoption)opt,obj->url.c_str());
      if( res!= CURLE_OK)
      {
        return Plt_Err(UNKNOWN_ERROR,(string)curl_easy_strerror(res));
        
      }
      
    }
    else if(opt==CURLOPT_FOLLOWLOCATION)
    {
      if(args[2].type!=PLT_INT)
      {
        return Plt_Err(TYPE_ERROR,"FOLLOWLOCATION option requires an integer value");
         
      }
      KlassInstance* d = (KlassInstance*)args[0].ptr;
      CurlObject* obj = (CurlObject*)(d->members)[".handle"].ptr;
      CURLcode res = curl_easy_setopt(obj->handle,(CURLoption)opt,args[2].i);
      if( res!= CURLE_OK)
      {
        return Plt_Err(UNKNOWN_ERROR,(string)curl_easy_strerror(res));
        
      }
    }
    else if(opt==CURLOPT_VERBOSE)
    {
      if(args[2].type!=PLT_INT)
      {
        return Plt_Err(TYPE_ERROR,"VERBOSE option requires an integer value");
         
      }
      KlassInstance* d = (KlassInstance*)args[0].ptr;
      CurlObject* obj = (CurlObject*)(d->members)[".handle"].ptr;
      CURLcode res = curl_easy_setopt(obj->handle,(CURLoption)opt,args[2].i);
      if( res!= CURLE_OK)
      {
        return Plt_Err(UNKNOWN_ERROR,(string)curl_easy_strerror(res));
        
      }
    }
    else if(opt==CURLOPT_PORT)
    {
      if(args[2].type!=PLT_INT)
      {
        return Plt_Err(TYPE_ERROR,"PORT option requires an integer value");
         
      }
      KlassInstance* d = (KlassInstance*)args[0].ptr;
      CurlObject* obj = (CurlObject*)(d->members)[".handle"].ptr;
      CURLcode res = curl_easy_setopt(obj->handle,(CURLoption)opt,args[2].i);
      if( res!= CURLE_OK)
      {
        return Plt_Err(UNKNOWN_ERROR,(string)curl_easy_strerror(res));
        
      }
    }
    else if(opt==CURLOPT_MIMEPOST)
    {
      if(args[2].type!=PLT_OBJ)
      {
        return Plt_Err(TYPE_ERROR,"MIMEPOST option requires a MimeObject");
         
      }
      KlassInstance* d = (KlassInstance*)args[0].ptr;
      CurlObject* obj = (CurlObject*)(d->members)[".handle"].ptr;
      KlassInstance* e = (KlassInstance*)args[2].ptr;
      curl_mime* m = (curl_mime*)((e->members)[".handle"].ptr);
      CURLcode res = curl_easy_setopt(obj->handle,(CURLoption)opt,m);
      if( res!= CURLE_OK)
      {
        return Plt_Err(UNKNOWN_ERROR,(string)curl_easy_strerror(res));
        
      }
    }
    else if(opt==CURLOPT_WRITEFUNCTION)
    {
      if(args[2].type == PLT_FUNC || args[2].type == PLT_NATIVE_FUNC)
      {
        //set callback
        KlassInstance* d = (KlassInstance*)args[0].ptr;
        d->members.emplace("wmcallback",args[2]);
        wmcallback = args[2];
        CurlObject* obj = (CurlObject*)(d->members)[".handle"].ptr;
        CURLcode res = curl_easy_setopt(obj->handle,(CURLoption)opt,&WMCallbackHandler);
        if( res!= CURLE_OK)
          return Plt_Err(UNKNOWN_ERROR,(string)curl_easy_strerror(res));
        return nil;
      }
      if(args[2].type!=PLT_INT )
        return Plt_Err(TYPE_ERROR,"WRITEFUNCTION option requires an integer value");
      if(args[2].i!=0)
        return Plt_Err(TYPE_ERROR,"Invalid option value "+to_string(args[2].i));
      KlassInstance* d = (KlassInstance*)args[0].ptr;
      auto btArr = vm_allocByteArray();
      PltObject p1;
      p1.type = 'c';
      p1.ptr = (void*)btArr;
      d->members.emplace("data",p1);
      CurlObject* obj = (CurlObject*)(d->members)[".handle"].ptr;
      CURLcode res = curl_easy_setopt(obj->handle,(CURLoption)opt,&WriteMemoryCallback);
      if( res!= CURLE_OK)
      {
        return Plt_Err(UNKNOWN_ERROR,(string)curl_easy_strerror(res));
        
      }
      curl_easy_setopt(obj->handle,CURLOPT_WRITEDATA,btArr);
    }
    else if(opt==CURLOPT_USERAGENT)
    {
      if(args[2].type!=PLT_STR)
      {
        return Plt_Err(TYPE_ERROR,"USERAGENT option requires a string value");
         
      }
      KlassInstance* d = (KlassInstance*)args[0].ptr;
      CurlObject* obj = (CurlObject*)(d->members)[".handle"].ptr;
      obj->useragent = *(string*)args[2].ptr;
      CURLcode res = curl_easy_setopt(obj->handle,(CURLoption)opt,obj->useragent.c_str());
      if( res!= CURLE_OK)
      {
        return Plt_Err(UNKNOWN_ERROR,(string)curl_easy_strerror(res));
        
      }
    }
    else if(opt==CURLOPT_POSTFIELDS)
    {
      if(args[2].type!=PLT_BYTEARR)
        return Plt_Err(TYPE_ERROR,"POSTFIELDS option requires a byte array!");
      vector<uint8_t>& l = *(vector<uint8_t>*)args[2].ptr;
      if(l.size()==0)
        return Plt_Err(VALUE_ERROR,"Empty bytearray");
      char* postfields = new char[l.size()];
      memcpy(postfields,&l[0],l.size());
      KlassInstance* d = (KlassInstance*)args[0].ptr;
      CurlObject* obj = (CurlObject*)(d->members)[".handle"].ptr;
      obj->postfields = postfields;
      CURLcode res = curl_easy_setopt(obj->handle,(CURLoption)opt,postfields);
      if( res!= CURLE_OK)
        return Plt_Err(UNKNOWN_ERROR,(string)curl_easy_strerror(res));
      res = curl_easy_setopt(obj->handle,CURLOPT_POSTFIELDSIZE,l.size());
      
    }
    else
    {
      return Plt_Err(VALUE_ERROR,"Unknown option used");
      
    }
    return nil;
}
PltObject perform(PltObject* args,int n)
{
    if(n!=1)
    {
        return Plt_Err(ARGUMENT_ERROR,"1 argument needed!");
        
    }
    KlassInstance* d = (KlassInstance*)args[0].ptr;
    string tmp = ".handle";
    CurlObject* obj = (CurlObject*)(d->members)[".handle"].ptr;
    CURLcode res;
    res = curl_easy_perform(obj->handle);
    return PObjFromInt64((long long int)res);
}
PltObject getinfo(PltObject* args,int n)
{
    if(n!=2)
    {
        return Plt_Err(ARGUMENT_ERROR,"2 arguments needed!");
        
    }
    if(args[0].type!='o' || ((KlassInstance*)args[0].ptr)->klass!=curlklass)
    {
      return Plt_Err(TYPE_ERROR,"Argument 1 must be a Curl object");
      
    }
    if(args[1].type!='l')
    {
      return Plt_Err(TYPE_ERROR,"Argument 2 must an Integer!");
      
    }
    KlassInstance* d = (KlassInstance*)args[0].ptr;
    string tmp = ".handle";
    CurlObject* obj = (CurlObject*)(d->members)[".handle"].ptr;
    CURL* handle = obj->handle;
    if(args[1].l==CURLINFO_CONTENT_TYPE)
    {
      char* out;
      CURLcode res = curl_easy_getinfo(handle,(CURLINFO)args[1].l,&out);
      if(!out || res!=CURLE_OK)
      {
        return nil;
      }
      return PObjFromStr(out);

    }
    else if(args[1].l==CURLINFO_HTTP_CODE)
    {
      long out;
      CURLcode res = curl_easy_getinfo(handle,(CURLINFO)args[1].l,&out);
      if(res!=CURLE_OK)
      {
        return Plt_Err(UNKNOWN_ERROR,curl_easy_strerror(res));
        
      }
      return PObjFromInt64((long long int)out);
    }
    return nil;
}
/*curl__del__ does the cleanup as well
PltObject cleanup(PltObject* args,int n)
{
    if(n!=1)
    {
        return Plt_Err(ARGUMENT_ERROR,"1 arguments needed!");
        
    }
    if(args[0].type!='o' || ((KlassInstance*)args[0].ptr)->klass!=curlklass)
    {
      return Plt_Err(TYPE_ERROR,"Argument 1 must be a Curl object");
      
    }
    KlassInstance* d = (KlassInstance*)args[0].ptr;
    string tmp = ".handle";
    CurlObject* obj = (CurlObject*)(d->members)[".handle"].ptr;
    curl_easy_cleanup(obj->handle);
    return nil;
}*/


PltObject ESCAPE(PltObject* args,int n)
{
  if(n!=2)
  {
    return Plt_Err(ARGUMENT_ERROR,"2 Arguments required.");
    
  }
  if(args[0].type != PLT_OBJ || ((KlassInstance*)args[0].ptr) -> klass !=curlklass)
  {
    return Plt_Err(TYPE_ERROR,"Argument 1 must be a Curl Object");
    
  }
  if(args[1].type!=PLT_STR)
  {
    return Plt_Err(TYPE_ERROR,"Argument 1 must be a string");
    
  }
  KlassInstance* ki = (KlassInstance*)args[0].ptr;
  CurlObject* k = (CurlObject*)ki->members[".handle"].ptr;
  string& str = *(string*)args[1].ptr;
  string* res = vm_allocString();
  char* output = curl_easy_escape(k->handle,str.c_str(),str.length());
  size_t i = 0;
  while(output[i])
  {
    res->push_back(output[i]);
    ++i;
  }
  curl_free(output);
  return PObjFromStrPtr(res);
}
PltObject UNESCAPE(PltObject* args,int n)
{
  if(n!=2)
  {
    return Plt_Err(ARGUMENT_ERROR,"2 Arguments required.");
    
  }
  if(args[0].type != PLT_OBJ || ((KlassInstance*)args[0].ptr) -> klass !=curlklass)
  {
    return Plt_Err(TYPE_ERROR,"Argument 1 must be a Curl Object");
    
  }
  if(args[1].type!=PLT_STR)
  {
    return Plt_Err(TYPE_ERROR,"Argument 1 must be a string");
    
  }
  KlassInstance* ki = (KlassInstance*)args[0].ptr;
  CurlObject* k = (CurlObject*)ki->members[".handle"].ptr;
  string& str = *(string*)args[1].ptr;
  string* res = vm_allocString();
  int decodelen = 0;
  char* output = curl_easy_unescape(k->handle,str.c_str(),str.length(),&decodelen);
  size_t i = 0;
  while(i<decodelen)
  {
    res->push_back(output[i]);
    ++i;
  }
  curl_free(output);
  return PObjFromStrPtr(res);
}
extern "C" void unload()
{
  curl_global_cleanup();
  delete curlklass;
  delete mimeklass;
  delete mimepartklass;
}