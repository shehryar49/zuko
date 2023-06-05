/*
Datetime Module Plutonium
Written by Shahryar Ahmad 
5 March 2023
The code is completely free to use and comes without any guarantee/warrantee
*/
#include "datetime.h"
#include <time.h>
using namespace std;
Klass* tmklass;
PltObject init()
{
    Module* d = vm_allocModule();
    d->members.emplace("time",PObjFromFunction("datetime.time",&TIME));
    d->members.emplace("ctime",PObjFromFunction("datetime.ctime",&CTIME));
    tmklass = vm_allocKlass(); //kinda like the tm_struct
    tmklass->name = "tm";
    d->members.emplace("localtime",PObjFromFunction("datetime.localtime",&LOCALTIME,tmklass));
    d->members.emplace("gmtime",PObjFromFunction("datetime.gmtime",&GMTIME,tmklass));
    return PObjFromModule(d);
}
PltObject TIME(PltObject* args,int32_t n)
{
    if(n!=0)
      return Plt_Err(ARGUMENT_ERROR,"0 arguments required!");
    time_t now = time(NULL);
    //time_t is typedef for long
    //so time_t can fit in int64_t
    PltObject ret;
    ret.type = PLT_INT64;
    ret.l = now;
    return ret;
}
PltObject CTIME(PltObject* args,int32_t n)
{
    if(n!=1)
      return Plt_Err(ARGUMENT_ERROR,"1 argument required!");
    if(args[0].type != PLT_INT64)
      return Plt_Err(TYPE_ERROR,"Argument must be an int64!");
    time_t now = (time_t)args[0].l;
    string* s = vm_allocString();
    char* tm = ctime(&now);
    //strings in plutonium are immutable
    //never do the following with strings that you do not allocate
    *s = tm;
    return PObjFromStrPtr(s);
}
PltObject LOCALTIME(PltObject* args,int32_t n)
{
    if(n!=1)
      return Plt_Err(ARGUMENT_ERROR,"1 argument required!");
    if(args[0].type != PLT_INT64)
      return Plt_Err(TYPE_ERROR,"Argument must be an int64!");
    time_t now = (time_t)args[0].l;
    tm* time = localtime(&now);
    if(!time)
      return Plt_Err(UNKNOWN_ERROR,"C localtime() returned nullptr");
    KlassInstance* ki = vm_allocKlassInstance();
    ki->klass = tmklass;
    ki->members.emplace("gmtoff",PObjFromInt64(time->tm_gmtoff));
    ki->members.emplace("hour",PObjFromInt(time->tm_hour));
    ki->members.emplace("isdst",PObjFromInt(time->tm_isdst));
    ki->members.emplace("mday",PObjFromInt(time->tm_mday));
    ki->members.emplace("min",PObjFromInt(time->tm_min));
    ki->members.emplace("mon",PObjFromInt(time->tm_mon));
    ki->members.emplace("sec",PObjFromInt(time->tm_sec));
    ki->members.emplace("wday",PObjFromInt(time->tm_wday));
    ki->members.emplace("yday",PObjFromInt(time->tm_yday));
    ki->members.emplace("zone",PObjFromStr((std::string)time->tm_zone));
    return PObjFromKlassInst(ki);
}
PltObject GMTIME(PltObject* args,int32_t n)
{
    if(n!=1)
      return Plt_Err(ARGUMENT_ERROR,"1 argument required!");
    if(args[0].type != PLT_INT64)
      return Plt_Err(TYPE_ERROR,"Argument must be an int64!");
    time_t now = (time_t)args[0].l;
    tm* time = localtime(&now);
    if(!time)
      return Plt_Err(UNKNOWN_ERROR,"C localtime() returned nullptr");
    KlassInstance* ki = vm_allocKlassInstance();
    ki->klass = tmklass;
    ki->members.emplace("gmtoff",PObjFromInt64(time->tm_gmtoff));
    ki->members.emplace("hour",PObjFromInt(time->tm_hour));
    ki->members.emplace("isdst",PObjFromInt(time->tm_isdst));
    ki->members.emplace("mday",PObjFromInt(time->tm_mday));
    ki->members.emplace("min",PObjFromInt(time->tm_min));
    ki->members.emplace("mon",PObjFromInt(time->tm_mon));
    ki->members.emplace("sec",PObjFromInt(time->tm_sec));
    ki->members.emplace("wday",PObjFromInt(time->tm_wday));
    ki->members.emplace("yday",PObjFromInt(time->tm_yday));
    ki->members.emplace("zone",PObjFromStr((std::string)time->tm_zone));
    return PObjFromKlassInst(ki);
}

