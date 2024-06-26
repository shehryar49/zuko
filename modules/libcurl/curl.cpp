#include "curl.h"
#include "curl_slist.h"
#include "mime.h"
#include "zobject.h"
#include <cstdint>
#include <curl/curl.h>
#include <curl/easy.h>
#include <string>
#include <vector>

using namespace std;

zclass* curl_class;

//Default WriteMemory function
//pushes all the bytes to a bytearray
size_t write_memory_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  zbytearr* mem = (zbytearr*)userp;
  size_t prevsize = mem->size;
  zbytearr_resize(mem,prevsize + realsize);
  memcpy(mem->arr+prevsize, contents, realsize);
  return realsize;
}
//WriteMemory function which calls zuko callback and passes the bytes
size_t wm_callback_handler(void *contents, size_t size, size_t nmemb, void *userp)
{
  zclass_object* obj = (zclass_object*)userp;
  zobject wmcallback = zclassobj_get(obj,".wmcallback");
  zobject writedata = zclassobj_get(obj,".writedata");

  size_t realsize = size * nmemb;
  auto btArr = vm_alloc_zbytearr();
  zbytearr_resize(btArr,realsize);
  memcpy(btArr->arr, contents, realsize);
  zobject rr;
  zobject p1 = zobj_from_bytearr(btArr);
  //send newly received memory to callback function
  
  zobject args[] = {p1,writedata};
  vm_call_object(&wmcallback,args,2, &rr);

  return realsize;
}
//xfer function callback handler
int xferfun_callback_handler(void* clientp,curl_off_t dltotal,curl_off_t dlnow,curl_off_t ultotal,curl_off_t ulnow)
{

  zclass_object* obj = (zclass_object*)clientp;
  zobject xfercallback = zclassobj_get(obj,".xfercallback");
  zobject xferinfodata = zclassobj_get(obj,".xferinfodata");

  zobject args[5] = {
    xferinfodata,
    zobj_from_int64(dltotal),
    zobj_from_int64(dlnow),
    zobj_from_int64(ultotal),
    zobj_from_int64(ulnow)
  };
  zobject r;
  bool good = vm_call_object(&xfercallback,args,5,&r);
  if(r.type == Z_INT)
    return r.i;
  return 0;
}
size_t read_callback_handler(char *ptr, size_t size, size_t nmemb, void *userdata)
{
  zclass_object* curl_object = (zclass_object*)userdata;
  zobject readfunction = zclassobj_get(curl_object,".readfunction");
  zobject readdata = zclassobj_get(curl_object,".readdata");

  /* copy as much data as possible into the 'ptr' buffer, but no more than
     'size' * 'nmemb' bytes! */
  
  zobject args[] = {
    zobj_from_int64(size*nmemb),
    readdata
  };
  zobject rr;
  bool good = vm_call_object(&readfunction,args,2,&rr);
  if(!good || rr.type!=Z_BYTEARR)
    return 0;
  zbytearr* bytes = (zbytearr*)rr.ptr;
  if(bytes->size > size*nmemb)
    return 0;
  memcpy(ptr,bytes->arr,bytes->size*sizeof(uint8_t));

  return (curl_off_t)bytes->size; 
}
size_t header_callback_handler(char *buffer, size_t size,size_t nitems, void *userdata)
{
  /* received header is nitems * size long in 'buffer' NOT ZERO TERMINATED */
  /* 'userdata' is set with CURLOPT_HEADERDATA */
  zclass_object* curl_object = (zclass_object*)userdata;
  zobject headerfn = zclassobj_get(curl_object,".headerfn");
  zobject headerdata = zclassobj_get(curl_object,".headerdata");

  zstr* str = vm_alloc_zstr(size*nitems);
  memcpy(str->val,buffer,size*nitems);
  zobject args[] = {zobj_from_str_ptr(str),headerdata};
  zobject rr;
  vm_call_object(&headerfn,args,2,&rr);
  return nitems * size;
}
zobject zcurl_construct(zobject* args,int n)
{
    if(n!=1)
        return z_err(ArgumentError,"1 arguments needed!");  
    zclass_object* curobj = (zclass_object*)args[0].ptr;
    if(args[0].type != Z_OBJ || curobj->_klass!=curl_class)
        return z_err(TypeError,"Argument 1 must be an object of libcurl.curl class");
    CURL* curl = curl_easy_init();
    if(!curl)
        return z_err(Error,"curl_easy_init() failed");
    //Return an object
    zclassobj_set(curobj,".handle",zobj_from_ptr(curl));
    return zobj_nil();
}
zobject zcurl_setopt(zobject* args,int n)
{
    if(n!=3)
        return z_err(ArgumentError,"3 arguments needed!");   
    if(args[1].type!=Z_INT64)
        return z_err(TypeError,"Argument 2 must an Integer!");
    if(args[0].type!='o' || ((zclass_object*)args[0].ptr)->_klass!=curl_class)
      return z_err(TypeError,"Argument 1 must be a Curl object");
    long long int opt = args[1].l;

    if(opt==CURLOPT_URL)
    {
        if(args[2].type!=Z_STR)
            return z_err(TypeError,"URL option requires a string value");
    
        zclass_object* d = (zclass_object*)args[0].ptr;
        CURL* handle = (CURL*)zclassobj_get(d,".handle").ptr;
        CURLcode res = curl_easy_setopt(handle,(CURLoption)opt,AS_STR(args[2])->val);
        if( res!= CURLE_OK)
            return z_err(Error,curl_easy_strerror(res));  
    }
    else if(opt==CURLOPT_FOLLOWLOCATION)
    {
      if(args[2].type!=Z_INT)
        return z_err(TypeError,"FOLLOWLOCATION option requires an integer value");
      zclass_object* d = (zclass_object*)args[0].ptr;
      CURL* obj = (CURL*)zclassobj_get(d,".handle").ptr;
      CURLcode res = curl_easy_setopt(obj,(CURLoption)opt,args[2].i);
      if( res!= CURLE_OK)
        return z_err(Error,curl_easy_strerror(res));
    }
    else if(opt==CURLOPT_VERBOSE)
    {
        if(args[2].type!=Z_INT)
            return z_err(TypeError,"VERBOSE option requires an integer value");
        zclass_object* d = (zclass_object*)args[0].ptr;
        CURL* obj = (CURL*)zclassobj_get(d,".handle").ptr;
        CURLcode res = curl_easy_setopt(obj,(CURLoption)opt,args[2].i);
        if( res!= CURLE_OK)
            return z_err(Error,curl_easy_strerror(res));
    }
    else if(opt==CURLOPT_PORT)
    {
        if(args[2].type!=Z_INT)
            return z_err(TypeError,"PORT option requires an integer value");
        zclass_object* d = (zclass_object*)args[0].ptr;
        CURL* obj = (CURL*)zclassobj_get(d,".handle").ptr;
        CURLcode res = curl_easy_setopt(obj,(CURLoption)opt,args[2].i);
        if( res!= CURLE_OK)
            return z_err(Error,curl_easy_strerror(res));
    }
    else if(opt==CURLOPT_MIMEPOST)
    {
        zclass_object* e = (zclass_object*)args[2].ptr;
        if(args[2].type!=Z_OBJ || e->_klass!=mime_class) 
            return z_err(TypeError,"MIMEPOST option requires a libcurl.mime object");
        zclass_object* d = (zclass_object*)args[0].ptr;
        CURL* obj = (CURL*)zclassobj_get(d,".handle").ptr;
        curl_mime* m = (curl_mime*)zclassobj_get(e,".handle").ptr;
        CURLcode res = curl_easy_setopt(obj,(CURLoption)opt,m);
        if( res!= CURLE_OK)
            return z_err(Error,curl_easy_strerror(res));
    }
    else if(opt==CURLOPT_WRITEFUNCTION)
    {
        if(args[2].type == Z_FUNC || args[2].type == Z_NATIVE_FUNC)
        {
            //set callback
            zclass_object* obj = (zclass_object*)args[0].ptr;
            CURL* handle = (CURL*)zclassobj_get(obj,".handle").ptr;
            zclassobj_set(obj,".wmcallback",args[2]);
            CURLcode res = curl_easy_setopt(handle,(CURLoption)opt,&wm_callback_handler);
            if( res!= CURLE_OK)
            return z_err(Error,curl_easy_strerror(res));
            curl_easy_setopt(handle,CURLOPT_WRITEDATA, obj);
            return zobj_nil();
        }
        if(args[2].type !=Z_INT || args[2].i!=0 )
            return z_err(TypeError,"Invalid option value");
        zclass_object* d = (zclass_object*)args[0].ptr;
        auto btArr = vm_alloc_zbytearr();
        zobject p1 = zobj_from_bytearr(btArr);
        zclassobj_set(d,"data",p1);
        CURL* obj = (CURL*)zclassobj_get(d,".handle").ptr;
        CURLcode res = curl_easy_setopt(obj,(CURLoption)opt,&write_memory_callback);
        if( res!= CURLE_OK)
            return z_err(Error,curl_easy_strerror(res));
        curl_easy_setopt(obj,CURLOPT_WRITEDATA,btArr);
    }
    else if(opt == CURLOPT_READFUNCTION)
    {
      if(args[2].type == Z_FUNC || args[2].type == Z_NATIVE_FUNC)
      {
        //set callback
        zclass_object* d = (zclass_object*)args[0].ptr;
        CURL* obj = (CURL*)zclassobj_get(d,".handle").ptr;
        zclassobj_set(d,".readfunction",args[2]);
        CURLcode res = curl_easy_setopt(obj,(CURLoption)opt,&read_callback_handler);
        if( res!= CURLE_OK)
          return z_err(Error,curl_easy_strerror(res));
        curl_easy_setopt(obj,CURLOPT_WRITEDATA, d);
        return zobj_nil();
      }
    }
    else if(opt == CURLOPT_HEADERFUNCTION)
    {
      if(args[2].type == Z_FUNC || args[2].type == Z_NATIVE_FUNC)
      {
        //set callback
        zclass_object* d = (zclass_object*)args[0].ptr;
        CURL* obj = (CURL*)zclassobj_get(d,".handle").ptr;
        zclassobj_set(d,".headerfn",args[2]);
        CURLcode res = curl_easy_setopt(obj,(CURLoption)opt,&header_callback_handler);
        if( res!= CURLE_OK)
          return z_err(Error,curl_easy_strerror(res));
        curl_easy_setopt(obj,CURLOPT_HEADERDATA, d);
        return zobj_nil();
      }
    }
    else if(opt == CURLOPT_READDATA)
    {
      zclass_object* curl_object = (zclass_object*)args[0].ptr;
      zclassobj_set(curl_object,".readdata",args[2]);
    }
    else if(opt == CURLOPT_WRITEDATA)
    {
      zclass_object* curl_object = (zclass_object*)args[0].ptr;
      zclassobj_set(curl_object,".writedata",args[2]);
    }
    else if(opt == CURLOPT_XFERINFODATA)
    {
      zclass_object* curl_object = (zclass_object*)args[0].ptr;
      zclassobj_set(curl_object,".xferinfodata",args[2]);
    }
    else if(opt == CURLOPT_HEADERDATA)
    {
      zclass_object* curl_object = (zclass_object*)args[0].ptr;
      zclassobj_set(curl_object,".headerdata",args[2]);
    }
    else if(opt==CURLOPT_USERAGENT)
    {
      if(args[2].type!=Z_STR)
        return z_err(TypeError,"USERAGENT option requires a string value");
      zclass_object* d = (zclass_object*)args[0].ptr;
      CURL* obj = (CURL*)zclassobj_get(d,".handle").ptr;
      CURLcode res = curl_easy_setopt(obj,(CURLoption)opt,AS_STR(args[2])->val);
      if( res!= CURLE_OK)
      {
        return z_err(Error,curl_easy_strerror(res));
        
      }
    }
    else if(opt==CURLOPT_CUSTOMREQUEST)
    {
      if(args[2].type!=Z_STR)
        return z_err(TypeError,"CUSTOMREQUEST option requires a string value");
      zclass_object* d = (zclass_object*)args[0].ptr;
      CURL* obj = (CURL*)zclassobj_get(d,".handle").ptr;
      CURLcode res = curl_easy_setopt(obj,(CURLoption)opt,AS_STR(args[2])->val);
      if( res!= CURLE_OK)
        return z_err(Error,curl_easy_strerror(res));
    }
    else if( opt == CURLOPT_NOPROGRESS)
    {
      zclass_object* d = (zclass_object*)args[0].ptr;
      CURL* obj = (CURL*)zclassobj_get(d,".handle").ptr;
      CURLcode res = curl_easy_setopt(obj,CURLOPT_NOPROGRESS,args[2].i);
      if(res != CURLE_OK)
	      return z_err(Error,curl_easy_strerror(res));
    }
    else if( opt == CURLOPT_UPLOAD)
    {
      zclass_object* d = (zclass_object*)args[0].ptr;
      CURL* obj = (CURL*)zclassobj_get(d,".handle").ptr;
      CURLcode res = curl_easy_setopt(obj,CURLOPT_UPLOAD,args[2].i);
      if(res != CURLE_OK)
	      return z_err(Error,curl_easy_strerror(res));
    }
    else if(opt == CURLOPT_HTTPHEADER)
    {
      if(args[2].type != Z_OBJ || ((zclass_object*)args[2].ptr)->_klass != slist_class)
        return z_err(TypeError,"Argument 1 must be an object of libcurl.slist");
      zclass_object* obj = (zclass_object*)args[2].ptr;
      CURL* handle = (CURL*)zclassobj_get((zclass_object*)args[0].ptr,".handle").ptr;
      curl_slist* list;
      list = (curl_slist*)zclassobj_get(obj,".handle").ptr;
      CURLcode res = curl_easy_setopt(handle,CURLOPT_HTTPHEADER,list);
      if(res != CURLE_OK)
	      return z_err(Error,curl_easy_strerror(res));
    }
    else if( opt == CURLOPT_POST)
    {
      zclass_object* d = (zclass_object*)args[0].ptr;
      CURL* obj = (CURL*)zclassobj_get(d,".handle").ptr;
      CURLcode res = curl_easy_setopt(obj,CURLOPT_POST,args[2].i);
      if(res != CURLE_OK)
	      return z_err(Error,curl_easy_strerror(res));
    }
    else if(opt == CURLOPT_XFERINFOFUNCTION)
    {
      if(args[2].type!=Z_FUNC && args[2].type != Z_NATIVE_FUNC)
        return z_err(TypeError,"XFERINFOFUNCTION option must be a callback function!");
      zclass_object* d = (zclass_object*)args[0].ptr;
      CURL* obj = (CURL*)zclassobj_get(d,".handle").ptr;
      zclassobj_set(d,".xfercallback",args[2]);
      CURLcode res = curl_easy_setopt(obj,CURLOPT_XFERINFOFUNCTION,xferfun_callback_handler);
      if(res != CURLE_OK)
        return z_err(Error,curl_easy_strerror(res));
      curl_easy_setopt(obj,CURLOPT_XFERINFODATA,d);
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
        return z_err(ArgumentError,"1 argument needed!");
    zclass_object* d = (zclass_object*)args[0].ptr;
    CURL* obj = (CURL*)zclassobj_get(d,".handle").ptr;
    CURLcode res;
    res = curl_easy_perform(obj);
    return zobj_from_int64((long long int)res);
}
zobject zcurl_getinfo(zobject* args,int n)
{
    if(n!=2)
        return z_err(ArgumentError,"2 arguments needed!");
    if(args[0].type!='o' || ((zclass_object*)args[0].ptr)->_klass!=curl_class)
      return z_err(TypeError,"Argument 1 must be a Curl object");
    if(args[1].type!='l')
      return z_err(TypeError,"Argument 2 must an Integer!");
    
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
zobject zcurl__del__(zobject* args,int n)
{
    if(n != 1 || args[0].type != Z_OBJ)
      return zobj_nil();
    zclass_object* d = (zclass_object*)args[0].ptr;
    if(d->_klass != curl_class)
      return zobj_nil();
    zobject ptr = zclassobj_get(d,".handle");
    if(ptr.type == Z_NIL)
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
    return z_err(ArgumentError,"2 Arguments required.");
  if(args[0].type != Z_OBJ || ((zclass_object*)args[0].ptr) -> _klass !=curl_class)
    return z_err(TypeError,"Argument 1 must be a Curl Object");
  if(args[1].type!=Z_STR)
    return z_err(TypeError,"Argument 1 must be a string");
  zclass_object* ki = (zclass_object*)args[0].ptr;
  CURL* handle = (CURL*)zclassobj_get(ki,".handle").ptr;
  zstr* str = AS_STR(args[1]);
  int decodelen = 0;
  char* output = curl_easy_unescape(handle,str->val,str->len,&decodelen);
  zobject ret = zobj_from_str(output);
  curl_free(output);
  return ret;
}