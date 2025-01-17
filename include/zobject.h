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

#include "zbytearray.h"
#include <stdint.h>
#include <stdbool.h>

#define Z_LIST 'j'
#define Z_DICT 'a'
#define Z_INT 'i'
#define Z_INT64 'l'
#define Z_FLOAT 'f'
#define Z_BYTE  'm'
#define Z_NATIVE_FUNC 'y'//native function
#define Z_MODULE 'q'
#define Z_STR 's'
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

#ifdef __cplusplus
extern "C"{
#endif

typedef struct zlist zlist;
typedef struct znativefun znativefun;

typedef struct zdict zdict;
typedef struct zfile zfile;
typedef struct zstr zstr;
typedef struct StrMap StrMap;
typedef struct zclass zclass;
typedef struct zclass_object zclass_object;
typedef struct zmodule zmodule;



typedef struct zobject
{
    union
    {
        double f;
        int64_t l;
        int32_t i;
        void* ptr;
    };
    char type;
}zobject;

typedef zobject(*NativeFunPtr)(zobject*,int);

#define AS_BOOL(x) (bool)x.i
#define AS_INT(x) x.i
#define AS_INT64(x) x.l
#define AS_DOUBLE(x) x.f
#define AS_BYTE(x) (uint8_t)x.i
#define AS_STR(x) ((zstr*)x.ptr)
#define AS_DICT(x) (zdict*)x.ptr
#define AS_LIST(x) (zlist*)x.ptr
#define AS_CLASS(x) (zclass*)x.ptr
#define AS_ClASSOBJ(x) (zclass_object*)x.ptr
#define AS_BYTEARRAY(x) (zbytearr*)x.ptr
#define AS_FILEOBJECT(x) (zfile*)x.ptr
#define AS_PTR(x) x.ptr

bool zobject_equals(zobject,zobject);





static inline bool isNumeric(char t)
{
  return (t == Z_INT || t == Z_FLOAT || t == Z_INT64);
}

zobject zobj_from_str_ptr(zstr*);
zobject zobj_from_int(int32_t);
zobject zobj_from_double(double);
zobject zobj_from_int64(int64_t);
zobject zobj_from_ptr(void*);
zobject zobj_from_byte(uint8_t);
zobject zobj_from_bool(bool);
zobject zobj_from_list(zlist*);
zobject zobj_from_dict(zdict*);
zobject zobj_from_module(zmodule*);
zobject zobj_from_class(zclass*);
zobject zobj_from_classobj(zclass_object*);
zobject zobj_from_bytearr(zbytearr*);
zobject zobj_from_file(zfile*);

#ifdef __cplusplus
}
#endif

#endif
