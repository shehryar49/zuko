#include "mimepart.h"
#include "zapi.h"
#include <curl/curl.h>


zclass* mimepart_class;

zobject mimepart_name(zobject* args,int n)
{
    if(n!=2)
        return z_err(ArgumentError,"2 argument needed!");
    if(args[1].type!='s')
        return z_err(TypeError,"Argument 2 must be string!");
    if(args[0].type!='o' || ((zclass_object*)args[0].ptr)->_klass!=mimepart_class)
      return z_err(TypeError,"Argument 1 must be a MimePart object");
  zclass_object* d = (zclass_object*)args[0].ptr;
  curl_mimepart* obj = (curl_mimepart*)AS_PTR(zclassobj_get(d,".handle"));
  zstr* name = AS_STR(args[1]);
  curl_mime_name(obj,name->val);
  return zobj_nil();
}
zobject mimepart_filename(zobject* args,int n)
{
    if(n!=2)
        return z_err(ArgumentError,"2 argument needed!");
    if(args[1].type!='s')
        return z_err(TypeError,"Argument 2 must be string!");
    if(args[0].type!='o' || ((zclass_object*)args[0].ptr)->_klass!=mimepart_class)
      return z_err(TypeError,"Argument 1 must be a MimePart object");
  zclass_object* d = (zclass_object*)args[0].ptr;
  curl_mimepart* obj = (curl_mimepart*)AS_PTR(zclassobj_get(d,".handle"));
  zstr* name = AS_STR(args[1]);
  curl_mime_filename(obj,name->val);
  return zobj_nil();
}
zobject mimepart_content_type(zobject* args,int n)
{
    if(n!=2)
        return z_err(ArgumentError,"2 argument needed!");
    if(args[1].type!='s')
        return z_err(TypeError,"Argument 2 must be string!");
    if(args[0].type!='o' || ((zclass_object*)args[0].ptr)->_klass!=mimepart_class)
      return z_err(TypeError,"Argument 1 must be a MimePart object");
  zclass_object* d = (zclass_object*)args[0].ptr;
  curl_mimepart* obj = (curl_mimepart*)AS_PTR(zclassobj_get(d,".handle"));
  zstr* name = AS_STR(args[1]);
  curl_mime_type(obj,name->val);
  return zobj_nil();
}
zobject mimepart_data(zobject* args,int n)
{
    if(n!=2)
        return z_err(ArgumentError,"1 argument needed!");
    if(args[1].type!='c')
      return z_err(TypeError,"Argument 2 must be a Byte array!");
    if(args[0].type!='o' || ((zclass_object*)args[0].ptr)->_klass!=mimepart_class)
      return z_err(TypeError,"Argument 1 must be a MimePart object");
    zclass_object* d = (zclass_object*)args[0].ptr;
    curl_mimepart* obj = (curl_mimepart*)AS_PTR(zclassobj_get(d,".handle"));
    auto l = AS_BYTEARRAY(args[1]);
    curl_mime_data(obj,(const char*)l->arr,l->size);
    return zobj_nil();
}

zobject mimepart__del__(zobject* args,int n)
{
    return zobj_nil();
/*   if(n!=1)
        return z_err(ArgumentError,"1 argument needed!");
    zclass_object* k = (zclass_object*)args[0].ptr;
    curl_mimepart* obj = (curl_mimepart*)zclassobj_get(k,".handle").ptr;
    delete obj;
    return zobj_nil();*/
}
