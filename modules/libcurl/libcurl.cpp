
/*
Libcurl Binding for Zuko(easy interface only)
The original code of libcurl is not modified in anyway. This is just a wrapper around libcurl
and requires libcurl libraries to be linked when compiling.
Written by Shahryar Ahmad
*/
#include "curl_slist.h"
#include "zapi.h"
#ifdef _WIN32
  #define CURL_STATICLIB // don't want to distribute dlls
  #pragma comment(lib,"crypt32.lib")
  #pragma comment(lib,"Normaliz.lib")
  #pragma comment(lib,"Ws2_32.lib")
  #pragma comment(lib,"Wldap32.lib")
  #pragma comment(lib,"libcurl_a.lib")
#endif
#include <curl/curl.h>
#include <string>
#include "libcurl.h"
#include "curl.h"
#include "mime.h"
#include "mimepart.h"

using namespace std;

zobject quickErr(zclass* k,string s)
{
  return z_err(k,s.c_str());
}
zobject init()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);

    zmodule* d = vm_alloc_zmodule("libcurl");
    //
    curl_class = vm_alloc_zclass("libcurl.curl");

    zclass_add_method(curl_class,"__construct__",&zcurl_construct);
    zclass_add_method(curl_class,"perform",&zcurl_perform);
    zclass_add_method(curl_class,"setopt",&zcurl_setopt);
    zclass_add_method(curl_class,"getinfo",&zcurl_getinfo);
    zclass_add_method(curl_class,"__del__",&zcurl__del__);
    zclass_add_method(curl_class,"escape",&zcurl_escape);
    zclass_add_method(curl_class,"unescape",&zcurl_unescape);
    
    //
    mimepart_class = vm_alloc_zclass("libcurl.mimepart");
    zclass_add_method(mimepart_class,"data",mimepart_data);
    zclass_add_method(mimepart_class,"name",mimepart_name);
    zclass_add_method(mimepart_class,"filename",mimepart_filename);
    zclass_add_method(mimepart_class,"filedata",mimepart_filedata);
    zclass_add_method(mimepart_class,"type",mimepart_content_type);
    zclass_add_method(mimepart_class,"__del__",mimepart__del__);

    //
    mime_class = vm_alloc_zclass("libcurl.mimepart");
    //add methods to object
    zclass_add_method(mime_class,"__construct__",&mime_construct);
    zclass_add_method(mime_class,"addpart",&mime_addpart);
    zclass_add_method(mime_class,"__del__",&mime__del__);
    //
    slist_class = vm_alloc_zclass("libcurl.slist");
    zclass_add_method(slist_class,"__construct__",&slist_construct);
    zclass_add_method(slist_class,"append",&slist_append);
    zclass_add_method(slist_class,"__del__",&slist__del__);
    //
    zmodule_add_class(d, "curl", curl_class);
    zmodule_add_class(d, "mime", mime_class);
    zmodule_add_class(d, "slist", slist_class);
    zmodule_add_fun(d,"strerror",&wrapped_strerror);
    zmodule_add_member(d,"OPT_URL",zobj_from_int64(CURLOPT_URL));
    zmodule_add_member(d,"OPT_PORT",zobj_from_int64(CURLOPT_PORT));
    zmodule_add_member(d,"OPT_POST",zobj_from_int64(CURLOPT_POST));
    //zmodule_add_member(d,"OPT_HTTPPOST",zobj_from_int64(CURLOPT_HTTPPOST)); deprecated use MIMEPOST
    zmodule_add_member(d,"OPT_POSTFIELDS",zobj_from_int64(CURLOPT_POSTFIELDS));
    zmodule_add_member(d,"OPT_USERAGENT",zobj_from_int64(CURLOPT_USERAGENT));
    zmodule_add_member(d,"OPT_FOLLOWLOCATION",zobj_from_int64(CURLOPT_FOLLOWLOCATION));
    zmodule_add_member(d,"OPT_WRITEFUNCTION",zobj_from_int64(CURLOPT_WRITEFUNCTION));
    zmodule_add_member(d,"OPT_WRITEDATA",zobj_from_int64(CURLOPT_WRITEDATA));
    zmodule_add_member(d,"OPT_READFUNCTION",zobj_from_int64(CURLOPT_READFUNCTION));
    zmodule_add_member(d,"OPT_READDATA",zobj_from_int64(CURLOPT_READDATA));
    zmodule_add_member(d,"OPT_HEADERFUNCTION",zobj_from_int64(CURLOPT_HEADERFUNCTION));
    zmodule_add_member(d,"OPT_HEADERDATA",zobj_from_int64(CURLOPT_HEADERDATA));

    zmodule_add_member(d,"OPT_MIMEPOST",zobj_from_int64(CURLOPT_MIMEPOST));
    zmodule_add_member(d,"OPT_VERBOSE",zobj_from_int64(CURLOPT_VERBOSE));
    zmodule_add_member(d,"OPT_XFERINFOFUNCTION",zobj_from_int64(CURLOPT_XFERINFOFUNCTION));
    zmodule_add_member(d,"OPT_XFERINFODATA",zobj_from_int64(CURLOPT_XFERINFODATA));
    
    zmodule_add_member(d,"OPT_NOPROGRESS",zobj_from_int64(CURLOPT_NOPROGRESS));
    zmodule_add_member(d,"OPT_HTTPHEADER",zobj_from_int64(CURLOPT_HTTPHEADER));
    zmodule_add_member(d,"OPT_UPLOAD",zobj_from_int64(CURLOPT_UPLOAD));
    zmodule_add_member(d,"OPT_CUSTOMREQUEST",zobj_from_int64(CURLOPT_CUSTOMREQUEST));
    
    zmodule_add_member(d,"CURLE_OK",zobj_from_int64(CURLE_OK));
    zmodule_add_member(d,"WriteMemory",zobj_from_int(0));
    zmodule_add_member(d,"INFO_CONTENT_TYPE",zobj_from_int64((long long int)CURLINFO_CONTENT_TYPE));
    zmodule_add_member(d,"INFO_HTTP_CODE",zobj_from_int64((long long int)CURLINFO_HTTP_CODE));
    zmodule_add_member(d,"VERSION_NUM",zobj_from_int(LIBCURL_VERSION_NUM));
    zmodule_add_member(d,"VERSION_MAJOR",zobj_from_int(LIBCURL_VERSION_MAJOR));
    zmodule_add_member(d,"VERSION_MINOR",zobj_from_int(LIBCURL_VERSION_MINOR));
    
    vm_mark_important(curl_class);
    vm_mark_important(mime_class);
    vm_mark_important(mimepart_class);
    return zobj_from_module(d);
}
zobject wrapped_strerror(zobject* args,int n)
{
  if(n!=1)
    return z_err(ArgumentError,"1 argument needed!"); 
  if(args[0].type!='l')
    return z_err(TypeError,"Integer argument needed!");
  const char* s = curl_easy_strerror((CURLcode)args[0].l);
  return zobj_from_str(s);
}

void unload()
{
  curl_global_cleanup();
  vm_unmark_important(curl_class);
  vm_unmark_important(mime_class);
  vm_unmark_important(mimepart_class);
}
