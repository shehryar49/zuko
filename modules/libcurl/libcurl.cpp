
/*
Libcurl Binding for Zuko(easy interface only)
The original code of libcurl is not modified in anyway. This is just a wrapper around libcurl
and requires libcurl libraries to be linked when compiling.
Written by Shahryar Ahmad
*/
#include "zapi.h"
#include "zobject.h"
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
  zbytearr* mem = (zbytearr*)userp;
  size_t prevsize = mem->size;
  zbytearr_resize(mem,prevsize + realsize);
  memcpy(mem->arr+prevsize, contents, realsize);
  return realsize;
}
//WriteMemory function which calls zuko callback and passes the bytes
zobject wmcallback;
zobject xfercallback;
size_t WMCallbackHandler(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  //send newly received memory to callback function
  auto btArr = vm_alloc_zbytearr();
  zbytearr_resize(btArr,realsize);
  memcpy(btArr->arr, contents, realsize);
  zobject rr;
  zobject p1;
  p1.type = Z_BYTEARR;
  p1.ptr = (void*)btArr;
  vm_call_object(&wmcallback,&p1,1,&rr);
  return realsize;
}
//xfer function
int xferfun(void* clientp,curl_off_t dltotal,curl_off_t dlnow,curl_off_t ultotal,curl_off_t ulnow)
{
  zobject args[4];
  zobject r;
  r.type = Z_INT64;
  r.l = dltotal;
  args[0] = r;
  r.l = dlnow;
  args[1] = r;
  r.l = ultotal;
  args[2] = r;
  r.l = ulnow;
  args[3] = r;
  vm_call_object(&xfercallback,args,4,&r);
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
zobject nil;
zclass* curlklass;
zclass* mimeklass;
zclass* mimepartklass;
zobject callback;//write function callback
zobject quickErr(zclass* k,string s)
{
  return z_err(k,s.c_str());
}
zobject init()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    nil.type = Z_NIL;
    zmodule* d = vm_alloc_zmodule();
    d->name = "libcurl";
    //
    curlklass = vm_alloc_zclass();
    curlklass->name = "curl";

    zclass_add_method(curlklass,"__construct__",&curlklass__construct__);
    zclass_add_method(curlklass,"perform",&perform);
    zclass_add_method(curlklass,"setopt",&setopt);
    zclass_add_method(curlklass,"getinfo",&getinfo);
    zclass_add_method(curlklass,"__del__",&curlklass__del__);
    zclass_add_method(curlklass,"escape",&ESCAPE);
    zclass_add_method(curlklass,"unescape",&UNESCAPE);
    
    //
    mimepartklass = vm_alloc_zclass();
    mimepartklass->name = "mimepart";
    //
    mimeklass = vm_alloc_zclass();
    mimeklass->name = "mime";
    //add methods to object
    zclass_add_method(mimeklass,"__construct__",&mime__construct__);
    zclass_add_method(mimeklass,"addpart",&addpart);
    zclass_add_method(mimeklass,"__del__",&MIME__del__);
    
    
    
    zmodule_add_class(d,"curl",curlklass);
    zmodule_add_class(d,"mime",mimeklass);
    zmodule_add_fun(d,"strerror",&STRERROR);
    zmodule_add_member(d,("OPT_URL"),zobj_from_int64(CURLOPT_URL));
    zmodule_add_member(d,("OPT_PORT"),zobj_from_int64(CURLOPT_PORT));
    zmodule_add_member(d,("OPT_POSTFIELDS"),zobj_from_int64(CURLOPT_POSTFIELDS));
    zmodule_add_member(d,("OPT_USERAGENT"),zobj_from_int64(CURLOPT_USERAGENT));
    zmodule_add_member(d,("OPT_FOLLOWLOCATION"),zobj_from_int64(CURLOPT_FOLLOWLOCATION));
    zmodule_add_member(d,("OPT_WRITEFUNCTION"),zobj_from_int64(CURLOPT_WRITEFUNCTION));
    zmodule_add_member(d,("OPT_MIMEPOST"),zobj_from_int64(CURLOPT_MIMEPOST));
    zmodule_add_member(d,("OPT_VERBOSE"),zobj_from_int64(CURLOPT_VERBOSE));
    zmodule_add_member(d,"OPT_XFERINFOFUNCTION",zobj_from_int64(CURLOPT_XFERINFOFUNCTION));
    zmodule_add_member(d,"OPT_NOPROGRESS",zobj_from_int64(CURLOPT_NOPROGRESS));

    zmodule_add_member(d,("CURLE_OK"),zobj_from_int64(CURLE_OK));
    zmodule_add_member(d,("WriteMemory"),zobj_from_int(0));
    zmodule_add_member(d,("INFO_CONTENT_TYPE"),zobj_from_int64((long long int)CURLINFO_CONTENT_TYPE));
    zmodule_add_member(d,("INFO_HTTP_CODE"),zobj_from_int64((long long int)CURLINFO_HTTP_CODE));
    zmodule_add_member(d,("VERSION_NUM"),zobj_from_int(LIBCURL_VERSION_NUM));
    zmodule_add_member(d,("VERSION_MAJOR"),zobj_from_int(LIBCURL_VERSION_MAJOR));
    zmodule_add_member(d,("VERSION_MINOR"),zobj_from_int(LIBCURL_VERSION_MINOR));
    
    vm_mark_important(curlklass);
    vm_mark_important(mimeklass);
    vm_mark_important(mimepartklass);
    return zobj_from_module(d);
}
zobject curlklass__del__(zobject* args,int n)//
{
    if(n != 1 || args[0].type != Z_OBJ)
      return nil;
    zclass_object* d = (zclass_object*)args[0].ptr;
    if(d->_klass != curlklass)
      return nil;
    zobject ptr = zclassobj_get(d,".handle");
    if(ptr.type == 'n')
      return nil;
    CurlObject* obj = (CurlObject*)ptr.ptr;
    if(obj->postfields!=NULL)
      delete[] obj->postfields;
    curl_easy_cleanup(obj->handle);
    delete obj;
    zclassobj_set(d,".handle",nil);
    return nil;
}
zobject STRERROR(zobject* args,int n)
{
  if(n!=1)
  {
    return z_err(ArgumentError,"1 argument needed!");
    
  }
  if(args[0].type!='l')
  {
    return z_err(TypeError,"Integer argument needed!");
    
  }
  const char* s = curl_easy_strerror((CURLcode)args[0].l);
  return zobj_from_str(s);
}
zobject curlklass__construct__(zobject* args,int n)
{
  if(n!=1)
    return z_err(ArgumentError,"1 arguments needed!");
    
  zclass_object* curobj = (zclass_object*)args[0].ptr;
  CURL* curl = curl_easy_init();
  if(!curl)
  {
    return z_err(Error,"failed");
    
  }
  //Return an object
  
  CurlObject* obj = new CurlObject;
  obj->handle = curl;
  obj->postfields = NULL;
  zclassobj_set(curobj,".handle",zobj_from_ptr(obj));
  return nil;
 
}
zobject MIME__del__(zobject* args,int n)
{
    if(n!=1)
        return z_err(ArgumentError,"1 argument needed!");
    zclass_object* k = (zclass_object*)args[0].ptr;
    curl_mime* obj = (curl_mime*)AS_PTR(zclassobj_get(k,".handle"));
    curl_mime_free(obj);
    return nil;
}
zobject mime__construct__(zobject* args,int n)
{
  if(n!=2)
  {
    return z_err(ArgumentError,"2 arguments needed!");
    
  }
  if(args[1].type!='o' || ((zclass_object*)args[1].ptr)->_klass!=curlklass)
  {
    return z_err(TypeError,"Curl Object needed!");
    
  }
  zclass_object* d = (zclass_object*)args[0].ptr;
  zclass_object* cobj = (zclass_object*)args[1].ptr;
  CurlObject*  curlobj = (CurlObject*)AS_PTR(zclassobj_get(cobj,".handle"));
  curl_mime* mime = curl_mime_init(curlobj->handle);
  
  //Add the mime handle to mime object
  zclassobj_set(d,(".handle"),zobj_from_ptr((void*)mime));

  return nil;
}
zobject MIME_NAME(zobject* args,int n)
{
    if(n!=2)
        return z_err(ArgumentError,"2 argument needed!");
    if(args[1].type!='s')
        return z_err(TypeError,"Argument 2 must be string!");
    if(args[0].type!='o' || ((zclass_object*)args[0].ptr)->_klass!=mimepartklass)
      return z_err(TypeError,"Argument 1 must be a MimePart object");
  zclass_object* d = (zclass_object*)args[0].ptr;
  MimePart* obj = (MimePart*)AS_PTR(zclassobj_get(d,".handle"));
  zstr* name = AS_STR(args[1]);
  curl_mime_name(obj->mimepart,name->val);
  return nil;
}
zobject MIME_FILENAME(zobject* args,int n)
{
    if(n!=2)
        return z_err(ArgumentError,"2 argument needed!");
    if(args[1].type!='s')
        return z_err(TypeError,"Argument 2 must be string!");
    if(args[0].type!='o' || ((zclass_object*)args[0].ptr)->_klass!=mimepartklass)
      return z_err(TypeError,"Argument 1 must be a MimePart object");
  zclass_object* d = (zclass_object*)args[0].ptr;
  MimePart* obj = (MimePart*)AS_PTR(zclassobj_get(d,".handle"));
  zstr* name = AS_STR(args[1]);
  curl_mime_filename(obj->mimepart,name->val);
  return nil;
}
zobject MIME_CONTENTTYPE(zobject* args,int n)
{
    if(n!=2)
        return z_err(ArgumentError,"2 argument needed!");
    if(args[1].type!='s')
        return z_err(TypeError,"Argument 2 must be string!");
    if(args[0].type!='o' || ((zclass_object*)args[0].ptr)->_klass!=mimepartklass)
      return z_err(TypeError,"Argument 1 must be a MimePart object");
  zclass_object* d = (zclass_object*)args[0].ptr;
  MimePart* obj = (MimePart*)AS_PTR(zclassobj_get(d,".handle"));
  zstr* name = AS_STR(args[1]);
  curl_mime_type(obj->mimepart,name->val);
  return nil;
}
zobject MIME_DATA(zobject* args,int n)
{
    if(n!=2)
        return z_err(ArgumentError,"1 argument needed!");
    if(args[1].type!='c')
      return z_err(TypeError,"Argument 2 must be a Byte array!");
    if(args[0].type!='o' || ((zclass_object*)args[0].ptr)->_klass!=mimepartklass)
      return z_err(TypeError,"Argument 1 must be a MimePart object");
    zclass_object* d = (zclass_object*)args[0].ptr;
    MimePart* obj = (MimePart*)AS_PTR(zclassobj_get(d,".handle"));
    auto l = AS_BYTEARRAY(args[1]);
    curl_mime_data(obj->mimepart,(const char*)l->arr,l->size);
    return nil;
}

zobject addpart(zobject* args,int n)
{
    if(n!=1)
        return z_err(ArgumentError,"1 arguments needed!");
    if(args[0].type!='o' || ((zclass_object*)args[0].ptr)->_klass!=mimeklass)
      return z_err(TypeError,"Argument 1 must be a Mime object");
    zclass_object* d = (zclass_object*)args[0].ptr;
    curl_mime* obj = (curl_mime*)AS_PTR(zclassobj_get(d,".handle"));
    curl_mimepart* mimepart = curl_mime_addpart(obj);
    MimePart* mp = new MimePart;
    mp->mimepart = mimepart;
    //add methods to object
    zclass_object* part = vm_alloc_zclassobj(mimepartklass);
    zclassobj_set(part,"data",zobj_from_method("mimepart.data",&MIME_DATA,mimepartklass));
    zclassobj_set(part,("name"),zobj_from_method("mimepart.name",&MIME_NAME,mimepartklass));
    zclassobj_set(part,("filename"),zobj_from_method("mimepart.filename",&MIME_FILENAME,mimepartklass));
    zclassobj_set(part,("type"),zobj_from_method("mimepart.type",&MIME_CONTENTTYPE,mimepartklass));
    zclassobj_set(part,("__del__"),zobj_from_method("mimepart.__del__",&MIMEPART__del__,mimepartklass));
    zclassobj_set(part,(".handle"),zobj_from_ptr((void*)mp));

    return zobj_from_classobj(part);
}
zobject MIMEPART__del__(zobject* args,int n)
{
    if(n!=1)
        return z_err(ArgumentError,"1 argument needed!");
    zclass_object* k = (zclass_object*)args[0].ptr;
    MimePart* obj = (MimePart*)zclassobj_get(k,".handle").ptr;
    delete obj;
    return nil;
}
zobject setopt(zobject* args,int n)
{
    if(n!=3)
    {
        return z_err(ArgumentError,"3 arguments needed!");
        
    }
    if(args[1].type!=Z_INT64)
    {
        return z_err(TypeError,"Argument 2 must an Integer!");
        
    }
    if(args[0].type!='o' || ((zclass_object*)args[0].ptr)->_klass!=curlklass)
    {
      return z_err(TypeError,"Argument 1 must be a Curl object");
      
    }
    long long int opt = args[1].l;
    if(opt==CURLOPT_URL)
    {
      if(args[2].type!=Z_STR)
      {
        return z_err(TypeError,"URL option requires a string value");
         
      }
      zclass_object* d = (zclass_object*)args[0].ptr;
      CurlObject* obj = (CurlObject*)zclassobj_get(d,".handle").ptr;
      obj->url = AS_STR(args[2])->val;
      CURLcode res = curl_easy_setopt(obj->handle,(CURLoption)opt,obj->url.c_str());
      if( res!= CURLE_OK)
        return z_err(Error,curl_easy_strerror(res));
      
    }
    else if(opt==CURLOPT_FOLLOWLOCATION)
    {
      if(args[2].type!=Z_INT)
      {
        return z_err(TypeError,"FOLLOWLOCATION option requires an integer value");
         
      }
      zclass_object* d = (zclass_object*)args[0].ptr;
      CurlObject* obj = (CurlObject*)zclassobj_get(d,".handle").ptr;
      CURLcode res = curl_easy_setopt(obj->handle,(CURLoption)opt,args[2].i);
      if( res!= CURLE_OK)
      {
        return z_err(Error,curl_easy_strerror(res));
        
      }
    }
    else if(opt==CURLOPT_VERBOSE)
    {
      if(args[2].type!=Z_INT)
      {
        return z_err(TypeError,"VERBOSE option requires an integer value");
         
      }
      zclass_object* d = (zclass_object*)args[0].ptr;
      CurlObject* obj = (CurlObject*)zclassobj_get(d,".handle").ptr;
      CURLcode res = curl_easy_setopt(obj->handle,(CURLoption)opt,args[2].i);
      if( res!= CURLE_OK)
      {
        return z_err(Error,curl_easy_strerror(res));
        
      }
    }
    else if(opt==CURLOPT_PORT)
    {
      if(args[2].type!=Z_INT)
      {
        return z_err(TypeError,"PORT option requires an integer value");
         
      }
      zclass_object* d = (zclass_object*)args[0].ptr;
      CurlObject* obj = (CurlObject*)zclassobj_get(d,".handle").ptr;
      CURLcode res = curl_easy_setopt(obj->handle,(CURLoption)opt,args[2].i);
      if( res!= CURLE_OK)
      {
        return z_err(Error,curl_easy_strerror(res));
        
      }
    }
    else if(opt==CURLOPT_MIMEPOST)
    {
      if(args[2].type!=Z_OBJ)
      {
        return z_err(TypeError,"MIMEPOST option requires a MimeObject");
         
      }
      zclass_object* d = (zclass_object*)args[0].ptr;
      CurlObject* obj = (CurlObject*)zclassobj_get(d,".handle").ptr;
      zclass_object* e = (zclass_object*)args[2].ptr;
      curl_mime* m = (curl_mime*)zclassobj_get(e,".handle").ptr;
      CURLcode res = curl_easy_setopt(obj->handle,(CURLoption)opt,m);
      if( res!= CURLE_OK)
      {
        return z_err(Error,curl_easy_strerror(res));
        
      }
    }
    else if(opt==CURLOPT_WRITEFUNCTION)
    {
      if(args[2].type == Z_FUNC || args[2].type == Z_NATIVE_FUNC)
      {
        //set callback
        zclass_object* d = (zclass_object*)args[0].ptr;
        zclassobj_set(d,"wmcallback",args[2]);
        wmcallback = args[2];
        CurlObject* obj = (CurlObject*)zclassobj_get(d,".handle").ptr;
        CURLcode res = curl_easy_setopt(obj->handle,(CURLoption)opt,&WMCallbackHandler);
        if( res!= CURLE_OK)
          return z_err(Error,curl_easy_strerror(res));
        return nil;
      }
      if(args[2].type!=Z_INT )
        return z_err(TypeError,"WRITEFUNCTION option requires an integer value");
      if(args[2].i!=0)
        return quickErr(TypeError,"Invalid option value "+to_string(args[2].i));
      zclass_object* d = (zclass_object*)args[0].ptr;
      auto btArr = vm_alloc_zbytearr();
      zobject p1;
      p1.type = 'c';
      p1.ptr = (void*)btArr;
      zclassobj_set(d,"data",p1);
      CurlObject* obj = (CurlObject*)zclassobj_get(d,".handle").ptr;
      CURLcode res = curl_easy_setopt(obj->handle,(CURLoption)opt,&WriteMemoryCallback);
      if( res!= CURLE_OK)
      {
        return z_err(Error,curl_easy_strerror(res));
        
      }
      curl_easy_setopt(obj->handle,CURLOPT_WRITEDATA,btArr);
    }
    else if(opt==CURLOPT_USERAGENT)
    {
      if(args[2].type!=Z_STR)
      {
        return z_err(TypeError,"USERAGENT option requires a string value");
         
      }
      zclass_object* d = (zclass_object*)args[0].ptr;
      CurlObject* obj = (CurlObject*)zclassobj_get(d,".handle").ptr;
      obj->useragent = AS_STR(args[2])->val;
      CURLcode res = curl_easy_setopt(obj->handle,(CURLoption)opt,obj->useragent.c_str());
      if( res!= CURLE_OK)
      {
        return z_err(Error,curl_easy_strerror(res));
        
      }
    }
    else if( opt == CURLOPT_NOPROGRESS)
    {
      zclass_object* d = (zclass_object*)args[0].ptr;
      CurlObject* obj = (CurlObject*)zclassobj_get(d,".handle").ptr;
      CURLcode res = curl_easy_setopt(obj->handle,CURLOPT_NOPROGRESS,args[2].i);
      if(res != CURLE_OK)
	      return z_err(Error,curl_easy_strerror(res));
    }
    else if(opt == CURLOPT_XFERINFOFUNCTION)
    {
      if(args[2].type!=Z_FUNC)
        return z_err(TypeError,"XFERINFOFUNCTION option must be a callback function!");
      zclass_object* d = (zclass_object*)args[0].ptr;
      zclassobj_set(d,".xfercallback",args[2]);
      xfercallback = args[2];
      CurlObject* obj = (CurlObject*)zclassobj_get(d,".handle").ptr;
      CURLcode res = curl_easy_setopt(obj->handle,CURLOPT_XFERINFOFUNCTION,xferfun);
      if(res != CURLE_OK)
        return z_err(Error,curl_easy_strerror(res));
    }
    else if(opt==CURLOPT_POSTFIELDS)
    {
      if(args[2].type!=Z_BYTEARR)
        return z_err(TypeError,"POSTFIELDS option requires a byte array!");
      vector<uint8_t>& l = *(vector<uint8_t>*)args[2].ptr;
      if(l.size()==0)
        return z_err(ValueError,"Empty bytearray");
      char* postfields = new char[l.size()];
      memcpy(postfields,&l[0],l.size());
      zclass_object* d = (zclass_object*)args[0].ptr;
      CurlObject* obj = (CurlObject*)zclassobj_get(d,".handle").ptr;
      obj->postfields = postfields;
      CURLcode res = curl_easy_setopt(obj->handle,(CURLoption)opt,postfields);
      if( res!= CURLE_OK)
        return z_err(Error,curl_easy_strerror(res));
      res = curl_easy_setopt(obj->handle,CURLOPT_POSTFIELDSIZE,l.size());
      
    }
    else
    {
      return z_err(ValueError,"Unknown option used");
      
    }
    return nil;
}
zobject perform(zobject* args,int n)
{
    if(n!=1)
    {
        return z_err(ArgumentError,"1 argument needed!");
        
    }
    zclass_object* d = (zclass_object*)args[0].ptr;
    CurlObject* obj = (CurlObject*)zclassobj_get(d,".handle").ptr;
    CURLcode res;
    res = curl_easy_perform(obj->handle);
    return zobj_from_int64((long long int)res);
}
zobject getinfo(zobject* args,int n)
{
    if(n!=2)
    {
        return z_err(ArgumentError,"2 arguments needed!");
        
    }
    if(args[0].type!='o' || ((zclass_object*)args[0].ptr)->_klass!=curlklass)
    {
      return z_err(TypeError,"Argument 1 must be a Curl object");
      
    }
    if(args[1].type!='l')
    {
      return z_err(TypeError,"Argument 2 must an Integer!");
      
    }
    zclass_object* d = (zclass_object*)args[0].ptr;
    CurlObject* obj = (CurlObject*)zclassobj_get(d,".handle").ptr;
    CURL* handle = obj->handle;
    if(args[1].l==CURLINFO_CONTENT_TYPE)
    {
      char* out;
      CURLcode res = curl_easy_getinfo(handle,(CURLINFO)args[1].l,&out);
      if(!out || res!=CURLE_OK)
      {
        return nil;
      }
      return zobj_from_str(out);

    }
    else if(args[1].l==CURLINFO_HTTP_CODE)
    {
      long out;
      CURLcode res = curl_easy_getinfo(handle,(CURLINFO)args[1].l,&out);
      if(res!=CURLE_OK)
      {
        return z_err(Error,curl_easy_strerror(res));
        
      }
      return zobj_from_int64((long long int)out);
    }
    return nil;
}
/*curl__del__ does the cleanup as well
zobject cleanup(zobject* args,int n)
{
    if(n!=1)
    {
        return z_err(ArgumentError,"1 arguments needed!");
        
    }
    if(args[0].type!='o' || ((zclass_object*)args[0].ptr)->_klass!=curlklass)
    {
      return z_err(TypeError,"Argument 1 must be a Curl object");
      
    }
    zclass_object* d = (zclass_object*)args[0].ptr;
    string tmp = ".handle";
    CurlObject* obj = (CurlObject*)(d->members)[".handle"].ptr;
    curl_easy_cleanup(obj->handle);
    return nil;
}*/


zobject ESCAPE(zobject* args,int n)
{
  if(n!=2)
  {
    return z_err(ArgumentError,"2 Arguments required.");
    
  }
  if(args[0].type != Z_OBJ || ((zclass_object*)args[0].ptr) -> _klass !=curlklass)
  {
    return z_err(TypeError,"Argument 1 must be a Curl Object");
    
  }
  if(args[1].type!=Z_STR)
  {
    return z_err(TypeError,"Argument 1 must be a string");
    
  }
  zclass_object* ki = (zclass_object*)args[0].ptr;
  CurlObject* k = (CurlObject*)zclassobj_get(ki,".handle").ptr;
  zstr* str = AS_STR(args[1]);
  char* output = curl_easy_escape(k->handle,str->val,str->len);
  zobject ret = zobj_from_str(output);
  curl_free(output);
  return ret;
}
zobject UNESCAPE(zobject* args,int n)
{
  if(n!=2)
  {
    return z_err(ArgumentError,"2 Arguments required.");
    
  }
  if(args[0].type != Z_OBJ || ((zclass_object*)args[0].ptr) -> _klass !=curlklass)
  {
    return z_err(TypeError,"Argument 1 must be a Curl Object");
    
  }
  if(args[1].type!=Z_STR)
  {
    return z_err(TypeError,"Argument 1 must be a string");
    
  }
  zclass_object* ki = (zclass_object*)args[0].ptr;
  CurlObject* k = (CurlObject*)AS_PTR(zclassobj_get(ki,".handle"));
  zstr* str = AS_STR(args[1]);
  int decodelen = 0;
  char* output = curl_easy_unescape(k->handle,str->val,str->len,&decodelen);
  zobject ret = zobj_from_str(output);
  curl_free(output);
  return ret;
}
extern "C" void unload()
{
  curl_global_cleanup();
  vm_unmark_important(curlklass);
  vm_unmark_important(mimeklass);
  vm_unmark_important(mimepartklass);
}
