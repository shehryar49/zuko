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

//This header should only be included by a zuko module!

#ifndef ZUKO_API_H
#define ZUKO_API_H
#include "apiver.h"
#include "zobject.h"
#include "zstr.h"
#include "zbytearray.h"
#include "strmap.h"
#include "funobject.h"
#include "klass.h"
#include "klassobject.h"
#include "module.h"
#include "nativefun.h"
#include "coroutineobj.h"
#include "zlist.h"
#include "zdict.h"
#include "apifunctions.h"

#ifdef __cplusplus
extern "C"{
#endif

//Error classes (defined in zapi.c)
extern zclass* Error;
extern zclass* TypeError;
extern zclass* ValueError;
extern zclass* MathError; 
extern zclass* NameError;
extern zclass* IndexError;
extern zclass* ArgumentError;
extern zclass* FileIOError;
extern zclass* KeyError;
extern zclass* OverflowError;
extern zclass* FileOpenError;
extern zclass* FileSeekError; 
extern zclass* ImportError;
extern zclass* ThrowError;
extern zclass* MaxRecursionError;
extern zclass* AccessError;
//these classes are set by the api_setup()


/**************/

/**************/
//Allocator function pointers (defined in zapi.c)
extern fn1 vm_alloc_zlist;
extern fn2 vm_alloc_zdict;
extern fn3 vm_alloc_zstr;
//extern fn4 vm_alloc_raw;
extern fn5 vm_alloc_zfile;
extern fn6 vm_alloc_zclass;
extern fn7 vm_alloc_zclassobj;
extern fn8 vm_alloc_znativefun;
extern fn9 vm_alloc_zmodule;
extern fn10 vm_alloc_zbytearr;
extern fn11 vm_call_object;
extern fn12 vm_mark_important;
extern fn13 vm_unmark_important;

#ifdef _WIN32
__declspec(dllexport)
#endif
int api_setup(apiFuncions* p,int ver);

//Some more helper functions for module developers

zclass* zclass_make_derived(zclass* base);
zobject zobj_from_str(const char*);// makes deep copy of str
zobject z_err(zclass*,const char*);
zobject zobj_from_method(const char*,NativeFunPtr,zclass*);
zobject zobj_from_function(const char*,NativeFunPtr);
inline zobject zobj_nil()
{
    zobject ret;
    ret.type = Z_NIL;
    return ret;
}
void zmodule_add_fun(zmodule* m,const char* name,NativeFunPtr p);
void zmodule_add_sig_fun(zmodule* m,const char* name,NativeFunPtr p,const char* sig);
void zclass_add_method(zclass* k,const char* name,NativeFunPtr p);
void zclass_add_sig_method(zclass* k,const char* name,NativeFunPtr p,const char* sig);

#ifdef __cplusplus
}
#endif

#endif