//Mysql connector module for Plutonium
//Written by Shahryar Ahmad  15/5/2021
//This module uses mariadb C connector and does not change the original
//connector's source code.
#include "mariadb.h"
#include <string>
#include <stdlib.h>
#include <mysql.h> //you can change this to path where
//your libmariadb-dev headers are placed

using namespace std;
int TypeMistmatch(string pattern,ZObject* args,int n)
{
  int k = 0;
  for(int k=0;k<pattern.length();k+=1)
  {
    if(pattern[k]!=args[k].type)
      return k;//argument number k is causing a type mismatch
  }
  return -1;//all good
}

ZObject nil;
Klass* connKlass;
Klass* resKlass;

ZObject quickErr(Klass* e,const string& msg)
{
  return Z_Err(e,msg.c_str());
}
ZObject init()
{
  nil.type = Z_NIL;
  Module* d = vm_allocModule();
  
  connKlass = vm_allocKlass();
  connKlass->name = "mysql.connection";
 
  Klass_addNativeMethod(connKlass,"query",&CONN__QUERY);
  Klass_addNativeMethod(connKlass,"store_result",&CONN__STORE_RESULT);
  Klass_addNativeMethod(connKlass,"__del__",&CONN__DEL__);
  Klass_addNativeMethod(connKlass,"close",&CONN__CLOSE);
  
  
  resKlass = vm_allocKlass();
  resKlass->name = "mysql.result";
  Klass_addNativeMethod(resKlass,"fetch_row",&RES__FETCH_ROW);
  Klass_addNativeMethod(resKlass,"num_rows",&RES__NUM_ROWS);
  Klass_addNativeMethod(resKlass,"num_fields",&RES__NUM_FIELDS);
  Klass_addNativeMethod(resKlass,"__del__",&RES__DEL__);
  
  
  //mark both classes important so VM does not garbage collect them
  //these classes will be used by the functions,so they should remain alive during
  //the life of the module
  vm_markImportant(connKlass);
  vm_markImportant(resKlass);

  Module_addNativeFun(d,"real_connect",&REAL_CONNECT);

  return ZObjFromModule(d);
}

ZObject REAL_CONNECT(ZObject* args,int n)
{
  if(n!=4 && n!=5)
    return Z_Err(ValueError,"Either 5 or 6 arguments needed!");
  int k;
  int port;
  if(n==4)
    k = TypeMistmatch("ssss",args,n);
  else
  {
    k = TypeMistmatch("ssssi",args,n);
    port = args[4].i;
  }
  if(k!=-1)
    return quickErr(TypeError,"Invalid type of argument "+to_string(k+1));
  
  MYSQL* conn = new MYSQL;
  mysql_init(conn);
  if(!conn)
    return Z_Err(Error,"Initialization failed.");
  KlassObject* ki = vm_allocKlassObject(connKlass);
  KlassObj_setMember(ki,".ptr",ZObjFromPtr((void*)conn));

  const char* host = AS_STR(args[0])->val;
  const char* username = AS_STR(args[1])->val;
  const char* password = AS_STR(args[2])->val;
  const char* dbname = AS_STR(args[3])->val;
  
  if(n==4)
  {
    if(!mysql_real_connect(conn, host,username,password,dbname,0,NULL,0)) {
      return Z_Err(Error,mysql_error(conn));
    }
  }
  else
  {
      if(!mysql_real_connect(conn, host,username,password,dbname,port,NULL,0)) {
      return Z_Err(Error,mysql_error(conn));
    }
  }
  return ZObjFromKlassObj(ki);
}
ZObject CONN__QUERY(ZObject* args,int n)
{
  if(n!=2)
    return Z_Err(ValueError,"2 arguments needed!");
  if(args[0].type!='o' || ((KlassObject*)args[0].ptr)->klass!=connKlass)
    return Z_Err(TypeError,"First argument should be a connection object!");
  if(args[1].type!='s')
    return Z_Err(TypeError,"Second argument should be a string!");
  KlassObject* ki = (KlassObject*)args[0].ptr;
  ZObject ptr = KlassObj_getMember(ki,".ptr");
  if(ptr.type == Z_NIL)
    return Z_Err(ValueError,"Connection is closed.");
  MYSQL* conn = (MYSQL*)ptr.ptr;
  const char* query = AS_STR(args[1])->val;;
  if(mysql_query(conn,query)!=0)
      return Z_Err(Error,mysql_error(conn));
  return nil;
}

ZObject CONN__STORE_RESULT(ZObject* args,int n)
{
  if(n!=1)
    return Z_Err(ValueError,"1 arguments needed!");
  if(args[0].type!='o' || ((KlassObject*)args[0].ptr)->klass!=connKlass)
    return Z_Err(TypeError,"First argument should be a connection object!");
  KlassObject* ki = (KlassObject*)args[0].ptr;
  ZObject ptr = KlassObj_getMember(ki,".ptr");
  if(ptr.type == Z_NIL)
    return Z_Err(ValueError,"Connection is closed.");
  MYSQL* conn = (MYSQL*)ptr.ptr;
  MYSQL_RES* res = mysql_store_result(conn);
  if(!res)//no result
    return nil;
  KlassObject* obj = vm_allocKlassObject(resKlass);
  
  KlassObj_setMember(obj,".ptr",ZObjFromPtr(res));
  
  return ZObjFromKlassObj(obj);  
}

ZObject CONN__CLOSE(ZObject* args,int n)
{
  if(n!=1)
    return Z_Err(ValueError,"1 arguments needed!");
  if(args[0].type!='o' || ((KlassObject*)args[0].ptr)->klass!=connKlass)
    return Z_Err(TypeError,"First argument should be a connection object!");
  KlassObject* ki = (KlassObject*)args[0].ptr;
  ZObject ptr = KlassObj_getMember(ki,".ptr");
  if(ptr.type == Z_NIL)
    return Z_Err(ValueError,"Connection already closed.");
  
  MYSQL* conn = (MYSQL*)ptr.ptr;
  mysql_close(conn);
  delete conn;
  KlassObj_setMember(ki,".ptr",nil);
  return nil;
}
ZObject CONN__DEL__(ZObject* args,int n)
{
  if(n!=1)
    return Z_Err(ValueError,"1 arguments needed!");
  if(args[0].type!='o' || ((KlassObject*)args[0].ptr)->klass!=connKlass)
    return Z_Err(TypeError,"First argument should be a connection object!");
  KlassObject* ki = (KlassObject*)args[0].ptr;
  ZObject ptr = KlassObj_getMember(ki,".ptr");
  if(ptr.type == Z_NIL)
    return nil;
  MYSQL* conn = (MYSQL*)ptr.ptr;
  mysql_close(conn);
  delete conn;
  KlassObj_setMember(ki,".ptr",nil);
  return nil;
}
/*ZObject FETCH_ROW(ZObject* args,int n)
{
  if(n!=1)
    return Z_Err(ValueError,"1 argument needed!");
  if(args[0].type!='o' || ((KlassObject*)args[0].ptr)->klass!=resKlass)
    return Z_Err(TypeError,"First argument should be a result object!");
  KlassObject* ki = (KlassObject*)args[0].ptr;
  if(ki->members[".ptr"].type == Z_NIL)
    return Z_Err(ValueError,"Connection is closed.");
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
        p->push_back(ZObjFromByteArr(btArr));
      }
    }
    return ZObjFromList(p);
  }
  else
    return nil;
}*/
ZObject RES__FETCH_ROW(ZObject* args,int n)
{
  if(n!=1)
    return Z_Err(ValueError,"1 argument needed!");
  if(args[0].type!='o' || ((KlassObject*)args[0].ptr)->klass!=resKlass)
    return Z_Err(TypeError,"First argument should be a result object!");
  KlassObject* ki = (KlassObject*)args[0].ptr;
  ZObject ptr = KlassObj_getMember(ki,".ptr");
  if(ptr.type == Z_NIL)
    return Z_Err(ValueError,"Result object invalid!");
    
  MYSQL_RES* res = (MYSQL_RES*)ptr.ptr;
  int totalrows = mysql_num_rows(res);
  int numfields = mysql_num_fields(res);
  MYSQL_ROW row;
  ZList* p = vm_allocList();
  if(row = mysql_fetch_row(res))
  {
    long unsigned int* lengths = mysql_fetch_lengths(res);
    for(size_t i = 0; i < numfields; i++)
    {
      if(!row[i])
        ZList_push(p,nil);
      else
      {
        auto str = vm_allocString(lengths[i]);
        if(lengths[i]!=0)
        {
          memcpy(str->val,row[i],lengths[i]);
        }
        ZList_push(p,ZObjFromStrPtr(str));
      }
    }
    return ZObjFromList(p);
  }
  else
    return nil;
}
ZObject RES__NUM_FIELDS(ZObject* args,int n)
{
  if(n!=1)
    return Z_Err(ValueError,"1 argument needed!");
  if(args[0].type!='o' || ((KlassObject*)args[0].ptr)->klass!=resKlass)
    return Z_Err(TypeError,"First argument should be a result object!");
  KlassObject* ki = (KlassObject*)args[0].ptr;
  ZObject ptr = KlassObj_getMember(ki,".ptr");
  if(ptr.type == Z_NIL)
    return Z_Err(Error,"Invalid result object!");
    
  MYSQL_RES* res = (MYSQL_RES*)ptr.ptr;
  int numfields = mysql_num_fields(res);
  return ZObjFromInt(numfields);
}
ZObject RES__NUM_ROWS(ZObject* args,int n)
{
  if(n!=1)
    return Z_Err(ValueError,"1 argument needed!");
  if(args[0].type!='o' || ((KlassObject*)args[0].ptr)->klass!=resKlass)
    return Z_Err(TypeError,"First argument should be a result object!");
  KlassObject* ki = (KlassObject*)args[0].ptr;
  ZObject ptr = KlassObj_getMember(ki,".ptr");
  if(ptr.type == Z_NIL)
    return Z_Err(Error,"Invalid result object!");
    
  MYSQL_RES* res = (MYSQL_RES*)ptr.ptr;
  int numrows = mysql_num_rows(res);
  return ZObjFromInt(numrows);
}
ZObject RES__DEL__(ZObject* args,int n)
{
  if(n!=1)
    return Z_Err(ValueError,"1 arguments needed!");
  if(args[0].type!='o' || ((KlassObject*)args[0].ptr)->klass!=resKlass)
    return Z_Err(TypeError,"First argument should be a result object!");
  KlassObject* ki = (KlassObject*)args[0].ptr;
  ZObject ptr = KlassObj_getMember(ki,".ptr");
  if(ptr.type == Z_NIL)
    return nil;
  MYSQL_RES* res = (MYSQL_RES*)ptr.ptr;
  mysql_free_result(res);
  KlassObj_setMember(ki,".ptr",nil);
  return nil;
}
void unload()
{
  vm_unmarkImportant(connKlass);
  vm_unmarkImportant(resKlass);
}
