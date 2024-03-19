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

typedef struct ZList ZList;
typedef struct NativeFunction NativeFunction;
typedef struct ZByteArr ZByteArr;
typedef struct ZDict ZDict;
typedef struct zfile zfile;
typedef struct ZStr ZStr;
typedef struct StrMap StrMap;
typedef struct Klass Klass;
typedef struct KlassObject KlassObject;
typedef struct Module Module;



typedef struct ZObject
{
    union
    {
        double f;
        int64_t l;
        int32_t i;
        void* ptr;
    };
    char type;
}ZObject;

typedef ZObject(*NativeFunPtr)(ZObject*,int);

#define AS_BOOL(x) (bool)x.i
#define AS_INT(x) x.i
#define AS_INT64(x) x.l
#define AS_DOUBLE(x) x.f
#define AS_BYTE(x) (uint8_t)x.i
#define AS_STR(x) ((ZStr*)x.ptr)
#define AS_DICT(x) (ZDict*)x.ptr
#define AS_LIST(x) (ZList*)x.ptr
#define AS_KLASS(x) (Klass*)x.ptr
#define AS_KlASSOBJ(x) (KlassObject*)x.ptr
#define AS_BYTEARRAY(x) (ZByteArr*)x.ptr
#define AS_FILEOBJECT(x) (zfile*)x.ptr
#define AS_PTR(x) x.ptr

bool ZObject_equals(ZObject,ZObject);





inline bool isNumeric(char t)
{
  return (t == Z_INT || t == Z_FLOAT || t == Z_INT64);
}

ZObject ZObjFromStrPtr(ZStr*);
ZObject ZObjFromInt(int32_t);
ZObject ZObjFromDouble(double);
ZObject ZObjFromInt64(int64_t);
ZObject ZObjFromPtr(void*);
ZObject ZObjFromByte(uint8_t);
ZObject ZObjFromBool(bool);
ZObject ZObjFromList(ZList*);
ZObject ZObjFromDict(ZDict*);
ZObject ZObjFromModule(Module*);
ZObject ZObjFromKlass(Klass*);
ZObject ZObjFromKlassObj(KlassObject*);
ZObject ZObjFromByteArr(ZByteArr*);
ZObject ZObjFromFile(zfile*);

#ifdef __cplusplus
}
#endif

#endif
