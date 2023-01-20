/*
Libcurl Binding for Plutonium(easy interface only)
The original code of libcurl is not modified in anyway. This is just a wrapper around libcurl
and requires libcurl libraries to be linked when compiling.
Written by Shahryar Ahmad
*/
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libcurl.h"
using namespace std;
struct MemoryStruct
{
  char* memory;
  size_t size;
};

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;
  char* ptr = new char[mem->size + realsize+1];
  for(int i=0;i<mem->size;i++)
    ptr[i] = mem->memory[i];
  delete[] mem->memory;
  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
  return realsize;
}
struct CurlObject //Wrapper around the curl handle
{
  CURL* handle;
  std::string url;
  std::string useragent;
  char* postfields;
  MemoryStruct chunk;
};
struct MimePart  //Wrapper around curl_mimepart
{
  char* name;
  char* data;
  curl_mimepart* mimepart;
};

Klass* curlklass;
Klass* mimeklass;
Klass* mimepartklass;
void init(PltObject* rr)
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    PltObject nil;
    Module* d = vm_allocModule();
    //
    curlklass = vm_allocKlass();
    curlklass->name = "Curl";
    curlklass->members.emplace("__construct__",PltObjectFromMethod("Curl.__construct__",&curlklass__construct__,curlklass));
    curlklass->members.emplace("perform",PltObjectFromMethod("Curl.perform",&perform,curlklass));
    curlklass->members.emplace("cleanup",PltObjectFromMethod("Curl.cleanup",&cleanup,curlklass));
    curlklass->members.emplace("setopt",PltObjectFromMethod("Curl.setopt",&setopt,curlklass));
    curlklass->members.emplace("getinfo",PltObjectFromMethod("Curl.getinfo",&getinfo,curlklass));
    curlklass->members.emplace("data",PltObjectFromMethod("Curl.data",&data,curlklass));
    curlklass->members.emplace(".destroy",PltObjectFromMethod(".destroy",&curlklass__destroy,curlklass));
    curlklass->members.emplace("escape",PltObjectFromFunction("escape",&ESCAPE,curlklass));
    curlklass->members.emplace("unescape",PltObjectFromFunction("unescape",&UNESCAPE,curlklass));
    
    //
    mimepartklass = vm_allocKlass();
    mimepartklass->name = "mimepart";
    //
    mimeklass = vm_allocKlass();
    mimeklass->name = "mime";
    //add methods to object
    mimeklass->members.emplace("__construct__",PltObjectFromMethod("mime.__construct__",&mime__construct__,mimeklass));
    mimeklass->members.emplace("addpart",PltObjectFromMethod("mime.addpart",&addpart,mimeklass));
    mimeklass->members.emplace("free",PltObjectFromMethod("mime.free",&MIME_FREE,mimeklass));
    mimeklass->members.emplace("mimepart",PltObjectFromKlass(mimepartklass));//to keep mimepart class alive when mime class is alive
    
    //
    
    //
    d->members.emplace("Curl",PltObjectFromKlass(curlklass));
    d->members.emplace("mime",PltObjectFromKlass(mimeklass));
    d->members.emplace(("strerror"),PltObjectFromFunction("libcurl.strerror",&STRERROR));
    d->members.emplace(("OPT_URL"),PltObjectFromInt64(CURLOPT_URL));
    d->members.emplace(("OPT_PORT"),PltObjectFromInt64(CURLOPT_PORT));
    d->members.emplace(("OPT_POSTFIELDS"),PltObjectFromInt64(CURLOPT_POSTFIELDS));
    d->members.emplace(("OPT_USERAGENT"),PltObjectFromInt64(CURLOPT_USERAGENT));
    d->members.emplace(("OPT_FOLLOWLOCATION"),PltObjectFromInt64(CURLOPT_FOLLOWLOCATION));
    d->members.emplace(("OPT_WRITEFUNCTION"),PltObjectFromInt64(CURLOPT_WRITEFUNCTION));
    d->members.emplace(("OPT_MIMEPOST"),PltObjectFromInt64(CURLOPT_MIMEPOST));
    d->members.emplace(("CURLE_OK"),PltObjectFromInt64(CURLE_OK));
    d->members.emplace(("WriteMemory"),PltObjectFromInt(0));
    d->members.emplace(("INFO_CONTENT_TYPE"),PltObjectFromInt64((long long int)CURLINFO_CONTENT_TYPE));
    d->members.emplace(("INFO_HTTP_CODE"),PltObjectFromInt64((long long int)CURLINFO_HTTP_CODE));
    d->members.emplace(("VERSION_NUM"),PltObjectFromInt(LIBCURL_VERSION_NUM));
    d->members.emplace(("VERSION_MAJOR"),PltObjectFromInt(LIBCURL_VERSION_MAJOR));
    d->members.emplace(("VERSION_MINOR"),PltObjectFromInt(LIBCURL_VERSION_MINOR));
    
    rr->type = PLT_MODULE;
    rr->ptr = (void*)d;
}
void curlklass__destroy(PltObject* args,int n,PltObject* rr)//called by the VM so no typechecking required
{
    KlassInstance* d = (KlassInstance*)args[0].ptr;
    CurlObject* obj = (CurlObject*)(d->members)[".handle"].ptr;
    delete[] obj->chunk.memory;
    if(obj->postfields!=NULL)
      delete[] obj->postfields;
    obj->chunk.memory = NULL;
    delete obj;

    (d->members)[".handle"] = PltObjectFromInt(0);
}
void STRERROR(PltObject* args,int n,PltObject* rr)
{
  if(n!=1)
  {
    *rr = Plt_Err(ARGUMENT_ERROR,"1 argument needed!");
    return;
  }
  if(args[0].type!='l')
  {
    *rr = Plt_Err(TYPE_ERROR,"Integer argument needed!");
    return;
  }
  string s = curl_easy_strerror((CURLcode)args[0].l);
  *rr = PltObjectFromString(s);
}
void curlklass__construct__(PltObject* args,int n,PltObject* rr)
{
  if(n!=1)
  {
    *rr = Plt_Err(ARGUMENT_ERROR,"1 arguments needed!");
    return;
  }
  KlassInstance* curobj = (KlassInstance*)args[0].ptr;
  CURL* curl = curl_easy_init();
  if(!curl)
  {
    *rr = Plt_Err(UNKNOWN_ERROR,"failed");
    return;
  }
  //Return an object

  //add methods to object
  
  CurlObject* obj = new CurlObject;
  obj->handle = curl;
  obj->chunk.memory = new char[1];
  obj->chunk.size = 0;
  obj->postfields = NULL;
  curobj->members.emplace((".handle"),PltObjectFromPointer(obj));
 
}
void MIME_FREE(PltObject* args,int n,PltObject* rr)
{
    if(n!=1)
    {
        *rr = Plt_Err(ARGUMENT_ERROR,"1 argument needed!");
        return;
    }
    KlassInstance* k = (KlassInstance*)args[0].ptr;
    curl_mime* obj = (curl_mime*)k->members[".handle"].ptr;
    curl_mime_free(obj);
}
void mime__construct__(PltObject* args,int n,PltObject* rr)
{
  if(n!=2)
  {
    *rr = Plt_Err(ARGUMENT_ERROR,"2 arguments needed!");
    return;
  }
  if(args[1].type!='o' || ((KlassInstance*)args[1].ptr)->klass!=curlklass)
  {
    *rr = Plt_Err(TYPE_ERROR,"Curl Object needed!");
    return;
  }
  KlassInstance* d = (KlassInstance*)args[0].ptr;
  KlassInstance* cobj = (KlassInstance*)args[1].ptr;
  CurlObject*  curlobj = (CurlObject*)cobj->members[".handle"].ptr;
  curl_mime* mime = curl_mime_init(curlobj->handle);
  
  //Add the mime handle to mime object
  d->members.emplace((".handle"),PltObjectFromPointer((void*)mime));
  //return nil
}
void MIME_NAME(PltObject* args,int n,PltObject* rr)
{
    if(n!=2)
    {
        *rr = Plt_Err(ARGUMENT_ERROR,"2 argument needed!");
        return;
    }
    if(args[1].type!='s')
    {
        *rr = Plt_Err(TYPE_ERROR,"Argument 2 must be string!");
        return;
    }
    if(args[0].type!='o' || ((KlassInstance*)args[0].ptr)->klass!=mimepartklass)
    {
      *rr = Plt_Err(TYPE_ERROR,"Argument 1 must be a MimePart object");
      return;
    }
  KlassInstance* d = (KlassInstance*)args[0].ptr;
  MimePart* obj = (MimePart*)(d->members)[".handle"].ptr;
  string& name = *(string*)args[1].ptr;
  obj->name = new char[name.length()+1];
  int i=0;
  size_t len = name.length();
    for(i=0;i<len;i++)
      obj->name[i] = name[i];
    obj->name[i] = 0;
    curl_mime_name(obj->mimepart,obj->name);
}
void MIME_DATA(PltObject* args,int n,PltObject* rr)
{
    if(n!=2)
    {
        *rr = Plt_Err(ARGUMENT_ERROR,"1 argument needed!");
        return;
    }
    if(args[1].type!='j')
    {
      *rr = Plt_Err(TYPE_ERROR,"Argument 2 must be a Byte List!");
      return;
    }
    if(args[0].type!='o' || ((KlassInstance*)args[0].ptr)->klass!=mimepartklass)
    {
      *rr = Plt_Err(TYPE_ERROR,"Argument 1 must be a MimePart object");
      return;
    }
    KlassInstance* d = (KlassInstance*)args[0].ptr;
    MimePart* obj = (MimePart*)(d->members)[".handle"].ptr;
    PltList l = *(PltList*)args[1].ptr;
    char* arr = new char[l.size()];
    for(int i=0;i<l.size();i++)
      arr[i] = l[i].i;
    obj->data = arr;
    curl_mime_data(obj->mimepart,arr,l.size());
}
void destroyMIMEPART(PltObject* args,int n,PltObject* rr)
{
    KlassInstance* d = (KlassInstance*)args[0].ptr;
    MimePart* obj = (MimePart*)(d->members)[".handle"].ptr;
    if(obj->name!=NULL)
      delete[] obj->name;
    if(obj->data!=NULL)
      delete[] obj->data;
    delete obj;
}
void addpart(PltObject* args,int n,PltObject* rr)
{
    if(n!=1)
    {
        *rr = Plt_Err(ARGUMENT_ERROR,"1 arguments needed!");
        return;
    }
    if(args[0].type!='o' || ((KlassInstance*)args[0].ptr)->klass!=mimeklass)
    {
      *rr = Plt_Err(TYPE_ERROR,"Argument 1 must be a Mime object");
      return;
    }
    KlassInstance* d = (KlassInstance*)args[0].ptr;

    curl_mime* obj = (curl_mime*)(d->members)[".handle"].ptr;
    curl_mimepart* mimepart = curl_mime_addpart(obj);
    MimePart* mp = new MimePart;
    mp->name = NULL;
    mp->data = NULL;
    mp->mimepart = mimepart;
    //add methods to object
    KlassInstance* part = vm_allocKlassInstance();
    part->klass = mimepartklass;
    part->members.emplace(("data"),PltObjectFromMethod("mimepart.data",&MIME_DATA,mimepartklass));
    part->members.emplace(("name"),PltObjectFromMethod("mimepart.name",&MIME_NAME,mimepartklass));
    part->members.emplace((".destroy"),PltObjectFromMethod("destroy",&destroyMIMEPART,mimepartklass));
    part->members.emplace((".handle"),PltObjectFromPointer((void*)mp));
    rr->type = PLT_OBJ;
    rr->ptr = (void*)part;
}
void setopt(PltObject* args,int n,PltObject* rr)
{
    if(n!=3)
    {
        *rr = Plt_Err(ARGUMENT_ERROR,"3 arguments needed!");
        return;
    }
    if(args[1].type!=PLT_INT64)
    {
        *rr = Plt_Err(TYPE_ERROR,"Argument 2 must an Integer!");
        return;
    }
    if(args[0].type!='o' || ((KlassInstance*)args[0].ptr)->klass!=curlklass)
    {
      *rr = Plt_Err(TYPE_ERROR,"Argument 1 must be a Curl object");
      return;
    }
    long long int opt = args[1].l;
    if(opt==CURLOPT_URL)
    {
      if(args[2].type!=PLT_STR)
      {
        *rr = Plt_Err(TYPE_ERROR,"URL option requires a string value");
        return; 
      }
      KlassInstance* d = (KlassInstance*)args[0].ptr;
      CurlObject* obj = (CurlObject*)(d->members)[".handle"].ptr;
      obj->url = *(string*)args[2].ptr;
      CURLcode res = curl_easy_setopt(obj->handle,(CURLoption)opt,obj->url.c_str());
      if( res!= CURLE_OK)
      {
        *rr = Plt_Err(UNKNOWN_ERROR,(string)curl_easy_strerror(res));
        return;
      }
      
    }
    else if(opt==CURLOPT_FOLLOWLOCATION)
    {
      if(args[2].type!=PLT_INT)
      {
        *rr = Plt_Err(TYPE_ERROR,"FOLLOWLOCATION option requires an integer value");
        return; 
      }
      KlassInstance* d = (KlassInstance*)args[0].ptr;
      CurlObject* obj = (CurlObject*)(d->members)[".handle"].ptr;
      CURLcode res = curl_easy_setopt(obj->handle,(CURLoption)opt,args[2].i);
      if( res!= CURLE_OK)
      {
        *rr = Plt_Err(UNKNOWN_ERROR,(string)curl_easy_strerror(res));
        return;
      }
    }
    else if(opt==CURLOPT_PORT)
    {
      if(args[2].type!=PLT_INT)
      {
        *rr = Plt_Err(TYPE_ERROR,"PORT option requires an integer value");
        return; 
      }
      KlassInstance* d = (KlassInstance*)args[0].ptr;
      CurlObject* obj = (CurlObject*)(d->members)[".handle"].ptr;
      CURLcode res = curl_easy_setopt(obj->handle,(CURLoption)opt,args[2].i);
      if( res!= CURLE_OK)
      {
        *rr = Plt_Err(UNKNOWN_ERROR,(string)curl_easy_strerror(res));
        return;
      }
    }
    else if(opt==CURLOPT_MIMEPOST)
    {
      if(args[2].type!=PLT_OBJ)
      {
        *rr = Plt_Err(TYPE_ERROR,"MIMEPOST option requires a MimeObject");
        return; 
      }
      KlassInstance* d = (KlassInstance*)args[0].ptr;
      CurlObject* obj = (CurlObject*)(d->members)[".handle"].ptr;
      KlassInstance* e = (KlassInstance*)args[2].ptr;
      curl_mime* m = (curl_mime*)((e->members)[".handle"].ptr);
      CURLcode res = curl_easy_setopt(obj->handle,(CURLoption)opt,m);
      if( res!= CURLE_OK)
      {
        *rr = Plt_Err(UNKNOWN_ERROR,(string)curl_easy_strerror(res));
        return;
      }
    }
    else if(opt==CURLOPT_WRITEFUNCTION)
    {
      if(args[2].type!=PLT_INT)
      {
        *rr = Plt_Err(TYPE_ERROR,"WRITEFUNCTION option requires an integer value");
        return; 
      }
      if(args[2].i!=0)
      {
        *rr = Plt_Err(TYPE_ERROR,"Invalid option value "+to_string(args[2].i));
        return; 
      }
      KlassInstance* d = (KlassInstance*)args[0].ptr;
      CurlObject* obj = (CurlObject*)(d->members)[".handle"].ptr;
      CURLcode res = curl_easy_setopt(obj->handle,(CURLoption)opt,&WriteMemoryCallback);
      if( res!= CURLE_OK)
      {
        *rr = Plt_Err(UNKNOWN_ERROR,(string)curl_easy_strerror(res));
        return;
      }
      curl_easy_setopt(obj->handle,CURLOPT_WRITEDATA,&obj->chunk);
    }
    else if(opt==CURLOPT_USERAGENT)
    {
      if(args[2].type!=PLT_STR)
      {
        *rr = Plt_Err(TYPE_ERROR,"USERAGENT option requires a string value");
        return; 
      }
      KlassInstance* d = (KlassInstance*)args[0].ptr;
      CurlObject* obj = (CurlObject*)(d->members)[".handle"].ptr;
      obj->useragent = *(string*)args[2].ptr;
      CURLcode res = curl_easy_setopt(obj->handle,(CURLoption)opt,obj->useragent.c_str());
      if( res!= CURLE_OK)
      {
        *rr = Plt_Err(UNKNOWN_ERROR,(string)curl_easy_strerror(res));
        return;
      }
    }
    else if(opt==CURLOPT_POSTFIELDS)
    {
      if(args[2].type!=PLT_LIST)
      {
        *rr = Plt_Err(TYPE_ERROR,"POSTFIELDS option requires a byte list");
        return; 
      }
      PltList l = *(PltList*)args[2].ptr;
      if(l.size()==0)

      {
        *rr = Plt_Err(VALUE_ERROR,"Empty list");
        return;
      }
      char* postfields = new char[l.size()];
      for(int i=0;i<l.size();i++)
      {
        postfields[i] = l[i].i;
      }

      KlassInstance* d = (KlassInstance*)args[0].ptr;
      CurlObject* obj = (CurlObject*)(d->members)[".handle"].ptr;
      obj->postfields = postfields;
      CURLcode res = curl_easy_setopt(obj->handle,(CURLoption)opt,postfields);
      if( res!= CURLE_OK)
      {
        *rr = Plt_Err(UNKNOWN_ERROR,(string)curl_easy_strerror(res));
        return;
      }
      curl_easy_setopt(obj->handle,CURLOPT_POSTFIELDSIZE,l.size());
    }
    else
    {
      *rr = Plt_Err(VALUE_ERROR,"Unknown option used");
      return;
    }
}
void perform(PltObject* args,int n,PltObject* rr)
{
    if(n!=1)
    {
        *rr = Plt_Err(ARGUMENT_ERROR,"1 argument needed!");
        return;
    }
    KlassInstance* d = (KlassInstance*)args[0].ptr;
    string tmp = ".handle";
    CurlObject* obj = (CurlObject*)(d->members)[".handle"].ptr;
    CURLcode res;
    res = curl_easy_perform(obj->handle);
    rr->type='l';
    rr->l = (long long int)res;
}
void getinfo(PltObject* args,int n,PltObject* rr)
{
    if(n!=2)
    {
        *rr = Plt_Err(ARGUMENT_ERROR,"2 arguments needed!");
        return;
    }
    if(args[0].type!='o' || ((KlassInstance*)args[0].ptr)->klass!=curlklass)
    {
      *rr = Plt_Err(TYPE_ERROR,"Argument 1 must be a Curl object");
      return;
    }
    if(args[1].type!='l')
    {
      *rr = Plt_Err(TYPE_ERROR,"Argument 2 must an Integer!");
      return;
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
        *rr = Plt_Err(UNKNOWN_ERROR,curl_easy_strerror(res));
        return;
      }
      *rr = PltObjectFromString(out);

    }
    else if(args[1].l==CURLINFO_HTTP_CODE)
    {
      long out;
      CURLcode res = curl_easy_getinfo(handle,(CURLINFO)args[1].l,&out);
      if(res!=CURLE_OK)
      {
        *rr = Plt_Err(UNKNOWN_ERROR,curl_easy_strerror(res));
        return;
      }
      rr->type = 'l';
      rr->l = (long long int)out;
    }
}
void cleanup(PltObject* args,int n,PltObject* rr)
{
    if(n!=1)
    {
        *rr = Plt_Err(ARGUMENT_ERROR,"1 arguments needed!");
        return;
    }
    if(args[0].type!='o' || ((KlassInstance*)args[0].ptr)->klass!=curlklass)
    {
      *rr = Plt_Err(TYPE_ERROR,"Argument 1 must be a Curl object");
      return;
    }
    KlassInstance* d = (KlassInstance*)args[0].ptr;
    string tmp = ".handle";
    CurlObject* obj = (CurlObject*)(d->members)[".handle"].ptr;
    curl_easy_cleanup(obj->handle);
}

void data(PltObject* args,int n,PltObject* rr)
{
    if(n!=1)
    {
        *rr = Plt_Err(ARGUMENT_ERROR,"1 argument needed!");
        return;
    }
    if(args[0].type!='o' || ((KlassInstance*)args[0].ptr)->klass!=curlklass)
    {
      *rr = Plt_Err(TYPE_ERROR,"Argument 1 must be a Curl object");
      return;
    }
    KlassInstance* d = (KlassInstance*)args[0].ptr;
    string tmp = ".handle";
    CurlObject* obj = (CurlObject*)(d->members)[".handle"].ptr;
    PltList* p = vm_allocList();
    for(int i=0;i<obj->chunk.size;i++)
      p->push_back(PltObjectFromByte(obj->chunk.memory[i]));
    rr->type = PLT_LIST;
    rr->ptr = (void*)p; 
}
void ESCAPE(PltObject* args,int n,PltObject* rr)
{
  if(n!=2)
  {
    *rr = Plt_Err(ARGUMENT_ERROR,"2 Arguments required.");
    return;
  }
  if(args[0].type != PLT_OBJ || ((KlassInstance*)args[0].ptr) -> klass !=curlklass)
  {
    *rr = Plt_Err(TYPE_ERROR,"Argument 1 must be a Curl Object");
    return;
  }
  if(args[1].type!=PLT_STR)
  {
    *rr = Plt_Err(TYPE_ERROR,"Argument 1 must be a string");
    return;
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
  rr->type = PLT_STR;
  rr->ptr = (void*)res;
}
void UNESCAPE(PltObject* args,int n,PltObject* rr)
{
  if(n!=2)
  {
    *rr = Plt_Err(ARGUMENT_ERROR,"2 Arguments required.");
    return;
  }
  if(args[0].type != PLT_OBJ || ((KlassInstance*)args[0].ptr) -> klass !=curlklass)
  {
    *rr = Plt_Err(TYPE_ERROR,"Argument 1 must be a Curl Object");
    return;
  }
  if(args[1].type!=PLT_STR)
  {
    *rr = Plt_Err(TYPE_ERROR,"Argument 1 must be a string");
    return;
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
  rr->type = PLT_STR;
  rr->ptr = (void*)res;
}
extern "C" void unload()
{
  curl_global_cleanup();
}