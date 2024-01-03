/*MIT License

Copyright (c) 2022 Shahryar Ahmad 

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/
#ifndef ZOBJECT_H_
#define ZOBJECT_H_

#include <stdint.h>
#include "zfileobj.h"
#include <string>
#include <vector>

using namespace std;

#define Z_LIST 'j'
#define Z_DICT 'a'
#define Z_INT 'i'
#define Z_INT64 'l'
#define Z_FLOAT 'f'
#define Z_BYTE  'm'
#define Z_NATIVE_FUNC 'y'//native function
#define Z_MODULE 'q'
#define Z_STR 's'
#define Z_MSTR 'k' //mutable strings
#define Z_FILESTREAM 'u'
#define Z_NIL 'n'
#define Z_OBJ 'o' //objects created using zuko code
#define Z_CLASS 'v' //Zuko code class
#define Z_BOOL 'b'
#define Z_POINTER 'p' //just a pointer that means something only to c++ code,the interpreter just stores it
#define Z_FUNC 'w' //zuko code function
#define Z_COROUTINE 'g'
#define Z_COROUTINE_OBJ 'z'
#define Z_ERROBJ 'e' //same as an object,just in thrown state
#define Z_BYTEARR 'c'
#define Z_RAW 't' //used by VM's GC
extern "C" struct ZObject
{
    union
    {
        double f;
        int64_t l;
        int32_t i;
        void* ptr;
    };
    char type;
};
bool ZObject_equals(const ZObject&,const ZObject&);
#include "zlist.h"
#include "zdict.h"

bool ZObject_equals(const ZObject& lhs,const ZObject& other)
{
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
    return *(string*)other.ptr==*(string*)lhs.ptr;
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
        if(!(ZObject_equals(a->arr[i],b->arr[i])))
          return false;
    }
    return true;
  }
  else if(lhs.type == Z_BYTEARR)
    return *(vector<uint8_t>*)lhs.ptr == *(vector<uint8_t>*)other.ptr;
  else if(other.type=='y' || other.type=='r' || other.type == Z_CLASS)
    return lhs.ptr==other.ptr;
  else if(lhs.type=='q' || lhs.type=='z')
    return lhs.ptr==other.ptr;
  else if(lhs.type==Z_DICT)//IMPORTANT
      return lhs.ptr == other.ptr;//*(ZDict*)lhs.ptr==*(ZDict*)other.ptr;
  return false;
}
#endif
