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
#ifndef VM_H_
#define VM_H_

#include <stdint.h>
#include "apifunctions.h" 
#include "builtinfunc.h"
#include "byte_src.h"
#include "zuko-src.h"

#ifdef __cplusplus
extern "C"{
#endif

void promote_numeric_type(zobject* a, char t);
  
const char* zuko_typename(char);
zobject zobj_from_str(const char* str);// makes deep copy of str
zobject z_err(zclass* errKlass,const char* des);
zlist* vm_alloc_zlist();
zbytearr* vm_alloc_zbytearr();
//uint8_t* vm_alloc_raw(size_t);
zstr* vm_alloc_zstr(size_t);
zclass* vm_alloc_zclass(const char*);
zmodule* vm_alloc_zmodule(const char*);
zclass_object* vm_alloc_zclassobj(zclass*);
coroutine* vm_alloc_coro_obj();//allocates coroutine object
zfun* vm_alloc_zfun();
zfun* vm_alloc_coro(); //coroutine can be represented by fun_object
zfile * vm_alloc_zfile();
zdict* vm_alloc_zdict();
znativefun* vm_alloc_znativefun();
//callObject also behaves as a kind of try/catch since v0.3.1
bool vm_call_object(zobject*,zobject*,int,zobject*);
void vm_mark_important(void* mem);
void vm_unmark_important(void* mem);
void* vm_realloc_raw(void*,size_t);
void* vm_alloc_raw(size_t);

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

extern zlist STACK;
void vm_init();
void vm_load(uint8_t*,size_t,zuko_src*);
void interpret(size_t,bool);
void vm_destroy();
#ifdef __cplusplus
}
#endif


#endif
