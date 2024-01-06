
/*
Libcurl Binding for Zuko(easy interface only)
The original code of libcurl is not modified in anyway. This is just a wrapper around libcurl
and requires libcurl libraries to be linked when compiling.
Written by Shahryar Ahmad
*/
#ifdef _WIN32
  #define CURL_STATICLIB
  #pragma comment(lib,"crypt32.lib")
  #pragma comment(lib,"Normaliz.lib")
  #pragma comment(lib,"Ws2_32.lib")
  #pragma comment(lib,"Wldap32.lib")
  #pragma comment(lib,"libcurl_a.lib")
#endif
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "libcurl.h"
#include <string>
using namespace std;

//Default WriteMemory function
//pushes all the bytes to a bytearray
size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  ZByteArr* mem = (ZByteArr*)userp;
  size_t prevsize = mem->size;
  ZByteArr_resize(mem,prevsize + realsize);
  memcpy(mem->arr+prevsize, contents, realsize);
  return realsize;
}
//WriteMemory function which calls zuko callback and passes the bytes
ZObject wmcallback;
ZObject xfercallback;
size_t WMCallbackHandler(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  //send newly received memory to callback function
  auto btArr = vm_allocByteArray();
  ZByteArr_resize(btArr,realsize);
  memcpy(btArr->arr, contents, realsize);
  ZObject rr;
  ZObject p1;
  p1.type = Z_BYTEARR;
  p1.ptr = (void*)btArr;
  vm_callObject(&wmcallback,&p1,1,&rr);
  return realsize;
}
//xfer function
int xferfun(void* clientp,curl_off_t dltotal,curl_off_t dlnow,curl_off_t ultotal,curl_off_t ulnow)
{
  ZObject args[4];
  ZObject r;
  r.type = Z_INT64;
  r.l = dltotal;
  args[0] = r;
  r.l = dlnow;
  args[1] = r;
  r.l = ultotal;
  args[2] = r;
  r.l = ulnow;
  args[3] = r;
  vm_callObject(&xfercallback,args,4,&r);
  if(r.type == Z_INT)
    return r.i;
  return 0;
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
ZObject nil;
Klass* curlklass;
Klass* mimeklass;
Klass* mimepartklass;
ZObject callback;//write function callback
ZObject quickErr(Klass* k,string s)
{
  return Z_Err(k,s.c_str());
}
ZObject init()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    nil.type = Z_NIL;
    Module* d = vm_allocModule();
    //
    curlklass = vm_allocKlass();
    curlklass->name = "Curl";

    Klass_addNativeMethod(curlklass,"__construct__",&curlklass__construct__);
    Klass_addNativeMethod(curlklass,"perform",&perform);
    Klass_addNativeMethod(curlklass,"setopt",&setopt);
    Klass_addNativeMethod(curlklass,"getinfo",&getinfo);
    Klass_addNativeMethod(curlklass,"__del__",&curlklass__del__);
    Klass_addNativeMethod(curlklass,"escape",&ESCAPE);
    Klass_addNativeMethod(curlklass,"unescape",&UNESCAPE);
    
    //
    mimepartklass = vm_allocKlass();
    mimepartklass->name = "mimepart";
    //
    mimeklass = vm_allocKlass();
    mimeklass->name = "mime";
    //add methods to object
    Klass_addNativeMethod(mimeklass,"__construct__",&mime__construct__);
    Klass_addNativeMethod(mimeklass,"addpart",&addpart);
    Klass_addNativeMethod(mimeklass,"__del__",&MIME__del__);
    
    
    
    Module_addKlass(d,"Curl",curlklass);
    Module_addKlass(d,"mime",mimeklass);
    Module_addNativeFun(d,"strerror",&STRERROR);
    Module_addMember(d,("OPT_URL"),ZObjFromInt64(CURLOPT_URL));
    Module_addMember(d,("OPT_PORT"),ZObjFromInt64(CURLOPT_PORT));
    Module_addMember(d,("OPT_POSTFIELDS"),ZObjFromInt64(CURLOPT_POSTFIELDS));
    Module_addMember(d,("OPT_USERAGENT"),ZObjFromInt64(CURLOPT_USERAGENT));
    Module_addMember(d,("OPT_FOLLOWLOCATION"),ZObjFromInt64(CURLOPT_FOLLOWLOCATION));
    Module_addMember(d,("OPT_WRITEFUNCTION"),ZObjFromInt64(CURLOPT_WRITEFUNCTION));
    Module_addMember(d,("OPT_MIMEPOST"),ZObjFromInt64(CURLOPT_MIMEPOST));
    Module_addMember(d,("OPT_VERBOSE"),ZObjFromInt64(CURLOPT_VERBOSE));
    Module_addMember(d,"OPT_XFERINFOFUNCTION",ZObjFromInt64(CURLOPT_XFERINFOFUNCTION));
    Module_addMember(d,"OPT_NOPROGRESS",ZObjFromInt64(CURLOPT_NOPROGRESS));

    Module_addMember(d,("CURLE_OK"),ZObjFromInt64(CURLE_OK));
    Module_addMember(d,("WriteMemory"),ZObjFromInt(0));
    Module_addMember(d,("INFO_CONTENT_TYPE"),ZObjFromInt64((long long int)CURLINFO_CONTENT_TYPE));
    Module_addMember(d,("INFO_HTTP_CODE"),ZObjFromInt64((long long int)CURLINFO_HTTP_CODE));
    Module_addMember(d,("VERSION_NUM"),ZObjFromInt(LIBCURL_VERSION_NUM));
    Module_addMember(d,("VERSION_MAJOR"),ZObjFromInt(LIBCURL_VERSION_MAJOR));
    Module_addMember(d,("VERSION_MINOR"),ZObjFromInt(LIBCURL_VERSION_MINOR));
    
    vm_markImportant(curlklass);
    vm_markImportant(mimeklass);
    vm_markImportant(mimepartklass);
    return ZObjFromModule(d);
}
ZObject curlklass__del__(ZObject* args,int n)//
{
    if(n != 1 || args[0].type != Z_OBJ)
      return nil;
    KlassObject* d = (KlassObject*)args[0].ptr;
    if(d->klass != curlklass)
      return nil;
    ZObject ptr = KlassObj_getMember(d,".handle");
    if(ptr.type == 'n')
      return nil;
    CurlObject* obj = (CurlObject*)ptr.ptr;
    if(obj->postfields!=NULL)
      delete[] obj->postfields;
    curl_easy_cleanup(obj->handle);
    delete obj;
    KlassObj_setMember(d,".handle",nil);
    return nil;
}
ZObject STRERROR(ZObject* args,int n)
{
  if(n!=1)
  {
    return Z_Err(ArgumentError,"1 argument needed!");
    
  }
  if(args[0].type!='l')
  {
    return Z_Err(TypeError,"Integer argument needed!");
    
  }
  const char* s = curl_easy_strerror((CURLcode)args[0].l);
  return ZObjFromStr(s);
}
ZObject curlklass__construct__(ZObject* args,int n)
{
  if(n!=1)
    return Z_Err(ArgumentError,"1 arguments needed!");
    
  KlassObject* curobj = (KlassObject*)args[0].ptr;
  CURL* curl = curl_easy_init();
  if(!curl)
  {
    return Z_Err(Error,"failed");
    
  }
  //Return an object
  
  CurlObject* obj = new CurlObject;
  obj->handle = curl;
  obj->postfields = NULL;
  KlassObj_setMember(curobj,".handle",ZObjFromPtr(obj));
  return nil;
 
}
ZObject MIME__del__(ZObject* args,int n)
{
    if(n!=1)
        return Z_Err(ArgumentError,"1 argument needed!");
    KlassObject* k = (KlassObject*)args[0].ptr;
    curl_mime* obj = (curl_mime*)AS_PTR(KlassObj_getMember(k,".handle"));
    curl_mime_free(obj);
    return nil;
}
ZObject mime__construct__(ZObject* args,int n)
{
  if(n!=2)
  {
    return Z_Err(ArgumentError,"2 arguments needed!");
    
  }
  if(args[1].type!='o' || ((KlassObject*)args[1].ptr)->klass!=curlklass)
  {
    return Z_Err(TypeError,"Curl Object needed!");
    
  }
  KlassObject* d = (KlassObject*)args[0].ptr;
  KlassObject* cobj = (KlassObject*)args[1].ptr;
  CurlObject*  curlobj = (CurlObject*)AS_PTR(KlassObj_getMember(cobj,".handle"));
  curl_mime* mime = curl_mime_init(curlobj->handle);
  
  //Add the mime handle to mime object
  KlassObj_setMember(d,(".handle"),ZObjFromPtr((void*)mime));

  return nil;
}
ZObject MIME_NAME(ZObject* args,int n)
{
    if(n!=2)
        return Z_Err(ArgumentError,"2 argument needed!");
    if(args[1].type!='s')
        return Z_Err(TypeError,"Argument 2 must be string!");
    if(args[0].type!='o' || ((KlassObject*)args[0].ptr)->klass!=mimepartklass)
      return Z_Err(TypeError,"Argument 1 must be a MimePart object");
  KlassObject* d = (KlassObject*)args[0].ptr;
  MimePart* obj = (MimePart*)AS_PTR(KlassObj_getMember(d,".handle"));
  ZStr* name = AS_STR(args[1]);
  curl_mime_name(obj->mimepart,name->val);
  return nil;
}
ZObject MIME_FILENAME(ZObject* args,int n)
{
    if(n!=2)
        return Z_Err(ArgumentError,"2 argument needed!");
    if(args[1].type!='s')
        return Z_Err(TypeError,"Argument 2 must be string!");
    if(args[0].type!='o' || ((KlassObject*)args[0].ptr)->klass!=mimepartklass)
      return Z_Err(TypeError,"Argument 1 must be a MimePart object");
  KlassObject* d = (KlassObject*)args[0].ptr;
  MimePart* obj = (MimePart*)AS_PTR(KlassObj_getMember(d,".handle"));
  ZStr* name = AS_STR(args[1]);
  curl_mime_filename(obj->mimepart,name->val);
  return nil;
}
ZObject MIME_CONTENTTYPE(ZObject* args,int n)
{
    if(n!=2)
        return Z_Err(ArgumentError,"2 argument needed!");
    if(args[1].type!='s')
        return Z_Err(TypeError,"Argument 2 must be string!");
    if(args[0].type!='o' || ((KlassObject*)args[0].ptr)->klass!=mimepartklass)
      return Z_Err(TypeError,"Argument 1 must be a MimePart object");
  KlassObject* d = (KlassObject*)args[0].ptr;
  MimePart* obj = (MimePart*)AS_PTR(KlassObj_getMember(d,".handle"));
  ZStr* name = AS_STR(args[1]);
  curl_mime_type(obj->mimepart,name->val);
  return nil;
}
ZObject MIME_DATA(ZObject* args,int n)
{
    if(n!=2)
        return Z_Err(ArgumentError,"1 argument needed!");
    if(args[1].type!='c')
      return Z_Err(TypeError,"Argument 2 must be a Byte array!");
    if(args[0].type!='o' || ((KlassObject*)args[0].ptr)->klass!=mimepartklass)
      return Z_Err(TypeError,"Argument 1 must be a MimePart object");
    KlassObject* d = (KlassObject*)args[0].ptr;
    MimePart* obj = (MimePart*)AS_PTR(KlassObj_getMember(d,".handle"));
    auto l = AS_BYTEARRAY(args[1]);
    curl_mime_data(obj->mimepart,(const char*)l->arr,l->size);
    return nil;
}

ZObject addpart(ZObject* args,int n)
{
    if(n!=1)
        return Z_Err(ArgumentError,"1 arguments needed!");
    if(args[0].type!='o' || ((KlassObject*)args[0].ptr)->klass!=mimeklass)
      return Z_Err(TypeError,"Argument 1 must be a Mime object");
    KlassObject* d = (KlassObject*)args[0].ptr;
    curl_mime* obj = (curl_mime*)AS_PTR(KlassObj_getMember(d,".handle"));
    curl_mimepart* mimepart = curl_mime_addpart(obj);
    MimePart* mp = new MimePart;
    mp->mimepart = mimepart;
    //add methods to object
    KlassObject* part = vm_allocKlassObject(mimepartklass);
    KlassObj_setMember(part,"data",ZObjFromMethod("mimepart.data",&MIME_DATA,mimepartklass));
    KlassObj_setMember(part,("name"),ZObjFromMethod("mimepart.name",&MIME_NAME,mimepartklass));
    KlassObj_setMember(part,("filename"),ZObjFromMethod("mimepart.filename",&MIME_FILENAME,mimepartklass));
    KlassObj_setMember(part,("type"),ZObjFromMethod("mimepart.type",&MIME_CONTENTTYPE,mimepartklass));
    KlassObj_setMember(part,("__del__"),ZObjFromMethod("mimepart.__del__",&MIMEPART__del__,mimepartklass));
    KlassObj_setMember(part,(".handle"),ZObjFromPtr((void*)mp));

    return ZObjFromKlassObj(part);
}
ZObject MIMEPART__del__(ZObject* args,int n)
{
    if(n!=1)
        return Z_Err(ArgumentError,"1 argument needed!");
    KlassObject* k = (KlassObject*)args[0].ptr;
    MimePart* obj = (MimePart*)KlassObj_getMember(k,".handle").ptr;
    delete obj;
    return nil;
}
ZObject setopt(ZObject* args,int n)
{
    if(n!=3)
    {
        return Z_Err(ArgumentError,"3 arguments needed!");
        
    }
    if(args[1].type!=Z_INT64)
    {
        return Z_Err(TypeError,"Argument 2 must an Integer!");
        
    }
    if(args[0].type!='o' || ((KlassObject*)args[0].ptr)->klass!=curlklass)
    {
      return Z_Err(TypeError,"Argument 1 must be a Curl object");
      
    }
    long long int opt = args[1].l;
    if(opt==CURLOPT_URL)
    {
      if(args[2].type!=Z_STR)
      {
        return Z_Err(TypeError,"URL option requires a string value");
         
      }
      KlassObject* d = (KlassObject*)args[0].ptr;
      CurlObject* obj = (CurlObject*)KlassObj_getMember(d,".handle").ptr;
      obj->url = AS_STR(args[2])->val;
      CURLcode res = curl_easy_setopt(obj->handle,(CURLoption)opt,obj->url.c_str());
      if( res!= CURLE_OK)
        return Z_Err(Error,curl_easy_strerror(res));
      
    }
    else if(opt==CURLOPT_FOLLOWLOCATION)
    {
      if(args[2].type!=Z_INT)
      {
        return Z_Err(TypeError,"FOLLOWLOCATION option requires an integer value");
         
      }
      KlassObject* d = (KlassObject*)args[0].ptr;
      CurlObject* obj = (CurlObject*)KlassObj_getMember(d,".handle").ptr;
      CURLcode res = curl_easy_setopt(obj->handle,(CURLoption)opt,args[2].i);
      if( res!= CURLE_OK)
      {
        return Z_Err(Error,curl_easy_strerror(res));
        
      }
    }
    else if(opt==CURLOPT_VERBOSE)
    {
      if(args[2].type!=Z_INT)
      {
        return Z_Err(TypeError,"VERBOSE option requires an integer value");
         
      }
      KlassObject* d = (KlassObject*)args[0].ptr;
      CurlObject* obj = (CurlObject*)KlassObj_getMember(d,".handle").ptr;
      CURLcode res = curl_easy_setopt(obj->handle,(CURLoption)opt,args[2].i);
      if( res!= CURLE_OK)
      {
        return Z_Err(Error,curl_easy_strerror(res));
        
      }
    }
    else if(opt==CURLOPT_PORT)
    {
      if(args[2].type!=Z_INT)
      {
        return Z_Err(TypeError,"PORT option requires an integer value");
         
      }
      KlassObject* d = (KlassObject*)args[0].ptr;
      CurlObject* obj = (CurlObject*)KlassObj_getMember(d,".handle").ptr;
      CURLcode res = curl_easy_setopt(obj->handle,(CURLoption)opt,args[2].i);
      if( res!= CURLE_OK)
      {
        return Z_Err(Error,curl_easy_strerror(res));
        
      }
    }
    else if(opt==CURLOPT_MIMEPOST)
    {
      if(args[2].type!=Z_OBJ)
      {
        return Z_Err(TypeError,"MIMEPOST option requires a MimeObject");
         
      }
      KlassObject* d = (KlassObject*)args[0].ptr;
      CurlObject* obj = (CurlObject*)KlassObj_getMember(d,".handle").ptr;
      KlassObject* e = (KlassObject*)args[2].ptr;
      curl_mime* m = (curl_mime*)KlassObj_getMember(e,".handle").ptr;
      CURLcode res = curl_easy_setopt(obj->handle,(CURLoption)opt,m);
      if( res!= CURLE_OK)
      {
        return Z_Err(Error,curl_easy_strerror(res));
        
      }
    }
    else if(opt==CURLOPT_WRITEFUNCTION)
    {
      if(args[2].type == Z_FUNC || args[2].type == Z_NATIVE_FUNC)
      {
        //set callback
        KlassObject* d = (KlassObject*)args[0].ptr;
        KlassObj_setMember(d,"wmcallback",args[2]);
        wmcallback = args[2];
        CurlObject* obj = (CurlObject*)KlassObj_getMember(d,".handle").ptr;
        CURLcode res = curl_easy_setopt(obj->handle,(CURLoption)opt,&WMCallbackHandler);
        if( res!= CURLE_OK)
          return Z_Err(Error,curl_easy_strerror(res));
        return nil;
      }
      if(args[2].type!=Z_INT )
        return Z_Err(TypeError,"WRITEFUNCTION option requires an integer value");
      if(args[2].i!=0)
        return quickErr(TypeError,"Invalid option value "+to_string(args[2].i));
      KlassObject* d = (KlassObject*)args[0].ptr;
      auto btArr = vm_allocByteArray();
      ZObject p1;
      p1.type = 'c';
      p1.ptr = (void*)btArr;
      KlassObj_setMember(d,"data",p1);
      CurlObject* obj = (CurlObject*)KlassObj_getMember(d,".handle").ptr;
      CURLcode res = curl_easy_setopt(obj->handle,(CURLoption)opt,&WriteMemoryCallback);
      if( res!= CURLE_OK)
      {
        return Z_Err(Error,curl_easy_strerror(res));
        
      }
      curl_easy_setopt(obj->handle,CURLOPT_WRITEDATA,btArr);
    }
    else if(opt==CURLOPT_USERAGENT)
    {
      if(args[2].type!=Z_STR)
      {
        return Z_Err(TypeError,"USERAGENT option requires a string value");
         
      }
      KlassObject* d = (KlassObject*)args[0].ptr;
      CurlObject* obj = (CurlObject*)KlassObj_getMember(d,".handle").ptr;
      obj->useragent = AS_STR(args[2])->val;
      CURLcode res = curl_easy_setopt(obj->handle,(CURLoption)opt,obj->useragent.c_str());
      if( res!= CURLE_OK)
      {
        return Z_Err(Error,curl_easy_strerror(res));
        
      }
    }
    else if( opt == CURLOPT_NOPROGRESS)
    {
      KlassObject* d = (KlassObject*)args[0].ptr;
      CurlObject* obj = (CurlObject*)KlassObj_getMember(d,".handle").ptr;
      CURLcode res = curl_easy_setopt(obj->handle,CURLOPT_NOPROGRESS,args[2].i);
      if(res != CURLE_OK)
	      return Z_Err(Error,curl_easy_strerror(res));
    }
    else if(opt == CURLOPT_XFERINFOFUNCTION)
    {
      if(args[2].type!=Z_FUNC)
        return Z_Err(TypeError,"XFERINFOFUNCTION option must be a callback function!");
      KlassObject* d = (KlassObject*)args[0].ptr;
      KlassObj_setMember(d,".xfercallback",args[2]);
      xfercallback = args[2];
      CurlObject* obj = (CurlObject*)KlassObj_getMember(d,".handle").ptr;
      CURLcode res = curl_easy_setopt(obj->handle,CURLOPT_XFERINFOFUNCTION,xferfun);
      if(res != CURLE_OK)
        return Z_Err(Error,curl_easy_strerror(res));
    }
    else if(opt==CURLOPT_POSTFIELDS)
    {
      if(args[2].type!=Z_BYTEARR)
        return Z_Err(TypeError,"POSTFIELDS option requires a byte array!");
      vector<uint8_t>& l = *(vector<uint8_t>*)args[2].ptr;
      if(l.size()==0)
        return Z_Err(ValueError,"Empty bytearray");
      char* postfields = new char[l.size()];
      memcpy(postfields,&l[0],l.size());
      KlassObject* d = (KlassObject*)args[0].ptr;
      CurlObject* obj = (CurlObject*)KlassObj_getMember(d,".handle").ptr;
      obj->postfields = postfields;
      CURLcode res = curl_easy_setopt(obj->handle,(CURLoption)opt,postfields);
      if( res!= CURLE_OK)
        return Z_Err(Error,curl_easy_strerror(res));
      res = curl_easy_setopt(obj->handle,CURLOPT_POSTFIELDSIZE,l.size());
      
    }
    else
    {
      return Z_Err(ValueError,"Unknown option used");
      
    }
    return nil;
}
ZObject perform(ZObject* args,int n)
{
    if(n!=1)
    {
        return Z_Err(ArgumentError,"1 argument needed!");
        
    }
    KlassObject* d = (KlassObject*)args[0].ptr;
    CurlObject* obj = (CurlObject*)KlassObj_getMember(d,".handle").ptr;
    CURLcode res;
    res = curl_easy_perform(obj->handle);
    return ZObjFromInt64((long long int)res);
}
ZObject getinfo(ZObject* args,int n)
{
    if(n!=2)
    {
        return Z_Err(ArgumentError,"2 arguments needed!");
        
    }
    if(args[0].type!='o' || ((KlassObject*)args[0].ptr)->klass!=curlklass)
    {
      return Z_Err(TypeError,"Argument 1 must be a Curl object");
      
    }
    if(args[1].type!='l')
    {
      return Z_Err(TypeError,"Argument 2 must an Integer!");
      
    }
    KlassObject* d = (KlassObject*)args[0].ptr;
    CurlObject* obj = (CurlObject*)KlassObj_getMember(d,".handle").ptr;
    CURL* handle = obj->handle;
    if(args[1].l==CURLINFO_CONTENT_TYPE)
    {
      char* out;
      CURLcode res = curl_easy_getinfo(handle,(CURLINFO)args[1].l,&out);
      if(!out || res!=CURLE_OK)
      {
        return nil;
      }
      return ZObjFromStr(out);

    }
    else if(args[1].l==CURLINFO_HTTP_CODE)
    {
      long out;
      CURLcode res = curl_easy_getinfo(handle,(CURLINFO)args[1].l,&out);
      if(res!=CURLE_OK)
      {
        return Z_Err(Error,curl_easy_strerror(res));
        
      }
      return ZObjFromInt64((long long int)out);
    }
    return nil;
}
/*curl__del__ does the cleanup as well
ZObject cleanup(ZObject* args,int n)
{
    if(n!=1)
    {
        return Z_Err(ArgumentError,"1 arguments needed!");
        
    }
    if(args[0].type!='o' || ((KlassObject*)args[0].ptr)->klass!=curlklass)
    {
      return Z_Err(TypeError,"Argument 1 must be a Curl object");
      
    }
    KlassObject* d = (KlassObject*)args[0].ptr;
    string tmp = ".handle";
    CurlObject* obj = (CurlObject*)(d->members)[".handle"].ptr;
    curl_easy_cleanup(obj->handle);
    return nil;
}*/


ZObject ESCAPE(ZObject* args,int n)
{
  if(n!=2)
  {
    return Z_Err(ArgumentError,"2 Arguments required.");
    
  }
  if(args[0].type != Z_OBJ || ((KlassObject*)args[0].ptr) -> klass !=curlklass)
  {
    return Z_Err(TypeError,"Argument 1 must be a Curl Object");
    
  }
  if(args[1].type!=Z_STR)
  {
    return Z_Err(TypeError,"Argument 1 must be a string");
    
  }
  KlassObject* ki = (KlassObject*)args[0].ptr;
  CurlObject* k = (CurlObject*)KlassObj_getMember(ki,".handle").ptr;
  ZStr* str = AS_STR(args[1]);
  char* output = curl_easy_escape(k->handle,str->val,str->len);
  ZObject ret = ZObjFromStr(output);
  curl_free(output);
  return ret;
}
ZObject UNESCAPE(ZObject* args,int n)
{
  if(n!=2)
  {
    return Z_Err(ArgumentError,"2 Arguments required.");
    
  }
  if(args[0].type != Z_OBJ || ((KlassObject*)args[0].ptr) -> klass !=curlklass)
  {
    return Z_Err(TypeError,"Argument 1 must be a Curl Object");
    
  }
  if(args[1].type!=Z_STR)
  {
    return Z_Err(TypeError,"Argument 1 must be a string");
    
  }
  KlassObject* ki = (KlassObject*)args[0].ptr;
  CurlObject* k = (CurlObject*)AS_PTR(KlassObj_getMember(ki,".handle"));
  ZStr* str = AS_STR(args[1]);
  int decodelen = 0;
  char* output = curl_easy_unescape(k->handle,str->val,str->len,&decodelen);
  ZObject ret = ZObjFromStr(output);
  curl_free(output);
  return ret;
}
extern "C" void unload()
{
  curl_global_cleanup();
  vm_unmarkImportant(curlklass);
  vm_unmarkImportant(mimeklass);
  vm_unmarkImportant(mimepartklass);
}
