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
    d->members.emplace("time",ZObjFromFunction("datetime.time",&TIME));
    d->members.emplace("ctime",ZObjFromFunction("datetime.ctime",&CTIME));
    tmklass = vm_allocKlass(); //kinda like the tm_struct
    tmklass->name = "tm";
    d->members.emplace("localtime",ZObjFromFunction("datetime.localtime",&LOCALTIME,tmklass));
    d->members.emplace("gmtime",ZObjFromFunction("datetime.gmtime",&GMTIME,tmklass));
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
    string* s = vm_allocString();
    char* tm = ctime(&now);
    //strings in plutonium are immutable
    //never do the following with strings that you do not allocate
    *s = tm;
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
    KlassObject* ki = vm_allocKlassObject();
    ki->klass = tmklass;
    #ifdef _WIN32
      //Fuck microsoft
    #else
      ki->members.emplace("gmtoff",ZObjFromInt64(time->tm_gmtoff));
      ki->members.emplace("zone", ZObjFromStr((std::string)time->tm_zone));
    #endif
    ki->members.emplace("hour",ZObjFromInt(time->tm_hour));
    ki->members.emplace("isdst",ZObjFromInt(time->tm_isdst));
    ki->members.emplace("mday",ZObjFromInt(time->tm_mday));
    ki->members.emplace("min",ZObjFromInt(time->tm_min));
    ki->members.emplace("mon",ZObjFromInt(time->tm_mon));
    ki->members.emplace("sec",ZObjFromInt(time->tm_sec));
    ki->members.emplace("wday",ZObjFromInt(time->tm_wday));
    ki->members.emplace("yday",ZObjFromInt(time->tm_yday));

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
    KlassObject* ki = vm_allocKlassObject();
    ki->klass = tmklass;
    #ifdef _WIN32
      //Fuck microsoft
    #else
      ki->members.emplace("gmtoff", ZObjFromInt64(time->tm_gmtoff));
      ki->members.emplace("zone", ZObjFromStr((std::string)time->tm_zone));
    #endif
    
    ki->members.emplace("hour",ZObjFromInt(time->tm_hour));
    ki->members.emplace("isdst",ZObjFromInt(time->tm_isdst));
    ki->members.emplace("mday",ZObjFromInt(time->tm_mday));
    ki->members.emplace("min",ZObjFromInt(time->tm_min));
    ki->members.emplace("mon",ZObjFromInt(time->tm_mon));
    ki->members.emplace("sec",ZObjFromInt(time->tm_sec));
    ki->members.emplace("wday",ZObjFromInt(time->tm_wday));
    ki->members.emplace("yday",ZObjFromInt(time->tm_yday));
    return ZObjFromKlassObj(ki);
}

