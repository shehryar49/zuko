#include "zobject.h"
#include "klassobject.h"
#include "zlist.h"
#include "zfileobj.h"
#include "module.h"
#include "klass.h"
#include "klassobject.h"
#include "zbytearray.h"
#include "zdict.h"
#include "nativefun.h"
#include <stdbool.h>
bool ZObject_equals(ZObject lhs,ZObject other)
{
  if(lhs.type == 'i' && other.type == 'l')
    return (int64_t)lhs.i == other.l;
  if(lhs.type == 'l' && other.type=='i')
    return lhs.l == (int64_t)other.i;
  if(other.type!=lhs.type)
      return false;
  if(lhs.type==Z_NIL)
    return true;
  else if(lhs.type==Z_INT)
    return lhs.i==other.i;
  else if(lhs.type==Z_INT64)
    return lhs.l==other.l;
  else if(lhs.type==Z_FLOAT)
    return lhs.f==other.f;
  else if(lhs.type==Z_BYTE)
    return lhs.i==other.i;
  else if(lhs.type==Z_BOOL)
    return lhs.i==other.i;
  else if(lhs.type==Z_FILESTREAM)
    return ((zfile*)lhs.ptr)==((zfile*)other.ptr);
  else if(lhs.type==Z_STR)
  {
    ZStr* a = (ZStr*)lhs.ptr;
    ZStr* b = (ZStr*)other.ptr;
    return (a->len == b->len) && (strcmp(a->val,b->val) == 0);
  }
  else if(lhs.type==Z_LIST)
  {
    ZList* a = (ZList*)lhs.ptr;
    ZList* b = (ZList*)other.ptr;
    if(a == b)
      return true;
    if(a->size != b->size)
      return false;
    for(size_t i=0;i<a->size;i++)
    {
        if(!(ZObject_equals(a->arr[i],b->arr[i])))
          return false;
    }
    return true;
  }
  else if(lhs.type == Z_BYTEARR)
  {
    ZByteArr* a = (ZByteArr*)lhs.ptr;
    ZByteArr* b = (ZByteArr*)other.ptr;
    return a == b || ZByteArr_equal(a,b);
  }
  else if(other.type=='y' || other.type=='r' || other.type == Z_CLASS || other.type==Z_POINTER)
    return lhs.ptr==other.ptr;
  else if(lhs.type=='q' || lhs.type=='z')
    return lhs.ptr==other.ptr;
  else if(lhs.type==Z_DICT)//IMPORTANT
     return lhs.ptr == other.ptr || ZDict_equal((ZDict*)lhs.ptr,(ZDict*)other.ptr);
  return false;
}
//Helper functions

ZObject ZObjFromStrPtr(ZStr* s)
{
  ZObject ret;
  ret.type = 's';
  ret.ptr = (void*)s;
  return ret;
}
ZObject ZObjFromInt(int32_t x)
{
  ZObject ret;
  ret.type = 'i';
  ret.i = x;
  return ret;
}
ZObject ZObjFromDouble(double f)
{
  ZObject ret;
  ret.type = 'f';
  ret.f = f;
  return ret;
}
ZObject ZObjFromInt64(int64_t x)
{
  ZObject ret;
  ret.type = 'l';
  ret.l = x;
  return ret;
}
ZObject ZObjFromPtr(void* p)
{
  ZObject ret;
  ret.type = 'p';
  ret.ptr = p;
  return ret;
}
ZObject ZObjFromByte(uint8_t x)
{
  ZObject ret;
  ret.type = 'm';
  ret.i = x;
  return ret;
}
ZObject ZObjFromBool(bool b)
{
  ZObject ret;
  ret.type = Z_BOOL;
  ret.i = b;
  return ret;
}

ZObject ZObjFromList(ZList* l)
{
  ZObject ret;
  ret.type = Z_LIST;
  ret.ptr = (void*)l;
  return ret;
}
ZObject ZObjFromDict(ZDict* l)
{
  ZObject ret;
  ret.type = Z_DICT;
  ret.ptr = (void*)l;
  return ret;
}
ZObject ZObjFromModule(Module* k)
{
  ZObject ret;
  ret.type = Z_MODULE;
  ret.ptr = (void*)k;
  return ret;
}
ZObject ZObjFromKlass(Klass* k)
{
  ZObject ret;
  ret.type = Z_CLASS;
  ret.ptr = (void*)k;
  return ret;
}
ZObject ZObjFromKlassObj(KlassObject* ki)
{
  ZObject ret;
  ret.type = Z_OBJ;
  ret.ptr = (void*)ki;
  return ret;
}
ZObject ZObjFromByteArr(ZByteArr* k)
{
  ZObject ret;
  ret.type = Z_BYTEARR;
  ret.ptr = (void*)k;
  return ret;
}
ZObject ZObjFromFile(zfile* file)
{
  ZObject ret;
  ret.type = Z_FILESTREAM;
  ret.ptr = (void*)file;
  return ret;
}

