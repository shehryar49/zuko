/*
Datetime Module Plutonium
Written by Shahryar Ahmad 
5 March 2023
The code is completely free to use and comes without any guarantee/warrantee
*/
#include "datetime.h"
#include "zobject.h"
#include <time.h>

zclass* tmklass;
zobject init()
{
    zmodule* d = vm_allocModule();
    Module_addNativeFun(d,"time",&TIME);
    Module_addNativeFun(d,"ctime",&CTIME);
    tmklass = vm_allocklass(); //kinda like the tm_struct
    tmklass->name = "tm";
    // not actually methods of tmklass, but the following functions use tmklass
    // 
    zmodule_add_member(d,"localtime",zobj_from_method("localtime",&LOCALTIME,tmklass));
    zmodule_add_member(d,"gmtime",zobj_from_method("gmtime",&GMTIME,tmklass));

    return zobj_from_module(d);
}
zobject TIME(zobject* args,int32_t n)
{
    if(n!=0)
      return Z_Err(ArgumentError,"0 arguments required!");
    time_t now = time(NULL);
    //time_t is typedef for long
    //so time_t can fit in int64_t
    zobject ret;
    ret.type = Z_INT64;
    ret.l = now;
    return ret;
}
zobject CTIME(zobject* args,int32_t n)
{
    if(n!=1)
      return Z_Err(ArgumentError,"1 argument required!");
    if(args[0].type != Z_INT64)
      return Z_Err(TypeError,"Argument must be an int64!");
    time_t now = (time_t)args[0].l;
    char* tm = ctime(&now);
    zstr* s = vm_allocString(strlen(tm));
    //strings in zuko are immutable
    //never do the following with strings that you do not allocate
    strcpy(s->val,tm);
    return zobj_from_str_ptr(s);
}
zobject LOCALTIME(zobject* args,int32_t n)
{
    if(n!=1)
      return Z_Err(ArgumentError,"1 argument required!");
    if(args[0].type != Z_INT64)
      return Z_Err(TypeError,"Argument must be an int64!");
    time_t now = (time_t)args[0].l;
    tm* time = localtime(&now);
    if(!time)
      return Z_Err(Error,"C localtime() returned nullptr");
    zclass_object* ki = vm_allocklassObject(tmklass);
    ki->_klass = tmklass;
    #ifdef _WIN32
      //Fuck microsoft
    #else
      zclassobj_set(ki,"gmtoff",zobj_from_int64(time->tm_gmtoff));
      zclassobj_set(ki,"zone", zobj_from_str(time->tm_zone));
    #endif

    zclassobj_set(ki,"hour",zobj_from_int(time->tm_hour));
    zclassobj_set(ki,"isdst",zobj_from_int(time->tm_isdst));
    zclassobj_set(ki,"mday",zobj_from_int(time->tm_mday));
    zclassobj_set(ki,"min",zobj_from_int(time->tm_min));
    zclassobj_set(ki,"mon",zobj_from_int(time->tm_mon));
    zclassobj_set(ki,"sec",zobj_from_int(time->tm_sec));
    zclassobj_set(ki,"wday",zobj_from_int(time->tm_wday));
    zclassobj_set(ki,"yday",zobj_from_int(time->tm_yday));

    return zobj_from_classobj(ki);
}
zobject GMTIME(zobject* args,int32_t n)
{
    if(n!=1)
      return Z_Err(ArgumentError,"1 argument required!");
    if(args[0].type != Z_INT64)
      return Z_Err(TypeError,"Argument must be an int64!");
    time_t now = (time_t)args[0].l;
    tm* time = localtime(&now);
    if(!time)
      return Z_Err(Error,"C localtime() returned nullptr");
    zclass_object* ki = vm_allocklassObject(tmklass);
    ki->_klass = tmklass;
    #ifdef _WIN32
      //Fuck microsoft
    #else
      zclassobj_set(ki,"gmtoff", zobj_from_int64(time->tm_gmtoff));
      zclassobj_set(ki,"zone", zobj_from_str(time->tm_zone));
    #endif
    
    zclassobj_set(ki,"hour",zobj_from_int(time->tm_hour));
    zclassobj_set(ki,"isdst",zobj_from_int(time->tm_isdst));
    zclassobj_set(ki,"mday",zobj_from_int(time->tm_mday));
    zclassobj_set(ki,"min",zobj_from_int(time->tm_min));
    zclassobj_set(ki,"mon",zobj_from_int(time->tm_mon));
    zclassobj_set(ki,"sec",zobj_from_int(time->tm_sec));
    zclassobj_set(ki,"wday",zobj_from_int(time->tm_wday));
    zclassobj_set(ki,"yday",zobj_from_int(time->tm_yday));
    return zobj_from_classobj(ki);
}

