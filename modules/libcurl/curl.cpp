#include "curl.h"
#include "klassobject.h"
#include "zapi.h"
#include "zobject.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include <string>
#include <vector>
using namespace std;

zclass* curl_class;

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
//zobject wmcallback;
//zobject xfercallback;
zobject quickErr(zclass*,std::string);

size_t WMCallbackHandler(void *contents, size_t size, size_t nmemb, void *userp)
{
  zclass_object* obj = (zclass_object*)userp;
  puts("here");
  zobject wmcallback = zclassobj_get(obj,".wmcallback");
  printf("wmcallback.type = %c\n",wmcallback.type);
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
  zclass_object* obj = (zclass_object*)clientp;
  zobject xfercallback = zclassobj_get(obj,".xfercallback");
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

zobject zcurl_construct(zobject* args,int n)
{
  if(n!=1)
    return z_err(ArgumentError,"1 arguments needed!");
    
  zclass_object* curobj = (zclass_object*)args[0].ptr;
  CURL* curl = curl_easy_init();
  if(!curl)
    return z_err(Error,"failed");
  
  //Return an object
  zclassobj_set(curobj,".handle",zobj_from_ptr(curl));
  return zobj_nil();
 
}
zobject zcurl_setopt(zobject* args,int n)
{
    if(n!=3)
    {
        return z_err(ArgumentError,"3 arguments needed!");
        
    }
    if(args[1].type!=Z_INT64)
    {
        return z_err(TypeError,"Argument 2 must an Integer!");
        
    }
    if(args[0].type!='o' || ((zclass_object*)args[0].ptr)->_klass!=curl_class)
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
      CURL* obj = (CURL*)zclassobj_get(d,".handle").ptr;
      CURLcode res = curl_easy_setopt(obj,(CURLoption)opt,AS_STR(args[2])->val);
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
      CURL* obj = (CURL*)zclassobj_get(d,".handle").ptr;
      CURLcode res = curl_easy_setopt(obj,(CURLoption)opt,args[2].i);
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
      CURL* obj = (CURL*)zclassobj_get(d,".handle").ptr;
      CURLcode res = curl_easy_setopt(obj,(CURLoption)opt,args[2].i);
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
      CURL* obj = (CURL*)zclassobj_get(d,".handle").ptr;
      CURLcode res = curl_easy_setopt(obj,(CURLoption)opt,args[2].i);
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
      CURL* obj = (CURL*)zclassobj_get(d,".handle").ptr;
      zclass_object* e = (zclass_object*)args[2].ptr;
      curl_mime* m = (curl_mime*)zclassobj_get(e,".handle").ptr;
      CURLcode res = curl_easy_setopt(obj,(CURLoption)opt,m);
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
       // wmcallback = args[2];
        CURL* obj = (CURL*)zclassobj_get(d,".handle").ptr;
        zclassobj_set(d,".wmcallback",args[2]);
        CURLcode res = curl_easy_setopt(obj,(CURLoption)opt,&WMCallbackHandler);
        if( res!= CURLE_OK)
          return z_err(Error,curl_easy_strerror(res));
        curl_easy_setopt(obj,CURLOPT_WRITEDATA, d);
        return zobj_nil();
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
      CURL* obj = (CURL*)zclassobj_get(d,".handle").ptr;
      CURLcode res = curl_easy_setopt(obj,(CURLoption)opt,&WriteMemoryCallback);
      if( res!= CURLE_OK)
      {
        return z_err(Error,curl_easy_strerror(res));
        
      }
      curl_easy_setopt(obj,CURLOPT_WRITEDATA,btArr);
    }
    else if(opt==CURLOPT_USERAGENT)
    {
      if(args[2].type!=Z_STR)
      {
        return z_err(TypeError,"USERAGENT option requires a string value");
         
      }
      zclass_object* d = (zclass_object*)args[0].ptr;
      CURL* obj = (CURL*)zclassobj_get(d,".handle").ptr;
      CURLcode res = curl_easy_setopt(obj,(CURLoption)opt,AS_STR(args[2])->val);
      if( res!= CURLE_OK)
      {
        return z_err(Error,curl_easy_strerror(res));
        
      }
    }
    else if( opt == CURLOPT_NOPROGRESS)
    {
      zclass_object* d = (zclass_object*)args[0].ptr;
      CURL* obj = (CURL*)zclassobj_get(d,".handle").ptr;
      CURLcode res = curl_easy_setopt(obj,CURLOPT_NOPROGRESS,args[2].i);
      if(res != CURLE_OK)
	      return z_err(Error,curl_easy_strerror(res));
    }
    else if(opt == CURLOPT_XFERINFOFUNCTION)
    {
      if(args[2].type!=Z_FUNC)
        return z_err(TypeError,"XFERINFOFUNCTION option must be a callback function!");
      zclass_object* d = (zclass_object*)args[0].ptr;
      zclassobj_set(d,".xfercallback",args[2]);
      CURL* obj = (CURL*)zclassobj_get(d,".handle").ptr;
      zclassobj_set(d,".xfercallback",args[2]);
      CURLcode res = curl_easy_setopt(obj,CURLOPT_XFERINFOFUNCTION,xferfun);
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
      zclass_object* d = (zclass_object*)args[0].ptr;
      CURL* obj = (CURL*)zclassobj_get(d,".handle").ptr;
      zclassobj_set(d,".postfields",args[2]);
      CURLcode res = curl_easy_setopt(obj,(CURLoption)opt,&l[0]);
      if( res!= CURLE_OK)
        return z_err(Error,curl_easy_strerror(res));
      res = curl_easy_setopt(obj,CURLOPT_POSTFIELDSIZE,l.size());
      
    }
    else
    {
      return z_err(ValueError,"Unknown option used");
      
    }
    return zobj_nil();
}
zobject zcurl_perform(zobject* args,int n)
{
    if(n!=1)
    {
        return z_err(ArgumentError,"1 argument needed!");
        
    }
    zclass_object* d = (zclass_object*)args[0].ptr;
    CURL* obj = (CURL*)zclassobj_get(d,".handle").ptr;
    CURLcode res;
    res = curl_easy_perform(obj);
    return zobj_from_int64((long long int)res);
}
zobject zcurl_getinfo(zobject* args,int n)
{
    if(n!=2)
    {
        return z_err(ArgumentError,"2 arguments needed!");
        
    }
    if(args[0].type!='o' || ((zclass_object*)args[0].ptr)->_klass!=curl_class)
    {
      return z_err(TypeError,"Argument 1 must be a Curl object");
      
    }
    if(args[1].type!='l')
    {
      return z_err(TypeError,"Argument 2 must an Integer!");
      
    }
    zclass_object* d = (zclass_object*)args[0].ptr;
    CURL* handle = (CURL*)zclassobj_get(d,".handle").ptr;
    if(args[1].l==CURLINFO_CONTENT_TYPE)
    {
      char* out;
      CURLcode res = curl_easy_getinfo(handle,(CURLINFO)args[1].l,&out);
      if(!out || res!=CURLE_OK)
      {
        return zobj_nil();
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
    return zobj_nil();
}
zobject zcurl__del__(zobject* args,int n)//
{
    if(n != 1 || args[0].type != Z_OBJ)
      return zobj_nil();
    zclass_object* d = (zclass_object*)args[0].ptr;
    if(d->_klass != curl_class)
      return zobj_nil();
    zobject ptr = zclassobj_get(d,".handle");
    if(ptr.type == 'n')
      return zobj_nil();
    CURL* obj = (CURL*)zclassobj_get(d,".handle").ptr;
    curl_easy_cleanup(obj);
    zclassobj_set(d,".handle",zobj_nil());
    return zobj_nil();
}


zobject zcurl_escape(zobject* args,int n)
{
  if(n!=2)
  {
    return z_err(ArgumentError,"2 Arguments required.");
    
  }
  if(args[0].type != Z_OBJ || ((zclass_object*)args[0].ptr) -> _klass !=curl_class)
  {
    return z_err(TypeError,"Argument 1 must be a Curl Object");
    
  }
  if(args[1].type!=Z_STR)
  {
    return z_err(TypeError,"Argument 1 must be a string");
    
  }
  zclass_object* ki = (zclass_object*)args[0].ptr;
  CURL* handle = (CURL*)zclassobj_get(ki,".handle").ptr;
  zstr* str = AS_STR(args[1]);
  char* output = curl_easy_escape(handle,str->val,str->len);
  zobject ret = zobj_from_str(output);
  curl_free(output);
  return ret;
}
zobject zcurl_unescape(zobject* args,int n)
{
  if(n!=2)
  {
    return z_err(ArgumentError,"2 Arguments required.");
    
  }
  if(args[0].type != Z_OBJ || ((zclass_object*)args[0].ptr) -> _klass !=curl_class)
  {
    return z_err(TypeError,"Argument 1 must be a Curl Object");
    
  }
  if(args[1].type!=Z_STR)
  {
    return z_err(TypeError,"Argument 1 must be a string");
    
  }
  zclass_object* ki = (zclass_object*)args[0].ptr;
  CURL* handle = (CURL*)zclassobj_get(ki,".handle").ptr;
  zstr* str = AS_STR(args[1]);
  int decodelen = 0;
  char* output = curl_easy_unescape(handle,str->val,str->len,&decodelen);
  zobject ret = zobj_from_str(output);
  curl_free(output);
  return ret;
}