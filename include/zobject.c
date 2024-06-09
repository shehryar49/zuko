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
#include "zstr.h"
#include <stdbool.h>

bool zobject_equals(zobject lhs,zobject other)
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
    zstr* a = (zstr*)lhs.ptr;
    zstr* b = (zstr*)other.ptr;
    return (a->len == b->len) && (strcmp(a->val,b->val) == 0);
  }
  else if(lhs.type==Z_LIST)
  {
    zlist* a = (zlist*)lhs.ptr;
    zlist* b = (zlist*)other.ptr;
    if(a == b)
      return true;
    if(a->size != b->size)
      return false;
    for(size_t i=0;i<a->size;i++)
    {
        if(!(zobject_equals(a->arr[i],b->arr[i])))
          return false;
    }
    return true;
  }
  else if(lhs.type == Z_BYTEARR)
  {
    zbytearr* a = (zbytearr*)lhs.ptr;
    zbytearr* b = (zbytearr*)other.ptr;
    return a == b || zbytearr_equal(a,b);
  }
  else if(other.type=='y' || other.type=='r' || other.type == Z_CLASS || other.type==Z_POINTER)
    return lhs.ptr==other.ptr;
  else if(lhs.type=='q' || lhs.type=='z')
    return lhs.ptr==other.ptr;
  else if(lhs.type==Z_DICT)//IMPORTANT
     return lhs.ptr == other.ptr || zdict_equal((zdict*)lhs.ptr,(zdict*)other.ptr);
  return false;
}
//Helper functions

zobject zobj_from_str_ptr(zstr* s)
{
  zobject ret;
  ret.type = 's';
  ret.ptr = (void*)s;
  return ret;
}
zobject zobj_from_int(int32_t x)
{
  zobject ret;
  ret.type = 'i';
  ret.i = x;
  return ret;
}
zobject zobj_from_double(double f)
{
  zobject ret;
  ret.type = 'f';
  ret.f = f;
  return ret;
}
zobject zobj_from_int64(int64_t x)
{
  zobject ret;
  ret.type = 'l';
  ret.l = x;
  return ret;
}
zobject zobj_from_ptr(void* p)
{
  zobject ret;
  ret.type = 'p';
  ret.ptr = p;
  return ret;
}
zobject zobj_from_byte(uint8_t x)
{
  zobject ret;
  ret.type = 'm';
  ret.i = x;
  return ret;
}
zobject zobj_from_bool(bool b)
{
  zobject ret;
  ret.type = Z_BOOL;
  ret.i = b;
  return ret;
}

zobject zobj_from_list(zlist* l)
{
  zobject ret;
  ret.type = Z_LIST;
  ret.ptr = (void*)l;
  return ret;
}
zobject zobj_from_dict(zdict* l)
{
  zobject ret;
  ret.type = Z_DICT;
  ret.ptr = (void*)l;
  return ret;
}
zobject zobj_from_module(zmodule* k)
{
  zobject ret;
  ret.type = Z_MODULE;
  ret.ptr = (void*)k;
  return ret;
}
zobject zobj_from_class(zclass* k)
{
  zobject ret;
  ret.type = Z_CLASS;
  ret.ptr = (void*)k;
  return ret;
}
zobject zobj_from_classobj(zclass_object* ki)
{
  zobject ret;
  ret.type = Z_OBJ;
  ret.ptr = (void*)ki;
  return ret;
}
zobject zobj_from_bytearr(zbytearr* k)
{
  zobject ret;
  ret.type = Z_BYTEARR;
  ret.ptr = (void*)k;
  return ret;
}
zobject zobj_from_file(zfile* file)
{
  zobject ret;
  ret.type = Z_FILESTREAM;
  ret.ptr = (void*)file;
  return ret;
}

