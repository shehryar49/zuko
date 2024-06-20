#include "curl_slist.h"
#include "klassobject.h"
#include <curl/curl.h>


zclass* slist_class;

zobject slist_construct(zobject* args,int32_t n)
{
    if(n!=1)
        return z_err(ArgumentError,"slist.__construct__() takes exactly 1 argument!");
    zobject tmp = args[0];
    if(tmp.type != Z_OBJ || ((zclass_object*)tmp.ptr)->_klass != slist_class )
        return z_err(TypeError,"Argument 1 must be an object of slist class!");
    zclass_object* obj = ((zclass_object*)tmp.ptr);

    curl_slist* list = NULL;
    zclassobj_set(obj,".handle",zobj_from_ptr(list));
    return zobj_nil();
}

zobject slist_append(zobject* args,int32_t n)
{
    if(n!=2)
        return z_err(ArgumentError,"slist.append() takes exactly 2 arguments!");
    zobject tmp = args[0];
    if(tmp.type != Z_OBJ || ((zclass_object*)tmp.ptr)->_klass != slist_class )
        return z_err(TypeError,"Argument 1 must be an object of slist class!");
    if(args[1].type != Z_STR)
        return z_err(TypeError,"Argument 2 must be a string!");
    const char* val = AS_STR(args[1])->val;
    zclass_object* obj = ((zclass_object*)tmp.ptr);


    zobject t = zclassobj_get(obj,".handle");
    curl_slist* list = (curl_slist*)t.ptr;
    list = curl_slist_append(list,val);
    zclassobj_set(obj,".handle",zobj_from_ptr(list));

    return zobj_nil();
}
zobject slist__del__(zobject* args,int32_t n)
{
    if(n!=1)
        return z_err(ArgumentError,"slist.__construct__() takes exactly 1 argument!");
    zobject tmp = args[0];
    if(tmp.type != Z_OBJ || ((zclass_object*)tmp.ptr)->_klass != slist_class )
        return z_err(TypeError,"Argument 1 must be an object of slist class!");

    zclass_object* obj = ((zclass_object*)tmp.ptr);
    zobject t = zclassobj_get(obj,".handle");
    curl_slist* list = (curl_slist*)t.ptr;

    zclassobj_set(obj,".handle",zobj_from_ptr(NULL));
    curl_slist_free_all(list);

    return zobj_nil();
}
