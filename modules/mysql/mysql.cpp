//Mysql connector module for Plutonium
//Written by Shahryar Ahmad  15/5/2021
//This module uses mariadb C connector and does not change the original
//connector's source code.
#include "mysql.h"
#include <string.h>
#include <stdlib.h>
#include <mysql.h> //you can change this to path where
//your libmariadb-dev headers are placed

using namespace std;
int TypeMistmatch(string pattern,PltObject* args,int n)
{
  int k = 0;
  for(int k=0;k<pattern.length();k+=1)
  {
    if(pattern[k]!=args[k].type)
      return k;//argument number k is causing a type mismatch
  }
  return -1;//all good
}
PltObject nil;
Klass* connKlass;
Klass* resKlass;
NativeFunction* p1;
NativeFunction* p2;
PltObject init()
{
  nil.type = PLT_NIL;
  Module* d = vm_allocModule();
  connKlass = vm_allocKlass();
  resKlass = vm_allocKlass();
  connKlass->name = "mysql.connection";

  connKlass->members.emplace("__del__",PObjFromFunction("__del__",&CONN__DEL__,connKlass));
  resKlass->name = "mysql.result";
  resKlass->members.emplace("__del__",PObjFromFunction("__del__",&RES__DEL__,resKlass));
  p1 = (NativeFunction*)resKlass->members["__del__"].ptr;
  p2 = (NativeFunction*)connKlass->members["__del__"].ptr;
  
  //mark both classes important so VM does not garbage collect them
  //these classes will be used by the functions,so they should remain alive during
  //the life of the module
  vm_markImportant(connKlass);
  vm_markImportant(resKlass);
  //and their deletors as well
  vm_markImportant(p1);
  vm_markImportant(p2);
  d->members.emplace("init",PObjFromFunction("mysql.init",&INIT));
  d->members.emplace("real_connect",PObjFromFunction("mysql.real_connect",&REAL_CONNECT));
  d->members.emplace("query",PObjFromFunction("mysql.query",&QUERY));  
  d->members.emplace("store_result",PObjFromFunction("mysql.store_result",&STORE_RESULT));
  d->members.emplace("fetch_row",PObjFromFunction("mysql.fetch_row",&FETCH_ROW));
  d->members.emplace("fetch_row_as_str",PObjFromFunction("mysql.fetch_row_as_str",&FETCH_ROW_AS_STR));
  d->members.emplace("num_fields",PObjFromFunction("mysql.num_fields",&NUM_FIELDS));
  d->members.emplace("num_rows",PObjFromFunction("mysql.nums_rows",&NUM_ROWS));  
  d->members.emplace("close",PObjFromFunction("mysql.close",&CLOSE));
    
  return PObjFromModule(d);
}
PltObject INIT(PltObject* args,int32_t n)
{
  if(n!=0)
    return Plt_Err(ArgumentError,"0 arguments needed!");
  MYSQL* conn = new MYSQL;
  mysql_init(conn);
  if(!conn)
    return Plt_Err(Error,"Initialization failed.");
  KlassInstance* ki = vm_allocKlassInstance();
  ki->klass = connKlass;
  ki->members = connKlass->members;
  ki->members.emplace(".ptr",PObjFromPtr(conn));
  return PObjFromKlassInst(ki);
}
PltObject REAL_CONNECT(PltObject* args,int n)
{
  if(n!=5 && n!=6)
    return Plt_Err(ValueError,"Either 5 or 6 arguments needed!");
  int k;
  int port;
  if(n==5)
    k = TypeMistmatch("ossss",args,n);
  else
  {
    k = TypeMistmatch("ossssi",args,n);
    port = args[5].i;
  }
  if(k!=-1)
    return Plt_Err(TypeError,"Invalid type of argument "+to_string(k));
  KlassInstance* ki = (KlassInstance*)args[0].ptr;
  if(ki->members[".ptr"].type == PLT_NIL)
    return Plt_Err(ValueError,"Connection is closed.");
  if(ki->klass!=connKlass)
    return Plt_Err(TypeError,"Argument 1 must be a connection object!");
  MYSQL* conn = (MYSQL*)ki->members[".ptr"].ptr;
  string& host = *(string*)args[1].ptr;
  if(n==5)
  {
    if(!mysql_real_connect(conn, host.c_str(),((string*)args[2].ptr)->c_str(), ((string*)args[3].ptr)->c_str(),((string*)args[4].ptr)->c_str(),0,NULL,0)) {
      return Plt_Err(Error,mysql_error(conn));
    }
  }
  else
  {
      if(!mysql_real_connect(conn, host.c_str(),((string*)args[2].ptr)->c_str(), ((string*)args[3].ptr)->c_str(),((string*)args[4].ptr)->c_str(),port,NULL,0)) {
      return Plt_Err(Error,mysql_error(conn));
    }
  }
  return nil;
}
PltObject QUERY(PltObject* args,int n)
{
  if(n!=2)
    return Plt_Err(ValueError,"2 arguments needed!");
  if(args[0].type!='o' || ((KlassInstance*)args[0].ptr)->klass!=connKlass)
    return Plt_Err(TypeError,"First argument should be a connection object!");
  if(args[1].type!='s')
    return Plt_Err(TypeError,"Second argument should be a string!");
  KlassInstance* ki = (KlassInstance*)args[0].ptr;
  if(ki->members[".ptr"].type == PLT_NIL)
    return Plt_Err(ValueError,"Connection is closed.");
  MYSQL* conn = (MYSQL*)ki->members[".ptr"].ptr;
  string& query = *(string*)args[1].ptr;
  if(mysql_query(conn,query.c_str())!=0)
      return Plt_Err(Error,mysql_error(conn));
  return nil;
}

PltObject STORE_RESULT(PltObject* args,int n)
{
  if(n!=1)
    return Plt_Err(ValueError,"1 arguments needed!");
  if(args[0].type!='o' || ((KlassInstance*)args[0].ptr)->klass!=connKlass)
    return Plt_Err(TypeError,"First argument should be a connection object!");
  KlassInstance* ki = (KlassInstance*)args[0].ptr;
  if(ki->members[".ptr"].type == PLT_NIL)
    return Plt_Err(ValueError,"Connection is closed.");
  MYSQL* conn = (MYSQL*)ki->members[".ptr"].ptr;
  MYSQL_RES* res = mysql_store_result(conn);
  if(!res)//no result
    return nil;
  KlassInstance* obj = vm_allocKlassInstance();
  obj->klass = resKlass;
  obj->members = resKlass->members;  
  obj->members.emplace(".ptr",PObjFromPtr(res));
  return PObjFromKlassInst(obj);  
}
PltObject FETCH_ROW(PltObject* args,int n)
{
  if(n!=1)
    return Plt_Err(ValueError,"1 argument needed!");
  if(args[0].type!='o' || ((KlassInstance*)args[0].ptr)->klass!=resKlass)
    return Plt_Err(TypeError,"First argument should be a result object!");
  KlassInstance* ki = (KlassInstance*)args[0].ptr;
  if(ki->members[".ptr"].type == PLT_NIL)
    return Plt_Err(ValueError,"Connection is closed.");
  MYSQL_RES* res = (MYSQL_RES*)ki->members[".ptr"].ptr;
 // int totalrows = mysql_num_rows(res);
  int numfields = mysql_num_fields(res);
  MYSQL_ROW row;
  PltList* p = vm_allocList();
  if(row = mysql_fetch_row(res))
  {
    long unsigned int* lengths = mysql_fetch_lengths(res);
    for(size_t i = 0; i < numfields; i++)
    {
      if(!row[i])
        p->push_back(nil);
      else
      {
        auto btArr = vm_allocByteArray();
        btArr->resize(lengths[i]);
        memcpy(&btArr->at(0),row[i],lengths[i]);
        p->push_back(PObjFromByteArr(btArr));
      }
    }
    return PObjFromList(p);
  }
  else
    return nil;
}
PltObject NUM_FIELDS(PltObject* args,int n)
{
  if(n!=1)
    return Plt_Err(ValueError,"1 argument needed!");
  if(args[0].type!='o' || ((KlassInstance*)args[0].ptr)->klass!=resKlass)
    return Plt_Err(TypeError,"First argument should be a result object!");
  KlassInstance* ki = (KlassInstance*)args[0].ptr;
  MYSQL_RES* res = (MYSQL_RES*)ki->members[".ptr"].ptr;
 // int totalrows = mysql_num_rows(res);
  int numfields = mysql_num_fields(res);
  return PObjFromInt(numfields);
}
PltObject NUM_ROWS(PltObject* args,int n)
{
  if(n!=1)
    return Plt_Err(ValueError,"1 argument needed!");
  if(args[0].type!='o' || ((KlassInstance*)args[0].ptr)->klass!=resKlass)
    return Plt_Err(TypeError,"First argument should be a result object!");
  KlassInstance* ki = (KlassInstance*)args[0].ptr;
  MYSQL_RES* res = (MYSQL_RES*)ki->members[".ptr"].ptr;
 // int totalrows = mysql_num_rows(res);
  int numrows = mysql_num_rows(res);
  return PObjFromInt(numrows);
}
PltObject FETCH_ROW_AS_STR(PltObject* args,int n)
{
  if(n!=1)
    return Plt_Err(ValueError,"1 argument needed!");
  if(args[0].type!='o' || ((KlassInstance*)args[0].ptr)->klass!=resKlass)
    return Plt_Err(TypeError,"First argument should be a result object!");
  KlassInstance* ki = (KlassInstance*)args[0].ptr;
  if(ki->members[".ptr"].type == PLT_NIL)
    return Plt_Err(ValueError,"Connection is closed.");
  MYSQL_RES* res = (MYSQL_RES*)ki->members[".ptr"].ptr;
  int totalrows = mysql_num_rows(res);
  int numfields = mysql_num_fields(res);
  MYSQL_ROW row;
  PltList* p = vm_allocList();
  if(row = mysql_fetch_row(res))
  {
    long unsigned int* lengths = mysql_fetch_lengths(res);
    for(size_t i = 0; i < numfields; i++)
    {
      if(!row[i])
        p->push_back(nil);
      else
      {
        auto str = vm_allocString();
        if(lengths[i]!=0)
        {
          str->resize(lengths[i]);
          memcpy(&str->at(0),row[i],lengths[i]);
        }
        p->push_back(PObjFromStrPtr(str));
      }
    }
    return PObjFromList(p);
  }
  else
    return nil;
}
PltObject CLOSE(PltObject* args,int n)
{
  if(n!=1)
    return Plt_Err(ValueError,"1 arguments needed!");
  if(args[0].type!='o' || ((KlassInstance*)args[0].ptr)->klass!=connKlass)
    return Plt_Err(TypeError,"First argument should be a connection object!");
  KlassInstance* ki = (KlassInstance*)args[0].ptr;
  if(ki->members[".ptr"].type == PLT_NIL)
    return Plt_Err(ValueError,"Connection already closed.");
  if(ki->members[".ptr"] == nil)
    return nil;
  MYSQL* conn = (MYSQL*)ki->members[".ptr"].ptr;
  mysql_close(conn);
  delete conn;
  ki->members[".ptr"] = nil;
  return nil;
}
PltObject CONN__DEL__(PltObject* args,int n)
{
  if(n!=1)
    return Plt_Err(ValueError,"1 arguments needed!");
  if(args[0].type!='o' || ((KlassInstance*)args[0].ptr)->klass!=connKlass)
    return Plt_Err(TypeError,"First argument should be a connection object!");
  KlassInstance* ki = (KlassInstance*)args[0].ptr;
  if(ki->members[".ptr"] == nil)
    return nil;
  MYSQL* conn = (MYSQL*)ki->members[".ptr"].ptr;
  mysql_close(conn);
  delete conn;
  ki->members[".ptr"] = nil;
  return nil;
}
PltObject RES__DEL__(PltObject* args,int n)
{
  if(n!=1)
    return Plt_Err(ValueError,"1 arguments needed!");
  if(args[0].type!='o' || ((KlassInstance*)args[0].ptr)->klass!=resKlass)
    return Plt_Err(TypeError,"First argument should be a result object!");
  KlassInstance* ki = (KlassInstance*)args[0].ptr;
  if(ki->members[".ptr"] == nil)
    return nil;
  MYSQL_RES* res = (MYSQL_RES*)ki->members[".ptr"].ptr;
  mysql_free_result(res);
  ki->members[".ptr"] = nil;
  return nil;
}
void unload()
{
  vm_unmarkImportant(connKlass);
  vm_unmarkImportant(resKlass);
  vm_unmarkImportant(p1);
  vm_unmarkImportant(p2);
}
