#include "mime.h"
#include "curl.h"
#include "klassobject.h"
#include "mimepart.h"
#include <curl/curl.h>
zclass* mime_class;

zobject mime_construct(zobject* args,int n)
{
    if(n!=2)
        return z_err(ArgumentError,"2 arguments needed!");
    if(args[1].type!='o' || ((zclass_object*)args[1].ptr)->_klass!=curl_class)
        return z_err(TypeError,"curl Object needed!");
    
    zclass_object* d = (zclass_object*)args[0].ptr;
    zclass_object* cobj = (zclass_object*)args[1].ptr;
    CURL* handle = zclassobj_get(cobj,".handle").ptr;
    curl_mime* mime = curl_mime_init(handle);
    
    //Add the mime handle to mime object
    zclassobj_set(d,(".handle"),zobj_from_ptr((void*)mime));

    return zobj_nil();
}
zobject mime_addpart(zobject* args,int n)
{
    if(n!=1)
        return z_err(ArgumentError,"1 arguments needed!");
    if(args[0].type!='o' || ((zclass_object*)args[0].ptr)->_klass!=mime_class)
      return z_err(TypeError,"Argument 1 must be a Mime object");
    zclass_object* d = (zclass_object*)args[0].ptr;
    curl_mime* obj = (curl_mime*)AS_PTR(zclassobj_get(d,".handle"));
    curl_mimepart* mimepart = curl_mime_addpart(obj);
   
    //add methods to object
    zclass_object* part = vm_alloc_zclassobj(mimepart_class);
    zclassobj_set(part,"data",zobj_from_method("mimepart.data",&mimepart_data,mimepart_class));
    zclassobj_set(part,("name"),zobj_from_method("mimepart.name",&mimepart_name,mimepart_class));
    zclassobj_set(part,("filename"),zobj_from_method("mimepart.filename",&mimepart_filename,mimepart_class));
    zclassobj_set(part,("type"),zobj_from_method("mimepart.type",&mimepart_content_type,mimepart_class));
    zclassobj_set(part,("__del__"),zobj_from_method("mimepart.__del__",&mimepart__del__,mimepart_class));
    zclassobj_set(part,(".handle"),zobj_from_ptr((void*)mimepart));

    return zobj_from_classobj(part);
}
zobject mime__del__(zobject* args,int n)
{
    if(n!=1)
        return z_err(ArgumentError,"1 argument needed!");
    zclass_object* k = (zclass_object*)args[0].ptr;
    curl_mime* obj = (curl_mime*)AS_PTR(zclassobj_get(k,".handle"));
    curl_mime_free(obj);
    return zobj_nil();
}
