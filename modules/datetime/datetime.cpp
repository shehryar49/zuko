/*
Datetime Module Plutonium
Written by Shahryar Ahmad 
5 March 2023
The code is completely free to use and comes without any guarantee/warrantee
*/
#include "datetime.h"
#include <time.h>

Klass* tmklass;
ZObject init()
{
    Module* d = vm_allocModule();
    Module_addNativeFun(d,"time",&TIME);
    Module_addNativeFun(d,"ctime",&CTIME);
    tmklass = vm_allocKlass(); //kinda like the tm_struct
    tmklass->name = "tm";
    // not actually methods of tmklass, but the following functions use tmklass
    // 
    Module_addMember(d,"localtime",ZObjFromMethod("localtime",&LOCALTIME,tmklass));
    Module_addMember(d,"gmtime",ZObjFromMethod("gmtime",&GMTIME,tmklass));

    return ZObjFromModule(d);
}
ZObject TIME(ZObject* args,int32_t n)
{
    if(n!=0)
      return Z_Err(ArgumentError,"0 arguments required!");
    time_t now = time(NULL);
    //time_t is typedef for long
    //so time_t can fit in int64_t
    ZObject ret;
    ret.type = Z_INT64;
    ret.l = now;
    return ret;
}
ZObject CTIME(ZObject* args,int32_t n)
{
    if(n!=1)
      return Z_Err(ArgumentError,"1 argument required!");
    if(args[0].type != Z_INT64)
      return Z_Err(TypeError,"Argument must be an int64!");
    time_t now = (time_t)args[0].l;
    char* tm = ctime(&now);
    ZStr* s = vm_allocString(strlen(tm));
    //strings in zuko are immutable
    //never do the following with strings that you do not allocate
    strcpy(s->val,tm);
    return ZObjFromStrPtr(s);
}
ZObject LOCALTIME(ZObject* args,int32_t n)
{
    if(n!=1)
      return Z_Err(ArgumentError,"1 argument required!");
    if(args[0].type != Z_INT64)
      return Z_Err(TypeError,"Argument must be an int64!");
    time_t now = (time_t)args[0].l;
    tm* time = localtime(&now);
    if(!time)
      return Z_Err(Error,"C localtime() returned nullptr");
    KlassObject* ki = vm_allocKlassObject(tmklass);
    ki->klass = tmklass;
    #ifdef _WIN32
      //Fuck microsoft
    #else
      KlassObj_setMember(ki,"gmtoff",ZObjFromInt64(time->tm_gmtoff));
      KlassObj_setMember(ki,"zone", ZObjFromStr(time->tm_zone));
    #endif

    KlassObj_setMember(ki,"hour",ZObjFromInt(time->tm_hour));
    KlassObj_setMember(ki,"isdst",ZObjFromInt(time->tm_isdst));
    KlassObj_setMember(ki,"mday",ZObjFromInt(time->tm_mday));
    KlassObj_setMember(ki,"min",ZObjFromInt(time->tm_min));
    KlassObj_setMember(ki,"mon",ZObjFromInt(time->tm_mon));
    KlassObj_setMember(ki,"sec",ZObjFromInt(time->tm_sec));
    KlassObj_setMember(ki,"wday",ZObjFromInt(time->tm_wday));
    KlassObj_setMember(ki,"yday",ZObjFromInt(time->tm_yday));

    return ZObjFromKlassObj(ki);
}
ZObject GMTIME(ZObject* args,int32_t n)
{
    if(n!=1)
      return Z_Err(ArgumentError,"1 argument required!");
    if(args[0].type != Z_INT64)
      return Z_Err(TypeError,"Argument must be an int64!");
    time_t now = (time_t)args[0].l;
    tm* time = localtime(&now);
    if(!time)
      return Z_Err(Error,"C localtime() returned nullptr");
    KlassObject* ki = vm_allocKlassObject(tmklass);
    ki->klass = tmklass;
    #ifdef _WIN32
      //Fuck microsoft
    #else
      KlassObj_setMember(ki,"gmtoff", ZObjFromInt64(time->tm_gmtoff));
      KlassObj_setMember(ki,"zone", ZObjFromStr(time->tm_zone));
    #endif
    
    KlassObj_setMember(ki,"hour",ZObjFromInt(time->tm_hour));
    KlassObj_setMember(ki,"isdst",ZObjFromInt(time->tm_isdst));
    KlassObj_setMember(ki,"mday",ZObjFromInt(time->tm_mday));
    KlassObj_setMember(ki,"min",ZObjFromInt(time->tm_min));
    KlassObj_setMember(ki,"mon",ZObjFromInt(time->tm_mon));
    KlassObj_setMember(ki,"sec",ZObjFromInt(time->tm_sec));
    KlassObj_setMember(ki,"wday",ZObjFromInt(time->tm_wday));
    KlassObj_setMember(ki,"yday",ZObjFromInt(time->tm_yday));
    return ZObjFromKlassObj(ki);
}

