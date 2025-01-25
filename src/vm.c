#include "zbytearray.h"
#include "zlist.h"
#include <time.h>
#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
    #include <dlfcn.h>
#endif
#include "apiver.h"
#include "builtinfunc.h"
#include "zfileobj.h"
#include "vm.h"
#include "byte_src.h"
#include "funobject.h"
#include "lntable.h"
#include "opcode.h"
#include "overflow.h"
#include "strmap.h"
#include "misc.h"
#include "zobject.h"
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sizet_vector.h"
#include "mem_map.h"
#define THREADED_INTERPRETER //ask vm to use threaded interpreter if possible
//not defining this macro will always result in the simple switch based interpret loop

#ifdef THREADED_INTERPRETER
  #ifdef __GNUC__
    #define NEXT_INST goto *targets[*ip];
    #define CASE_CP
    #define ISTHREADED //threaded interpreter can be and will be implemented
  #else
    #define NEXT_INST continue
    #define CASE_CP case
  #endif
#else
  #define NEXT_INST continue
  #define CASE_CP case
#endif


#define VEC_LAST(x) x.arr[x.size-1]



const char* fullform(char t)
{
  if (t == Z_INT)
    return "Integer 32 bit";
  else if (t == Z_INT64)
    return "Integer 64 bit";
  else if (t == Z_FLOAT)
    return "Float";
  else if (t == Z_STR)
    return "String";
  else if (t == Z_LIST)
    return "List";
  else if (t == Z_BYTE)
    return "Byte";
  else if(t == Z_BYTEARR)
    return "Byte Array";
  else if (t == Z_BOOL)
    return "Boolean";
  else if (t == Z_FILESTREAM)
    return "File Stream";
  else if (t == Z_DICT)
    return "zdict";
  else if (t == Z_MODULE)
    return "Module";
  else if (t == Z_FUNC)
    return "Function";
  else if (t == Z_CLASS)
    return "Class";
  else if (t == Z_OBJ)
    return "Class Object";
  else if (t == Z_NIL)
    return "nil";
  else if (t == Z_NATIVE_FUNC)
    return "Native Function";
  else if (t == Z_POINTER)
    return "Native Pointer";
  else if (t == Z_ERROBJ)
    return "Error Object";
  else if (t == 'g')
    return "Coroutine";
  else if (t == 'z')
    return "Coroutine Object";
  else
    return "Unknown";
}

//Error classes
zclass* Error;
zclass* TypeError;
zclass* ValueError;
zclass* MathError; 
zclass* NameError;
zclass* IndexError;
zclass* ArgumentError;
zclass* FileIOError;
zclass* KeyError;
zclass* OverflowError;
zclass* FileOpenError;
zclass* FileSeekError; 
zclass* ImportError;
zclass* ThrowError;
zclass* MaxRecursionError;

zlist STACK;
ptr_vector executing; // pointer to zuko function object we are executing, NULL means control is not in a function
ptr_vector callstack;
sizet_vector frames;
uint8_t* program = NULL;
uint32_t program_size;
uint8_t* ip; // instruction pointer
//////////////////
bool viaCO = false;
sizet_vector try_stack_cleanup;
size_t orgk = 0;
ptr_vector except_targ; // uint*
sizet_vector try_limit_cleanup;


ptr_vector module_handles;

size_t GC_THRESHHOLD;
size_t GC_MIN_COLLECT;
lntable* line_num_table;
str_vector* files;
str_vector* sources;
zlist aux; // auxiliary space for markV2
ptr_vector vm_important;//important memory not to free even when not reachable
ptr_vector vm_builtin; // addresses of builtin native functions

size_t GC_Cycles = 0;
ptr_vector vm_strings; // string constants used in bytecode
apiFuncions api; // to share VM's allocation api with modules
// just a struct with a bunch of function pointers
zobject nil;
void mark();
void collectGarbage();
size_t allocated = 0;
mem_map memory;
///////////////////
void vm_init()
{
    zlist_init(&aux);
    zlist_init(&STACK);
    ptr_vector_init(&callstack);
    ptr_vector_init(&executing);
    ptr_vector_init(&vm_strings);
    ptr_vector_init(&vm_important);
    ptr_vector_init(&vm_builtin);
    ptr_vector_init(&except_targ);
    ptr_vector_init(&module_handles);
    
    sizet_vector_init(&frames);
    sizet_vector_init(&try_stack_cleanup);
    sizet_vector_init(&try_limit_cleanup);
    mem_map_init(&memory);
    ptr_vector_push(&executing,NULL);
    sizet_vector_push(&frames,0);
}
void vm_load(uint8_t* bytecode,size_t length,zuko_src* p)
{
    program = bytecode;
    program_size = length;
    GC_THRESHHOLD = 4196;//chosen at random
    GC_MIN_COLLECT = 4196;
    line_num_table = &p->line_num_table;
    files = &p->files;
    sources = &p->sources;
    nil.type = Z_NIL;

    api.a1 = &vm_alloc_zlist;
    api.a2 = &vm_alloc_zdict;
    api.a3 = &vm_alloc_zstr;
    //    api.a4 = &allocMutString; RIP
    api.a5 = &vm_alloc_zfile;
    api.a6 = &vm_alloc_zclass;
    api.a7 = &vm_alloc_zclassobj;
    api.a8 = &vm_alloc_znativefun;
    api.a9 = &vm_alloc_zmodule;
    api.a10 = &vm_alloc_zbytearr;
    api.a11 = &vm_call_object;
    api.a12 = &vm_mark_important;
    api.a13 = &vm_unmark_important;
    api.k1 = Error;
    api.k2 = TypeError;
    api.k3 = ValueError;
    api.k4 = MathError; 
    api.k5 = NameError;
    api.k6 = IndexError;
    api.k7 = ArgumentError;
    api.k8 = FileIOError;
    api.k9 = KeyError;
    api.k10 = OverflowError;
    api.k11 = FileOpenError;
    api.k12 = FileSeekError; 
    api.k13 = ImportError;
    api.k14 = ThrowError;
    api.k15 = MaxRecursionError;
    srand(time(0));
}
size_t spitErr(zclass* e,const char* msg) // used to show a runtime error
{
    zobject p1;
    if (except_targ.size != 0)//check if error catching is enabled
    {
        STACK.size = VEC_LAST(try_stack_cleanup);
        zclass_object* E = vm_alloc_zclassobj(e);
        size_t msglen = strlen(msg);
        zstr* s = vm_alloc_zstr(msglen);
        memcpy(s->val,msg,msglen);
        StrMap_set(&(E->members),"msg",zobj_from_str_ptr(s));
        p1.type = Z_OBJ;
        p1.ptr = (void*)E;
        zlist_push(&STACK,p1);
        frames.size = VEC_LAST(try_limit_cleanup);
        ip = VEC_LAST(except_targ);
        except_targ.size--;
        try_stack_cleanup.size--;
        try_limit_cleanup.size--;
        return ip - program;
    }
    if(viaCO) //interpret was called via callObject, wrap the error in a nice object
    //remove all values from stack starting from frame made by callObject
    {
        while(VEC_LAST(callstack) != NULL)//there must be a null
        {
            frames.size--;
            callstack.size--;
        }
        STACK.size = VEC_LAST(frames);
        zclass_object* E = vm_alloc_zclassobj(e);
        size_t msglen = strlen(msg);
        zstr* s = vm_alloc_zstr(msglen);
        memcpy(s->val,&msg[0],msglen);
        StrMap_set(&(E->members),"msg",zobj_from_str_ptr(s));
        p1.type = Z_ERROBJ;
        p1.ptr = (void*)E;
        zlist_push(&STACK,p1);
        ip = program + program_size - 1;
        return ip -program;
    }
    //Error catching is not enabled
    size_t line_num = 0;
    byte_src src;
    const char* filename = "";
    const char* source_code = "";
    if(lntable_get(line_num_table,orgk,&src))
    {
        line_num = src.ln;
        filename = files->arr[src.file_index];
        source_code = sources->arr[src.file_index];
    }
    const char* type = e->name;
    fprintf(stderr,"\nFile %s\n", filename);
    fprintf(stderr,"%s at line %zu\n", type, line_num);

    size_t l = 1;
    
    size_t i = 0;

    while (source_code[i] !=0 && l <= line_num)
    {
        if (source_code[i] == '\n')
            l += 1;
        else if (l == line_num)
            fputc(source_code[i],stderr);
        i += 1;
    }
    fprintf(stderr,"\n%s\n", msg);

    if (callstack.size != 0 && e != MaxRecursionError) // printing stack trace for max recursion is stupid
    {
        fprintf(stderr,"<stack trace>\n");
        while (callstack.size != 0) // print stack trace
        {
            size_t L = (uint8_t*)VEC_LAST(callstack) - program;
            L -= 1;
            while (L>0 && !(lntable_get(line_num_table,L,&src)))
            {
                L -= 1; // L is now the index of CALLUDF opcode now
            }
            // which is ofcourse present in the LineNumberTable
            const char* file = files->arr[src.file_index];
            size_t ln = src.ln;
            fprintf(stderr,"  --by %s line %zu\n", file,ln);
            callstack.size--;
        }
    }

    ip = program+program_size-1;//set instruction pointer to last instruction
    //which is always OP_EXIT
    //if(!REPL_MODE)//nothing can be done, clear stack and exit
    //    STACK.size = 0;
    STACK.size = 0;
    return ip - program;
}
void DoThreshholdBusiness()
{
    if (allocated > GC_THRESHHOLD)
    {
        mark();
        collectGarbage();
    }
}
void PromoteType(zobject* a, char t)
{
  if (a->type == Z_INT)
  {
    if (t == Z_INT64) // promote to int64_t
    {
      a->type = Z_INT64;
      a->l = (int64_t)a->i;
    }
    else if (t == Z_FLOAT)
    {
      a->type = Z_FLOAT;
      a->f = (double)a->i;
    }
  }
  else if (a->type == Z_FLOAT)
  {
    if (t == Z_INT64) // promote to int64_t
    {
      a->type = Z_INT64;
      a->l = (int64_t)a->f;
    }
    else if (t == Z_INT)
    {
      a->type = Z_INT;
      a->f = (int32_t)a->f;
    }
    else if (t == Z_FLOAT)
      return;
  }
  else if (a->type == Z_INT64)
  {
    if (t == Z_FLOAT) // only this one is needed
    {
      a->type = Z_FLOAT;
      a->f = (double)a->l;
    }
  }
}
 
static bool isHeapObj(zobject obj)
{
    if (obj.type != Z_INT && obj.type != Z_INT64 && obj.type != Z_FLOAT && obj.type != Z_NIL && obj.type != Z_BYTE && obj.type != Z_BOOL && obj.type != Z_POINTER)
        return true; // all other objects are on heap
    return false;
}
void markV2(zobject obj)
{
    aux.size = 0;
    int it;
    mem_info* tmp = NULL;
    if (isHeapObj(obj) && (tmp = mem_map_getref(&memory,obj.ptr)) && !(tmp->ismarked))
    {
        // mark object alive and push it
        tmp->ismarked = true;
        zlist_push(&aux,obj);
    }
    // if an object is on aux then
    //  - it is a heap object
    //  - it was unmarked but marked before pushing
    //  - known to VM's memory pool i.e not from outside (any native module or something)

    // After following above constraints,aux will not have any duplicates or already marked objects
    zobject curr;
    while (aux.size != 0)
    {
        zlist_fastpop(&aux,&curr);
        // for each curr object we get ,it was already marked alive before pushing onto aux
        // so we only process objects referenced by it and not curr itself
        if (curr.type == Z_DICT)
        {
            zdict* d = (zdict *)curr.ptr;
            for (size_t i=0;i<d->capacity;i++)
            {
                if(d->table[i].stat != OCCUPIED)
                    continue;
                // if e.first or e.second is a heap object and known to our memory pool
                // add it to aux
                zobject key = d->table[i].key;
                zobject val = d->table[i].val;
                if (isHeapObj(key) && (tmp = mem_map_getref(&memory,key.ptr)) && !(tmp->ismarked))
                {
                    tmp->ismarked = true;
                    zlist_push(&aux,key);
                }
                if (isHeapObj(val) && (tmp = mem_map_getref(&memory,val.ptr))&& !(tmp->ismarked))
                {
                    tmp->ismarked = true;
                    zlist_push(&aux,val);
                }
            }
        }
        else if (curr.type == Z_LIST)
        {
            zlist* d = (zlist *)curr.ptr;
            for (size_t i = 0;i<d->size;i++)
            {
                zobject e = d->arr[i];
                if (isHeapObj(e) && (tmp = mem_map_getref(&memory,e.ptr)) && !(tmp->ismarked))
                {
                    tmp->ismarked = true;
                    zlist_push(&aux,e);
                }
            }
        }
        else if (curr.type == 'z') // coroutine object
        {
            Coroutine* g = (Coroutine *)curr.ptr;
            for (size_t i = 0;i<g->locals.size;i++)
            {
                zobject e = g->locals.arr[i];
                if (isHeapObj(e) && (tmp = mem_map_getref(&memory,e.ptr)) && !(tmp->ismarked))
                {
                    tmp->ismarked = true;
                    zlist_push(&aux,e);
                }
            }
            zfun* gf = g->fun;
            tmp = mem_map_getref(&memory,(void *)gf);
            if (tmp && !(tmp->ismarked))
            {
                zobject fn;
                fn.type = Z_FUNC;
                fn.ptr = (void *)gf;
                tmp->ismarked = true;
                zlist_push(&aux,fn);
            }
        }
        else if (curr.type == Z_MODULE)
        {
            zmodule* k = (zmodule*)curr.ptr;
            for (size_t idx=0; idx<  k->members.capacity;idx++)
            {
                if(k->members.table[idx].stat!= SM_OCCUPIED)
                    continue;
                const char* key = k->members.table[idx].key;
                zobject val = k->members.table[idx].val;
                
                if (isHeapObj(val) && (tmp = mem_map_getref(&memory,val.ptr)) && !(tmp->ismarked))
                {
                    tmp->ismarked = true;
                    zlist_push(&aux,val);
                }
            }
        }
        else if (curr.type == Z_CLASS)
        {
            zclass* k = (zclass* )curr.ptr;
            for (size_t idx = 0; idx < k->members.capacity;idx++)
            {
                if(k->members.table[idx].stat != SM_OCCUPIED)
                    continue;
                const char* key = k->members.table[idx].key;
                zobject val = k->members.table[idx].val;
                if (isHeapObj(val) && (tmp = mem_map_getref(&memory,val.ptr)) && !(tmp->ismarked))
                {
                    tmp->ismarked = true;
                    zlist_push(&aux,val);
                }
                if( (tmp = mem_map_getref(&memory,(void*)key)))
                    tmp->ismarked = true;
                
            }
        }
        else if (curr.type == Z_NATIVE_FUNC)
        {
            znativefun *fn = (znativefun*)curr.ptr;
            tmp = mem_map_getref(&memory,(void *)fn->_klass);
            if (tmp && !(tmp->ismarked))
            {
                zobject klass;
                klass.type = Z_CLASS;
                klass.ptr = (void *)fn->_klass;
                tmp->ismarked = true;
                zlist_push(&aux,klass);
            }
        }
        else if (curr.type == Z_OBJ)
        {
            zclass_object* k = (zclass_object *)curr.ptr;
            zclass* kk = k->_klass;
            tmp = mem_map_getref(&memory,(void*)kk);
            if (tmp && !(tmp->ismarked))
            {
                zobject klass;
                klass.type = Z_CLASS;
                klass.ptr = (void *)kk;
                tmp->ismarked = true;
                zlist_push(&aux,klass);
            }
            for (size_t idx = 0; idx < k->members.capacity;idx++)
            {
                if(k->members.table[idx].stat != SM_OCCUPIED)
                    continue;
                const char* key = k->members.table[idx].key;
                zobject val = k->members.table[idx].val;
                if (isHeapObj(val) && (tmp = mem_map_getref(&memory,val.ptr)) && !(tmp->ismarked))
                {
                    tmp->ismarked = true;
                    zlist_push(&aux,val);
                }
            }
        }
        else if (curr.type == Z_FUNC || curr.type == 'g')
        {
            zclass* k = ((zfun *)curr.ptr)->_klass;
            tmp = mem_map_getref(&memory,k);
            if (tmp && !(tmp->ismarked))
            {
                zobject klass;
                klass.type = Z_CLASS;
                klass.ptr = (void *)k;
                tmp->ismarked = true;
                zlist_push(&aux,klass);
            }
            for (size_t i = 0;i< ((zfun *)curr.ptr)->opt.size;i++ )
            {
                zobject e = ((zfun*)curr.ptr)->opt.arr[i];
                if (isHeapObj(e) && (tmp = mem_map_getref(&memory,e.ptr)) && !(tmp->ismarked))
                {
                    tmp->ismarked = true;
                    zlist_push(&aux,e);
                }
            }
        }
    } // end while loop
}
void mark()
{
    for (size_t i=0;i<STACK.size;i++)
        markV2(STACK.arr[i]);
    for(size_t i = 0;i < vm_important.size; i++)
    {
        void* e = vm_important.arr[i];
        mem_info* tmp = NULL;
        tmp = mem_map_getref(&memory,e);
        if(tmp)
        {
            zobject p;
            p.type = tmp->type;
            p.ptr = e;
            markV2(p);
        }
    }
}

void collectGarbage()
{
    size_t pre = allocated;
    ptr_vector to_free;
    ptr_vector_init(&to_free);
    for (size_t i = 0; i< memory.capacity; i++)
    {
        if(memory.table[i].stat != MM_OCCUPIED)
            continue;
        void* ptr = memory.table[i].key;
        mem_info* m = &(memory.table[i].val);        
        if (m->ismarked)
            m->ismarked = false;
        else
        {
            //call destructor of unmarked objects
            if (m->type == Z_OBJ)
            {
                zclass_object* obj = (zclass_object*)ptr;
                zobject dummy;
                dummy.type = Z_OBJ;
                dummy.ptr = ptr;
                zobject cb;
                if (StrMap_get(&(obj->members),"__del__",&cb))
                {
                    zobject p1 = cb;
                    if(p1.type == Z_NATIVE_FUNC || p1.type == Z_FUNC)
                    {
                        zobject rr;
                        vm_call_object(&p1,&dummy,1,&rr);
                    }
                }
            }
            ptr_vector_push(&to_free,ptr);

        }
    }
    for (size_t i = 0; i <to_free.size; i++)
    {
        void* e = to_free.arr[i];
        mem_info* m = NULL;
        m = mem_map_getref(&memory,e);
        
        if (m->type == Z_LIST)
        {
            zlist_destroy((zlist*)e);
            free(e);
            allocated -= sizeof(zlist);
        }
        else if (m->type == Z_MODULE)
        {
            zmodule* m = (zmodule*)e;
            StrMap_destroy(&(m->members));
            free(e);
            allocated -= sizeof(zmodule);
        }
        else if (m->type == Z_CLASS)
        {
            zclass* k = (zclass*)e;
            StrMap_destroy(&(k->members));
            free(e);
            allocated -= sizeof(zclass);
        }
        else if (m->type == Z_OBJ)
        {
            zclass_object* k = (zclass_object*)e;
            StrMap_destroy(&(k->members));
            free(e);
            allocated -= sizeof(zclass_object);
        }
        else if (m->type == Z_DICT)
        {
            zdict_destroy((zdict*)e);
            free(e);
            allocated -= sizeof(zdict);
        }
        else if (m->type == Z_FILESTREAM)
        {
            zfile* zf = (zfile*)e;
            if(zf->open)
                fclose(zf->fp);
            free(e);
            allocated -= sizeof(zfile);
        }
        else if (m->type == Z_FUNC)
        {
            zlist_destroy(&((zfun*)e) ->opt);
            free(e);
            allocated -= sizeof(zfun);
        }
        else if(m->type == Z_COROUTINE)
        {
            free(e);
            allocated -= sizeof(zfun);
        }
        else if (m->type == 'z')
        {
            zlist_destroy(&((Coroutine*)e)->locals);
            free(e);
            allocated -= sizeof(Coroutine);
        }
        else if (m->type == Z_ERROBJ)
        {
            free(e);
            allocated -= sizeof(zclass_object);
        }
        else if (m->type == Z_STR)
        {
            zstr* p = (zstr*)e;
            allocated -= sizeof(zstr) + p->len + 1;
            free(p->val);
            free(p);
        }      
        else if (m->type == Z_NATIVE_FUNC)
        {
            free(e);
            allocated -= sizeof(znativefun);
        }
        else if(m->type == Z_BYTEARR)
        {
            zbytearr_destroy((zbytearr*)e);
            free(e);
            allocated -= sizeof(zbytearr);
        }
        else if(m->type == Z_RAW)
        {
            free(e);
        }
        mem_map_erase(&memory,e);
    }
    ptr_vector_destroy(&to_free);
    size_t recycled = pre - allocated;
    if (recycled < 4196) // a gc cycle is expensive so if we are collecting enough bytes we update the threshold
    {
        GC_THRESHHOLD *= 2;
    }
}
bool invokeOperator(const char* meth, zobject A, size_t args, const char* op, zobject* rhs, bool raiseErrOnNF) // check if the object has the specified operator overloaded and prepares to call it by updating callstack and frames
{
    //Operators invoked by this function are either unary or binary
    //So at max 1 argument is given that is zobject rhs (non-NULL means usable)
    //raiseErrOnNF = whether to raise error on not found or not ?
    zclass_object*obj = (zclass_object*)A.ptr;
    zobject p3;
    const char* s1;// to avoid multiple object creation across multiple invokeCalls
    char error_buffer[50];
    if (StrMap_get(&(obj->members),meth,&p3))
    {
        if (p3.type == Z_FUNC)
        {
            zfun *fn = (zfun *)p3.ptr;
            if (fn->opt.size != 0)
                spitErr(ArgumentError, "Optional parameters not allowed for operator functions!");
            if (fn->args == args)
            {
                ptr_vector_push(&callstack,ip+1);
                if (callstack.size >= 1000)
                {
                    spitErr(MaxRecursionError, "Max recursion limit 1000 reached.");
                    return false;
                }
                ptr_vector_push(&executing,fn);
                sizet_vector_push(&frames,STACK.size);
                zlist_push(&STACK,A);
                if (rhs != NULL)
                    zlist_push(&STACK,*rhs);
                ip = program + fn->i;
                return true;
            }
        }
        else if (p3.type == Z_NATIVE_FUNC)
        {
            znativefun *fn = (znativefun*)p3.ptr;
            NativeFunPtr M = fn->addr;
            zobject rr; 
            zobject argArr[2] = {A, (!rhs) ? nil : *rhs};
            // do typechecking 
            if(fn->signature)
            {
                size_t len = strlen(fn->signature);
                if(len != 2)
                {
                    snprintf(error_buffer,50,"Native function %s takes %zu arguments, 2 given!",fn->name,len);
                    spitErr(ArgumentError,error_buffer);
                    return false;
                }
                size_t i = 0;
                while(i<2)
                {
                if(argArr[i].type != fn->signature[i])
                {
                    snprintf(error_buffer,50,"Argument %zu to %s must be a %s",i+1,fn->name,fullform(fn->signature[i]));
                    spitErr(TypeError,error_buffer);
                    return false;
                }
                i+=1;
            }
        }
        rr = M(argArr, args);
        if (rr.type == Z_ERROBJ)
        {
            zclass_object* E = (zclass_object*)rr.ptr;
            zobject msg;
            StrMap_get(&(E->members),"msg",&msg);
            snprintf(error_buffer,50,"%s(): %s",meth,((zstr*)msg.ptr) -> val);
            spitErr(E->_klass, error_buffer);
            return false;
        }
        zlist_push(&STACK,rr);
        ip++;
        return true;
        }
    }
    if (!raiseErrOnNF)
        return false;
    if (args == 2)
    {
        snprintf(error_buffer,50,"Operator '%s' not supported for types %s and %s",op,fullform(A.type),fullform(rhs->type));
        spitErr(TypeError,error_buffer);
    }
    else
    {
        snprintf(error_buffer,50,"Operator '%s' unsupported for type %s",op,fullform(A.type));
        spitErr(TypeError,error_buffer);
    }
    return false;
}
void interpret(size_t offset , bool panic) //by default panic if stack is not empty when finished
{
    // Some registers
    int32_t i1;
    int32_t i2;
    #ifdef ISTHREADED
        #ifdef __GNUC__
        //label addresses to use with goto
        void* targets[] = 
        {
        NULL,
        &&LOAD,
        &&CALLMETHOD,
        &&ADD,
        &&MUL,
        &&DIV,
        &&SUB,
        &&JMP,
        &&CALL,
        &&XOR,
        &&ASSIGN,
        &&CMP_JMPIFFALSE,
        &&EQ,
        &&NOTEQ,
        &&SMALLERTHAN,
        &&GREATERTHAN,
        &&CALLUDF,
        &&INPLACE_INC,
        &&LOAD_STR,
        &&GOTOIFFALSE,
        &&NPOP_STACK,
        &&MOD,
        &&LOAD_NIL,
        &&LOAD_INT32,
        &&OP_RETURN,
        &&JMPNPOPSTACK,
        &&GOTONPSTACK,
        &&GOTO,
        &&POP_STACK,
        &&LOAD_CONST,
        &&CO_STOP,
        &&SMOREQ,
        &&GROREQ,
        &&NEG,
        &&NOT,
        &&INDEX,
        &&ASSIGNINDEX,
        &&IMPORT,
        &&LOOP,
        &&CALLFORVAL,
        &&INC_GLOBAL,
        &&DLOOP, //unused for now
        &&LOAD_GLOBAL,
        &&MEMB,
        &&JMPIFFALSENOPOP,
        &&ASSIGNMEMB,
        &&BUILD_CLASS,
        &&ASSIGN_GLOBAL,
        &&LOAD_FUNC,
        &&OP_EXIT,
        &&LSHIFT,
        &&RSHIFT,
        &&BITWISE_AND,
        &&BITWISE_OR,
        &&COMPLEMENT,
        &&BUILD_DERIVED_CLASS,
        &&LOAD_TRUE,
        &&IS,
        &&ONERR_GOTO,
        &&POP_EXCEP_TARG,
        &&THROW,
        &&LOAD_CO,
        &&YIELD,
        &&YIELD_AND_EXPECTVAL,
        &&LOAD_LOCAL,
        &&GC,
        &&NOPOPJMPIF,
        &&SELFMEMB,
        &&ASSIGNSELFMEMB,
        &&LOAD_FALSE,
        &&LOAD_BYTE,
        &&SETUP_LOOP,
        &&SETUP_DLOOP,
        &&RETURN_INT32,
        &&LOADVAR_SUBINT32,
        &&CALL_DIRECT,
        &&INDEX_FAST,
        &&LOADVAR_ADDINT32,
        &&LOAD_INT64,
        &&LOAD_DOUBLE
        };
        //size_t counts[sizeof(targets)/sizeof(void*)];
        //memset(counts,0,sizeof(targets));
        #endif
    #endif
    zobject p1;
    zobject p2;
    zobject p3;
    zobject p4;
    char error_buffer[100];
    char c1;
    zlist* pl_ptr1; // zuko list pointer 1
    zobject alwaysi32;
    zobject alwaysByte;
    zlist values;
    zlist names;
    zlist_init(&values);
    zlist_init(&names);
    alwaysi32.type = Z_INT;
    alwaysByte.type = Z_BYTE;
    int32_t* alwaysi32ptr = &alwaysi32.i;
    zdict *pd_ptr1;
    zbytearr* bt_ptr1;
    ip = program + offset;

    #ifndef ISTHREADED
    uint8_t inst;
    while (*ip != OP_EXIT)
    {
    inst = *ip;
    switch (inst)
    {
    #else
    NEXT_INST;
    #endif
    CASE_CP LOAD_GLOBAL:
    {
        ip += 1;
        memcpy(&i1, ip, 4);
        zlist_push(&STACK,STACK.arr[i1]);
        ip += 4;
        NEXT_INST;
    }
    CASE_CP LOAD_LOCAL:
    {
        ip += 1;
        memcpy(&i1, ip, 4);
        zlist_push(&STACK,STACK.arr[VEC_LAST(frames) + i1]);
        ip += 4;
        NEXT_INST;
    }
    CASE_CP LOAD_INT64:
    {
        ip++;
        zobject tmp;
        tmp.type = Z_INT64;
        memcpy(&tmp.l,ip,8);
        zlist_push(&STACK,tmp);
        ip+=8;
        NEXT_INST;
    }
    CASE_CP LOAD_DOUBLE:
    {
        ip++;
        zobject tmp;
        tmp.type = Z_FLOAT;
        memcpy(&tmp.f,ip,8);
        zlist_push(&STACK,tmp);
        ip+=8;
        NEXT_INST;
    }
    CASE_CP INC_GLOBAL:
    {
        orgk = ip - program;
        memcpy(&i1, ++ip, 4);
        ip += 3;
        c1 = STACK.arr[i1].type;
        if (c1 == Z_INT)
        {
            if (STACK.arr[i1].i == INT_MAX)
            {
                STACK.arr[i1].l = (int64_t)INT_MAX + 1;
                STACK.arr[i1].type = Z_INT64;
            }
            else
                STACK.arr[i1].i += 1;
        }
        else if (c1 == Z_INT64)
        {
            if (STACK.arr[i1].l == LLONG_MAX)
            {
                spitErr(OverflowError, "Numeric overflow");
                NEXT_INST;
            }
            STACK.arr[i1].l += 1;
        }
        else if (c1 == Z_FLOAT)
        {
            if (STACK.arr[i1].f == DBL_MAX)
            {
                spitErr(OverflowError, "Numeric overflow");
                NEXT_INST;
            }
            STACK.arr[i1].f += 1;
        }
        else
        {
            snprintf(error_buffer,100,"Cannot add numeric constant to type %s",fullform(c1));
            spitErr(TypeError, error_buffer);
            NEXT_INST;
        }
        ip++; NEXT_INST;
    }  
    CASE_CP LOAD:
    {
        orgk = ip - program;
        ip += 1;
        c1 = *ip;
        ip += 1;
        if (c1 == Z_LIST)
        {
            int32_t listSize;
            memcpy(&listSize, ip, sizeof(int32_t));
            ip += 3;
            pl_ptr1 = vm_alloc_zlist();
            zlist_resize(pl_ptr1,listSize);
            memcpy(pl_ptr1->arr,STACK.arr + STACK.size-listSize,listSize*sizeof(zobject));
            zlist_erase_range(&STACK,STACK.size - listSize,STACK.size - 1);
            p1.type = Z_LIST;
            p1.ptr = (void *)pl_ptr1;
            zlist_push(&STACK,p1);
            DoThreshholdBusiness();
        }
        else if (c1 == Z_DICT)
        {
            memcpy(&i1, ip, sizeof(int32_t));
            //i1 is dictionary size
            ip += 3;
            pd_ptr1 = vm_alloc_zdict();
            while (i1 != 0)
            {
            zlist_fastpop(&STACK,&p1);
            zlist_fastpop(&STACK,&p2);
            if(p2.type!=Z_INT && p2.type!=Z_INT64 && p2.type!=Z_FLOAT && p2.type!=Z_STR  && p2.type!=Z_BYTE && p2.type!=Z_BOOL)
            {
                snprintf(error_buffer,100,"Key of type %s not allowed.",fullform(p2.type));
                spitErr(TypeError,error_buffer);
                NEXT_INST;
            }
            if (zdict_get(pd_ptr1,p2,&p4))
            {
                spitErr(ValueError, "Duplicate keys in dictionary not allowed!");
                NEXT_INST;
            }
            zdict_set(pd_ptr1,p2,p1);
            i1 -= 1;
            }
            p1.ptr = (void *)pd_ptr1;
            p1.type = Z_DICT;
            zlist_push(&STACK,p1);
            DoThreshholdBusiness();
        }
        else if (c1 == 'v')
        {
            memcpy(&i1, ip, sizeof(int32_t));
            ip += 3;
            zlist_push(&STACK,STACK.arr[VEC_LAST(frames) + i1]);
        }
        ip++; NEXT_INST;
    }
    CASE_CP LOAD_CONST:
    {
        ip += 1;
        memcpy(&i1, ip, 4);
        ip += 4;
        //zlist_push(&STACK,vm_constants[i1]);
        NEXT_INST;
    }
    CASE_CP LOAD_INT32:
    {
        ip += 1;
        memcpy(alwaysi32ptr, ip, 4);
        ip += 4;
        zlist_push(&STACK,alwaysi32);
        NEXT_INST;
    }
    CASE_CP LOAD_BYTE:
    {
        ip += 1;
        alwaysByte.i = *ip;
        zlist_push(&STACK,alwaysByte);
        ip++; NEXT_INST;
    }
    CASE_CP ASSIGN:
    {
        ip += 1;
        memcpy(&i1, ip, 4);
        ip += 4;
        zlist_fastpop(&STACK,&p1);
        STACK.arr[VEC_LAST(frames) + i1] = p1;
        NEXT_INST;
    }
    CASE_CP ASSIGN_GLOBAL:
    {
        ip += 1;
        memcpy(&i1, ip, 4);
        ip += 4;
        zlist_fastpop(&STACK,&p1);
        STACK.arr[i1] = p1;
        NEXT_INST;
    }
    CASE_CP ASSIGNINDEX:
    {
        orgk = ip - program;
        p1 = STACK.arr[--STACK.size];
        p2 = STACK.arr[--STACK.size];
        p3 = STACK.arr[--STACK.size];

        if (p3.type == Z_LIST)
        {
            pl_ptr1 = (zlist *)p3.ptr;
            int64_t idx = 0;
            if (p1.type == Z_INT)
            idx = p1.i;
            else if (p1.type == Z_INT64)
            idx = p1.l;
            else
            {
                spitErr(TypeError,"index not an integer!");
                NEXT_INST;
            }
            if (idx < 0 || idx > (int64_t)pl_ptr1->size)
            {
                spitErr(IndexError,"index out of range!");
                NEXT_INST;
            }
            (*pl_ptr1).arr[idx] = p2;
        }
        else if (p3.type == Z_BYTEARR)
        {
            bt_ptr1 = (zbytearr*)p3.ptr;
            int64_t idx = 0;
            if (p1.type == Z_INT)
            idx = p1.i;
            else if (p1.type == Z_INT64)
            idx = p1.l;
            else
            {
                spitErr(TypeError, "index not an integer!");
                NEXT_INST;
            }
            if (idx < 0 || idx > (int64_t)bt_ptr1->size)
            {
                spitErr(IndexError, "index out of range!");
                NEXT_INST;
            }
            if(p2.type!=Z_BYTE)
            {
                spitErr(TypeError,"Error byte value required for bytearray!");
                NEXT_INST;
            }
            bt_ptr1->arr[idx] = (uint8_t)p2.i;
        }
        else if (p3.type == Z_DICT)
        {
            pd_ptr1 = (zdict *)p3.ptr;
            zdict_set(pd_ptr1,p1,p2);
        }
        else
        {
            snprintf(error_buffer,100,"index assignment unsupported for type %s",fullform(p3.type));
            spitErr(TypeError, error_buffer);
            NEXT_INST;
        }
        ip++; 
        NEXT_INST;
    }
    CASE_CP CALL:
    {
        orgk = ip - program;
        ip += 1;
        memcpy(&i1, ip, 4);
        ip += 4;
        int32_t howmany = *ip;
        p1 = ((BuiltinFunc)vm_builtin.arr[i1])(STACK.arr+STACK.size - howmany, howmany);
        if (p1.type == Z_ERROBJ)
        {
            zclass_object* E = (zclass_object*)p1.ptr;
            zobject msg;
            StrMap_get(&(E->members),"msg",&msg);
            spitErr(E->_klass,((zstr*)msg.ptr)->val);
            NEXT_INST;
        }
        STACK.size -= howmany;
        ip++;
        NEXT_INST;
    }
    CASE_CP CALLFORVAL:
    {
        orgk = ip - program;
        ip += 1;
        memcpy(&i1, ip, 4);
        ip += 4;
        i2 = *ip;
        p1 = ((BuiltinFunc)vm_builtin.arr[i1])(STACK.arr+STACK.size - i2, i2);
        if (p1.type == Z_ERROBJ)
        {
            zclass_object* E = (zclass_object*)p1.ptr;
            zobject msg;
            StrMap_get(&(E->members),"msg",&msg);
            spitErr(E->_klass, ((zstr*)msg.ptr)->val);
            NEXT_INST;
        }
        STACK.size -= i2;
        zlist_push(&STACK,p1);
        ip++; NEXT_INST;
    }
    CASE_CP CALLMETHOD:
    {
        orgk = ip - program;
        ip++;
        memcpy(&i1, ip, 4);
        ip += 4;
        const char* method_name = ((zstr*)vm_strings.arr[i1])->val;
        i2 = *ip;
        p1 = STACK.arr[STACK.size-i2-1]; // Parent object from which member is being called
        if (p1.type == Z_MODULE)
        {
            zmodule* m = (zmodule*)p1.ptr;
            if (!StrMap_get(&(m->members),method_name,&p3))
            {
                snprintf(error_buffer,100,"The module has no member %s",method_name);
                spitErr(NameError, error_buffer);
                NEXT_INST;
            }
            if (p3.type == Z_NATIVE_FUNC)
            {
            znativefun *fn = (znativefun *)p3.ptr;
            NativeFunPtr f = fn->addr;
            zobject *argArr = &STACK.arr[STACK.size-i2];
            if(fn->signature)
            {
                size_t len = strlen(fn->signature);
                if(len != i2)
                {
                    snprintf(error_buffer,sizeof(error_buffer),"native function %s takes %zu arguments, %d given",fn->name,len,i2);
                    spitErr(ArgumentError,error_buffer);
                    NEXT_INST;
                }
                size_t i = 0;
                while(i<i2)
                {
                if(argArr[i].type != fn->signature[i])
                {
                    snprintf(error_buffer,100,"argument %zu to %s must be a %s",i+1,fn->name,fullform(fn->signature[i]));
                    spitErr(TypeError,error_buffer);
                    NEXT_INST;
                }
                i+=1;
                }
            }
            p4 = f(argArr, i2);

            if (p4.type == Z_ERROBJ)
            {
                // The module raised an error
                zclass_object* E = (zclass_object*)p4.ptr;
                zobject msg;
                StrMap_get(&(E->members),"msg",&msg);
                snprintf(error_buffer,100,"%s(): %s",method_name,AS_STR(msg)->val);
                spitErr(E->_klass,error_buffer);
                NEXT_INST;
            }
            if (strcmp(fullform(p4.type),"Unknown")==0 && p4.type != Z_NIL)
            {
                spitErr(ValueError, "Error invalid response from module!");
                NEXT_INST;
            }
            zlist_erase_range(&STACK,STACK.size-i2-1,STACK.size-1);
            zlist_push(&STACK,p4);
            }
            else if (p3.type == Z_CLASS)
            {
                zclass_object*obj = vm_alloc_zclassobj((zclass*)p3.ptr); // instance of class
                zobject r;
                r.type = Z_OBJ;
                r.ptr = (void *)obj;
                STACK.arr[STACK.size-i2-1] = r;
                zobject construct;
                if (StrMap_get(&(obj->members),"__construct__",&construct))
                {
                    if (construct.type != Z_NATIVE_FUNC)
                    {
                        spitErr(TypeError, "Error constructor of module's class is not a native function!");
                        NEXT_INST;
                    }
                    znativefun *fn = (znativefun *)construct.ptr;
                    NativeFunPtr p = fn->addr;

                    zobject *args = &STACK.arr[STACK.size-i2-1];
                    if(fn->signature)
                    {
                        size_t len = strlen(fn->signature);
                        if(len != i2)
                        {
                            snprintf(error_buffer,100,"Native function %s takes %zu arguments, %d given.",fn->name,len,i2);
                            spitErr(ArgumentError,error_buffer);
                            NEXT_INST;
                        }
                        size_t i = 0;
                        while(i<i2)
                        {
                            if(args[i].type != fn->signature[i])
                            {
                                snprintf(error_buffer,100,"Argument %zu to %s must be %s",i+1,fn->name,fullform(fn->signature[i]));
                                spitErr(TypeError,error_buffer);
                                NEXT_INST;
                            }
                            i+=1;
                        }
                    }
                    p1 = p(args, i2 + 1);
                    if (p1.type == Z_ERROBJ)
                    {
                        // The module raised an error
                        zclass_object* E = (zclass_object*)p1.ptr;
                        zobject msg;
                        StrMap_get(&(E->members),"msg",&msg);
                        snprintf(error_buffer,100,"%s(): %s",method_name,AS_STR(msg)->val);
                        spitErr(E->_klass, error_buffer);
                        NEXT_INST;
                    }
                }
                STACK.size -= i2;
                DoThreshholdBusiness();
            }
            else // that's it modules cannot have zuko code functions (at least not right now)
            {
                spitErr(TypeError, "Error member of module is not a function so cannot be called.");
                NEXT_INST;
            }
        }
        else if (p1.type == Z_LIST || p1.type == Z_DICT || p1.type == Z_BYTEARR || p1.type == Z_STR)
        {
            zobject callmethod(const char*, zobject *, int32_t);
            p3 = callmethod(method_name, &STACK.arr[STACK.size-i2-1], i2 + 1);
            if (p3.type == Z_ERROBJ)
            {
                zclass_object* E = (zclass_object*)p3.ptr;
                zobject msg;
                StrMap_get(&(E->members),"msg",&msg);
                spitErr(E->_klass, ((zstr *)msg.ptr)->val);
                NEXT_INST;
            }
            zlist_erase_range(&STACK,STACK.size-i2-1,STACK.size-1);
            zlist_push(&STACK,p3);
        }
        else if (p1.type == Z_OBJ)
        {
            zclass_object*obj = (zclass_object*)p1.ptr;
            zobject tmp;

            if (!StrMap_get(&(obj->members),method_name,&tmp))
            {
                snprintf(error_buffer,100,"Object has no member %s",method_name);
                spitErr(NameError,error_buffer);
                NEXT_INST;
            }
            else
                p4 = tmp;
            if (p4.type == Z_FUNC)
            {
                zfun *memFun = (zfun *)p4.ptr;
                if ((size_t)i2 + 1 + memFun->opt.size < memFun->args || (size_t)i2 + 1 > memFun->args)
                {
                    snprintf(error_buffer,100,"Function %s takes %zu arguments, %d given! ",memFun->name,memFun->args-1,i2);
                    NEXT_INST;
                }
                ptr_vector_push(&callstack,ip+1);
                if (callstack.size >= 1000)
                {
                    spitErr(MaxRecursionError, "Error max recursion limit 1000 reached.");
                    NEXT_INST;
                }
                ptr_vector_push(&executing,memFun);
                sizet_vector_push(&frames,STACK.size-i2-1);
                // add default arguments
                for (size_t i = memFun->opt.size - (memFun->args - 1 - (size_t)i2); i < (memFun->opt.size); i++)
                {
                    zlist_push(&STACK,memFun->opt.arr[i]);
                }
            //
                ip = program + memFun->i;
                NEXT_INST;
            }
            else if (p4.type == Z_NATIVE_FUNC)
            {
                znativefun *fn = (znativefun *)p4.ptr;
                NativeFunPtr R = fn->addr;
                zobject *args = &STACK.arr[STACK.size-i2-1];
                zobject rr;
                if(fn->signature)
                {
                    size_t len = strlen(fn->signature);
                    if(len != i2+1)
                    {
                        snprintf(error_buffer,100,"Native function %s takes %zu arguments, %d given!",fn->name,len,i2);
                        NEXT_INST;
                    }
                    size_t i = 0;
                    while(i<i2+1)
                    {
                        if(args[i].type != fn->signature[i])
                        {
                            snprintf(error_buffer,100,"Argument %zu to %s should be a %s",i+1,fn->name,fullform(fn->signature[i]));
                            NEXT_INST;
                        }
                        i+=1;
                    }
                }
                rr = R(args, i2 + 1);
                if (rr.type == Z_ERROBJ)
                {
                    zclass_object* E = (zclass_object*)rr.ptr;
                    zobject msg;
                    StrMap_get(&(E->members),"msg",&msg);
                    snprintf(error_buffer,100,"%s: %s",fn->name,AS_STR(msg)->val);
                    spitErr(E->_klass,error_buffer);
                    NEXT_INST;
                }
                zlist_erase_range(&STACK,STACK.size-i2-1,STACK.size-1);
                zlist_push(&STACK,rr);
            }
            else
            {
                snprintf(error_buffer,100,"Member %s of object has type %s and is not callable!",method_name,fullform(p4.type));
                spitErr(NameError,error_buffer);
                NEXT_INST;
            }
        }
        else if (p1.type == 'z')
        {
            if(i2!=0)
                p4 = STACK.arr[STACK.size - 1];
            Coroutine *g = (Coroutine *)p1.ptr;
            if (strcmp(method_name,"isalive") == 0)
            {
                if(i2!=0)
                {
                    spitErr(NameError,"Error coroutine member isalive() takes 0 arguments!");
                    NEXT_INST;
                }
                zobject isAlive;
                isAlive.type = Z_BOOL;
                isAlive.i = (g->state != STOPPED);
                zobject tmp;
                zlist_fastpop(&STACK,&tmp);
                zlist_push(&STACK,isAlive);
                ip++;
                NEXT_INST;
            }
            if (strcmp(method_name,"resume"))
            {
                snprintf(error_buffer,100,"Coroutine object has no member %s",method_name);
                spitErr(NameError,error_buffer);
                NEXT_INST;
            }

            if (g->state == STOPPED)
            {
                spitErr(ValueError, "Error the coroutine has terminated, cannot resume it!");
                NEXT_INST;
            }

            if (g->state == RUNNING)
            {
                spitErr(ValueError, "Error the coroutine already running cannot resume it!");
                NEXT_INST;
            }

            if (g->giveValOnResume)
            {
            if (i2 != 1)
            {
                spitErr(ValueError, "Error the coroutine expects one value to be resumed!");
                NEXT_INST;
            }
            }
            else if(i2 != 0)
            {
                spitErr(ValueError, "Error the coroutine does not expect any value to resume it!");
                NEXT_INST;
            }
            ptr_vector_push(&callstack,ip + 1);
            if (callstack.size >= 1000)
            {
                spitErr(MaxRecursionError, "Error max recursion limit 1000 reached.");
                NEXT_INST;
            }
            ptr_vector_push(&executing,NULL);
            sizet_vector_push(&frames,STACK.size-i2);
            zlist_insert_list(&STACK,STACK.size,&(g->locals));
            g->state = RUNNING;
            g->giveValOnResume = false;
            ip = program + g->curr;
            NEXT_INST;
        }
        else
        {
            snprintf(error_buffer,100,"Member type %s is not callable!",fullform(p1.type));
            spitErr(TypeError,error_buffer);
            NEXT_INST;
        }
        ip++; NEXT_INST;
    }
    CASE_CP ASSIGNMEMB:
    {
        orgk = ip - program;
        zlist_fastpop(&STACK,&p2);//value
        zlist_fastpop(&STACK,&p1);//parent
        ip++;
        memcpy(&i1, ip, 4);
        ip += 3;
        const char* memberName = ((zstr*)vm_strings.arr[i1])->val;        
        if (p1.type != Z_OBJ)
        {
            snprintf(error_buffer,100,"Member assignment unsupported for type %s",fullform(p1.type));
            spitErr(TypeError,error_buffer);
            NEXT_INST;
        }
        zclass_object*ptr = (zclass_object*)p1.ptr;
        zobject* ref;
        if (!(ref = StrMap_getRef(&(ptr->members),memberName)))
        {
            snprintf(error_buffer,100,"Object no member named %s",memberName);
            spitErr(NameError,error_buffer);
            NEXT_INST;
        }
        *ref = p2;
        ip++; 
        NEXT_INST;
    }
    CASE_CP IMPORT:
    {
        orgk = ip - program;
        memcpy(&i1, ++ip, sizeof(int32_t));
        ip += 3;
        const char* s1 = ((zstr*)vm_strings.arr[i1])->val;
        typedef zobject (*initFun)();
        typedef int (*apiFun)(apiFuncions *,int);
        #ifdef _WIN32
            s1 = "C:\\zuko\\modules\\" + s1 + ".dll";
            HINSTANCE module = LoadLibraryA(s1);
            if (!module)
            {
                spitErr(ImportError, "LoadLibrary() returned " + to_string(GetLastError()));
                NEXT_INST;
            }
            apiFun a = (apiFun)GetProcAddress(module, "api_setup");
            initFun f = (initFun)GetProcAddress(module, "init");
            
        #endif
        #ifdef __linux__
            snprintf(error_buffer,100,"./modules/%s.so",s1);
            void* module = dlopen(error_buffer,RTLD_LAZY);
            if(!module)
            {
                snprintf(error_buffer,100,"/opt/zuko/modules/%s.so",s1);
                module = dlopen(error_buffer, RTLD_LAZY);
                if (!module)
                {
                    spitErr(ImportError, "Failed to import module");
                    NEXT_INST;
                }
            }
            initFun f = (initFun)dlsym(module, "init");
            apiFun a = (apiFun)dlsym(module, "api_setup");
        #endif
        #ifdef __APPLE__
            s1 = "/opt/zuko/modules/"+s1+".dylib";
            void *module = dlopen(s1.c_str(), RTLD_LAZY);
            if (!module)
            {
                spitErr(ImportError, (std::string)(dlerror()));
                NEXT_INST;
            }
            initFun f = (initFun)dlsym(module, "init");
            apiFun a = (apiFun)dlsym(module, "api_setup");
        #endif
        if (!f)
        {
            spitErr(ImportError, "Error init() function not found in the module");
            NEXT_INST;
        }
        if (!a)
        {
            spitErr(ImportError, "Error api_setup() function not found in the module");
            NEXT_INST;
        }
        int ret = a(&api,ZUKO_API_VERSION);
        if(!ret)
        {
            spitErr(ImportError,"The module is not compatible with this interpreter! It uses a different version of extension api.");
            NEXT_INST;
        }
        p1 = f();
        if (p1.type != Z_MODULE)
        {
            spitErr(ValueError, "Error module's init() should return a module object!");
            NEXT_INST;
        }
        ptr_vector_push(&module_handles,module);
        zlist_push(&STACK,p1);
        ip++; NEXT_INST;
    }
    CASE_CP LOOP:
    {
        orgk = ip - program;
        ip+=1;
        uint8_t is_global = *(ip++);
        int32_t lcv_idx;
        int32_t end_idx;
        int32_t where;

        memcpy(&lcv_idx,ip,4);
        ip+=4;
        if(is_global)
        {
            memcpy(&end_idx,ip,4);
            ip+=4;
            end_idx += VEC_LAST(frames);
        }
        else
        {
            lcv_idx+=VEC_LAST(frames);
            end_idx=lcv_idx+1;
        }
        memcpy(&where,ip,4);
        ip+=4;
        STACK.size = end_idx + 2;
        if(STACK.arr[lcv_idx].type != Z_INT64)
        {
            snprintf(error_buffer,100,"Type of loop control variable changed to %s",fullform(STACK.arr[lcv_idx].type));
            spitErr(TypeError,error_buffer);
            NEXT_INST;
        }
        if(STACK.arr[lcv_idx].l  > LLONG_MAX - STACK.arr[end_idx+1].l)
        {
            spitErr(OverflowError,"Numeric overflow when adding step");
            NEXT_INST;
        }
        STACK.arr[lcv_idx].l += STACK.arr[end_idx+1].l;
        
        if(STACK.arr[lcv_idx].l <= STACK.arr[end_idx].l)
            ip = program + where;
        //if(--STACK.arr[end_idx].l)
        //    ip = program + where;
        NEXT_INST;
    }
    CASE_CP DLOOP:
    {
        orgk = ip - program;
        ip+=1;
        uint8_t is_global = *(ip++);
        int32_t lcv_idx;
        int32_t end_idx;
        int32_t where;

        memcpy(&lcv_idx,ip,4);
        ip+=4;
        if(is_global)
        {
            memcpy(&end_idx,ip,4);
            ip+=4;
            end_idx += VEC_LAST(frames);
        }
        else
        {
            lcv_idx+= VEC_LAST(frames);
            end_idx=lcv_idx+1;
        }
        memcpy(&where,ip,4);
        ip+=4;
        STACK.size = end_idx + 2;
        if(STACK.arr[lcv_idx].type != Z_INT64)
        {
            snprintf(error_buffer,100,"Type of loop control variable changed to %s",fullform(STACK.arr[lcv_idx].type));
            spitErr(TypeError,error_buffer);
            NEXT_INST;
        }
        if(STACK.arr[lcv_idx].l  < LLONG_MIN + STACK.arr[end_idx+1].l )
        {
            spitErr(OverflowError,"Numeric overflow when subtracting step");
            NEXT_INST;
        }
        STACK.arr[lcv_idx].l -= STACK.arr[end_idx+1].l;
        if(STACK.arr[lcv_idx].l >= STACK.arr[end_idx].l)
            ip = program + where;
        NEXT_INST;
    }
    CASE_CP SETUP_LOOP:
    {
        orgk = ip - program;
        ip+=1;
        uint8_t is_global = *(ip++);
        int32_t lcv_idx;
        int32_t end_idx;
        int32_t where;

        memcpy(&lcv_idx,ip,4);
        ip+=4;
        if(is_global)
        {
            memcpy(&end_idx,ip,4);
            ip+=4;
        }
        memcpy(&where,ip,4);
        ip+=4;
        if(!is_global)
        {
            lcv_idx += VEC_LAST(frames);
            end_idx = lcv_idx+1;
        }
        else
        {
            end_idx += VEC_LAST(frames);
        }    

        if(STACK.arr[lcv_idx].type == Z_INT)
            PromoteType(&STACK.arr[lcv_idx],Z_INT64);
        if(STACK.arr[end_idx].type == Z_INT)
            PromoteType(&STACK.arr[end_idx], Z_INT64);
        if(STACK.arr[end_idx+1].type == Z_INT)
            PromoteType(&STACK.arr[end_idx+1], Z_INT64);
        if(STACK.arr[lcv_idx].type != Z_INT64 || STACK.arr[end_idx].type!=Z_INT64 || STACK.arr[end_idx+1].type!=Z_INT64)
        {
            spitErr(TypeError,"for requires start, end and step all to be integers!");
            NEXT_INST;
        }
        if(STACK.arr[lcv_idx].l > STACK.arr[end_idx].l)
            ip = program + where;
        int64_t iterations = (int64_t)ceil((STACK.arr[end_idx].l - STACK.arr[lcv_idx].l + 1)/(double)STACK.arr[end_idx+1].l);
        //STACK.arr[end_idx].l = iterations;
        //printf("iterations = %lld\n",iterations);
        NEXT_INST;
    }
    CASE_CP SETUP_DLOOP:
    {
        orgk = ip - program;
        ip+=1;
        uint8_t is_global = *(ip++);
        int32_t lcv_idx;
        int32_t end_idx;
        int32_t where;

        memcpy(&lcv_idx,ip,4);
        ip+=4;
        if(is_global)
        {
            memcpy(&end_idx,ip,4);
            ip+=4;
        }
        memcpy(&where,ip,4);
        ip+=4;
        if(!is_global)
        {
            lcv_idx += VEC_LAST(frames);
            end_idx = lcv_idx+1;
        }
        else
        {
            end_idx += VEC_LAST(frames);
        }    

        if(STACK.arr[lcv_idx].type == Z_INT)
            PromoteType(&STACK.arr[lcv_idx],Z_INT64);
        if(STACK.arr[end_idx].type == Z_INT)
            PromoteType(&STACK.arr[end_idx], Z_INT64);
        if(STACK.arr[end_idx+1].type == Z_INT)
            PromoteType(&STACK.arr[end_idx+1], Z_INT64);
        if(STACK.arr[lcv_idx].type != Z_INT64 || STACK.arr[end_idx].type!=Z_INT64 || STACK.arr[end_idx+1].type!=Z_INT64)
        {
            spitErr(TypeError,"for requires start, end and step all to be integers!");
            NEXT_INST;
        }
        if(STACK.arr[lcv_idx].l < STACK.arr[end_idx].l)
            ip = program + where;
        
        NEXT_INST;
    }
    CASE_CP OP_RETURN:
    {
        ip = VEC_LAST(callstack);
        callstack.size--;
        executing.size--;
        p1 = STACK.arr[STACK.size - 1];
        STACK.size = VEC_LAST(frames)+1;
        STACK.arr[VEC_LAST(frames)] = p1;
        frames.size--;
        if(!ip)
            return;//return from interpret function
        NEXT_INST;
    }
    CASE_CP RETURN_INT32:
    {
        ip+=1;
        memcpy(&p1.i,ip,4);
        ip+=4;
        ip = VEC_LAST(callstack);
        callstack.size--;
        executing.size--;
        STACK.size = VEC_LAST(frames);
        p1.type = Z_INT;
//        STACK.arr[STACK.size++] = p1;
        zlist_push(&STACK,p1);
        frames.size--;
        if(!ip)
            return;//return from interpret function
        NEXT_INST;
    }
    CASE_CP YIELD:
    {
        executing.size--;
        zlist_fastpop(&STACK,&p1);
        zobject* locals = STACK.arr + VEC_LAST(frames);
        size_t total = STACK.size - VEC_LAST(frames);
        STACK.size = VEC_LAST(frames);
        zlist_fastpop(&STACK,&p2);//genObj
        Coroutine *g = (Coroutine *)p2.ptr;
        zlist_resize(&(g->locals),total);
        if(total != 0)
            memcpy(g->locals.arr,locals,total*sizeof(zobject));
        g->curr = ip - program + 1;
        g->state = SUSPENDED;
        g->giveValOnResume = false;
        frames.size--;
        zlist_push(&STACK,p1);
        ip = callstack.arr[callstack.size - 1] - 1;
        callstack.size--;
        ip++; 
        NEXT_INST;
    }
    CASE_CP YIELD_AND_EXPECTVAL:
    {
        executing.size--;
        zlist_fastpop(&STACK,&p2);
        zobject* locals = STACK.arr + VEC_LAST(frames);
        size_t total = STACK.size - VEC_LAST(frames);
        STACK.size = VEC_LAST(frames);
        zlist_fastpop(&STACK,&p1);
        Coroutine *g = (Coroutine *)p1.ptr;
        zlist_resize(&(g->locals),total);
        if(total != 0)
            memcpy(g->locals.arr,locals,total*sizeof(zobject));
        g->curr = ip - program + 1;
        g->state = SUSPENDED;
        g->giveValOnResume = true;
        frames.size--;
        zlist_push(&STACK,p2);
        ip = callstack.arr[callstack.size - 1] - 1;
        callstack.size--;
        ip++; NEXT_INST;
    }
    CASE_CP LOAD_NIL:
    {
        p1.type = Z_NIL;
        zlist_push(&STACK,p1);
        ip++; NEXT_INST;
    }
    CASE_CP CO_STOP:
    {
        executing.size--;
        zobject val;
        zlist_fastpop(&STACK,&val);
        //we don't really need to save the coroutines locals anymore
        STACK.size = VEC_LAST(frames);
        zobject genObj;
        zlist_fastpop(&STACK,&genObj);
        Coroutine *g = (Coroutine *)genObj.ptr;
        g->locals.size = 0;
        g->curr = ip - program + 1;
        g->state = STOPPED;
        frames.size--;
        zlist_push(&STACK,val);
        ip = callstack.arr[callstack.size - 1] - 1;
        callstack.size--;
        ip++; NEXT_INST;
    }
    CASE_CP POP_STACK:
    {
        --STACK.size;
        ip++;
        NEXT_INST;
    }
    CASE_CP NPOP_STACK:
    {
        ip += 1;
        memcpy(&i1, ip, 4);
        ip += 4;
        STACK.size -= i1;
        NEXT_INST;
    }
    CASE_CP LOAD_TRUE:
    {
        p1.type = Z_BOOL;
        p1.i = 1;
        zlist_push(&STACK,p1);
        ip++; NEXT_INST;
    }
    CASE_CP LOAD_FALSE:
    {
        p1.type = Z_BOOL;
        p1.i = 0;
        zlist_push(&STACK,p1);
        ip++; NEXT_INST;
    }
    CASE_CP LSHIFT:
    {
        orgk = ip - program;
        zlist_fastpop(&STACK,&p2);
        zlist_fastpop(&STACK,&p1);
        if (p1.type == Z_OBJ)
        {
            invokeOperator("__lshift__", p1, 2, "<<", &p2,true);
            NEXT_INST;
        }
        if (p2.type != Z_INT)
        {
            spitErr(TypeError, "Error rhs for '<<' must be an Integer 32 bit");
            NEXT_INST;
        }
        p3.type = p1.type;
        if (p1.type == Z_INT)
        {
            p3.i = p1.i << p2.i;
        }
        else if (p1.type == Z_INT64)
        {
            p3.l = p1.l << p2.i;
        }
        else if(p1.type == Z_BYTE)
        {
            uint8_t p = p1.i;
            p<<=p2.i;
            p3.i = p;
        }
        else
        {
            snprintf(error_buffer,100,"Operator << unsupported for type %s",fullform(p1.type));
            spitErr(TypeError, error_buffer);;
            NEXT_INST;
        }
        STACK.arr[STACK.size++] = p3;
        ip++; NEXT_INST;
    }
    CASE_CP RSHIFT:
    {
        orgk = ip - program;
        zlist_fastpop(&STACK,&p2);
        zlist_fastpop(&STACK,&p1);
        if (p1.type == Z_OBJ)
        {
            invokeOperator("__rshift__", p1, 2, ">>", &p2,true);
            NEXT_INST;
        }
        if (p2.type != Z_INT)
        {
            spitErr(TypeError, "Error rhs for '>>' must be an Integer 32 bit");
            NEXT_INST;
        }
        p3.type = p1.type;
        if (p1.type == Z_INT)
        {
            p3.i = p1.i >> p2.i;
        }
        else if (p1.type == Z_INT64)
        {
            p3.l = p1.l >> p2.i;
        }
        else if(p1.type == Z_BYTE)
        {
            uint8_t p = p1.i;
            p>>=p2.i;
            p3.i = p;
        }
        else
        {
            snprintf(error_buffer,100,"Operator >> unsupported for type %s",fullform(p1.type));
            spitErr(TypeError,error_buffer);
            NEXT_INST;
        }
        zlist_push(&STACK,p3);
        ip++; NEXT_INST;
    }
    CASE_CP BITWISE_AND:
    {
        orgk = ip - program;
        zlist_fastpop(&STACK,&p2);
        zlist_fastpop(&STACK,&p1);
        if (p1.type == Z_OBJ)
        {
            invokeOperator("__bitwiseand__", p1, 2, "&", &p2,true);
            NEXT_INST;
        }
        if (p1.type != p2.type)
        {
            spitErr(TypeError, "Error operands should have same type for '&' ");
            NEXT_INST;
        }
        p3.type = p1.type;
        if (p1.type == Z_INT)
        {
            p3.i = p1.i & p2.i;
        }
        else if (p1.type == Z_INT64)
        {
            p3.l = p1.l & p2.l;
        }
        else if (p1.type == Z_BYTE)
        {
            p3.i = (uint8_t)p1.i & (uint8_t)p2.i;
        }
        else
        {
            snprintf(error_buffer,100,"Operator & unsupported for type %s",fullform(p1.type));
            spitErr(TypeError, error_buffer);
            NEXT_INST;
        }
        zlist_push(&STACK,p3);
        ip++; NEXT_INST;
    }
    CASE_CP BITWISE_OR:
    {
        orgk = ip - program;
        zlist_fastpop(&STACK,&p2);
        zlist_fastpop(&STACK,&p1);
        if (p1.type == Z_OBJ)
        {
            invokeOperator("__bitwiseor__", p1, 2, "|", &p2,true);
            NEXT_INST;
        }
        if (p1.type != p2.type)
        {
            spitErr(TypeError, "Error operands should have same type for '|' ");
            NEXT_INST;
        }
        p3.type = p1.type;
        if (p1.type == Z_INT)
        {
            p3.i = p1.i | p2.i;
        }
        else if (p1.type == Z_INT64)
        {
            p3.l = p1.l | p2.l;
        }
        else if (p1.type == Z_BYTE)
        {
            p3.i = (uint8_t)p1.i | (uint8_t)p2.i;
        }
        else
        { 
            snprintf(error_buffer,100,"Operator '|' unsupported for type %s",fullform(p1.type));
            spitErr(TypeError, error_buffer);
            NEXT_INST;
        }
        zlist_push(&STACK,p3);
        ip++; NEXT_INST;
    }
    CASE_CP COMPLEMENT:
    {
        orgk = ip - program;
        zlist_fastpop(&STACK,&p1);
        if (p1.type == Z_OBJ)
        {
            invokeOperator("__complement__", p1, 1, "~",NULL,true);
            NEXT_INST;
        }
        if (p1.type == Z_INT)
        {
            p2.type = Z_INT;
            p2.i = ~p1.i;
            zlist_push(&STACK,p2);
        }
        else if (p1.type == Z_INT64)
        {
            p2.type = Z_INT64;
            p2.l = ~p1.l;
            zlist_push(&STACK,p2);
        }
        else if (p1.type == Z_BYTE)
        {
            
            p2.type = Z_BYTE;
            uint8_t p = p1.i;
            p = ~p;
            p2.i = p; 
            zlist_push(&STACK,p2);
        }
        else
        {
            snprintf(error_buffer,100,"Operator '~' unsupported for type %s",fullform(p1.type));
            spitErr(TypeError, error_buffer);
            NEXT_INST;
        }
        ip++;
        NEXT_INST;
    }
    CASE_CP XOR:
    {
        orgk = ip - program;
        zlist_fastpop(&STACK,&p2);
        zlist_fastpop(&STACK,&p1);
        if (p1.type == Z_OBJ)
        {
            invokeOperator("__xor__", p1, 2, "^", &p2,true);
            NEXT_INST;
        }
        if (p1.type != p2.type)
        {
            spitErr(TypeError, "Error operands should have same type for operator '^' ");
            NEXT_INST;
        }
        p3.type = p1.type;
        if (p1.type == Z_INT)
        {
            p3.i = p1.i ^ p2.i;
        }
        else if (p1.type == Z_INT64)
        {
            p3.l = p1.l ^ p2.l;
        }
        else if (p1.type == Z_BYTE)
        {
            p3.i = (uint8_t)p1.i ^ (uint8_t)p2.i;
        }
        else
        { 
            snprintf(error_buffer,100,"Operator '^' unsupported for type %s",fullform(p1.type));
            spitErr(TypeError, error_buffer);
            NEXT_INST;
        }
        zlist_push(&STACK,p3);
        ip++; NEXT_INST;
    }
    CASE_CP ADD:
    {
        orgk = ip - program;
        zlist_fastpop(&STACK,&p2);
        zlist_fastpop(&STACK,&p1);
        if (p1.type == Z_OBJ)
        {
            invokeOperator("__add__", p1, 2, "+", &p2,true);
            NEXT_INST;
        }
        if(p1.type==p2.type)
            c1 = p1.type;
        else if (isNumeric(p1.type) && isNumeric(p2.type))
        {
            if (p1.type == Z_FLOAT || p2.type == Z_FLOAT)
            c1 = Z_FLOAT;
            else if (p1.type == Z_INT64 || p2.type == Z_INT64)
            c1 = Z_INT64;
            else if (p1.type == Z_INT || p2.type == Z_INT)
            c1 = Z_INT;
            PromoteType(&p1, c1);
            PromoteType(&p2, c1);
        }
        else
        { 
            snprintf(error_buffer,100,"Operator '+' unsupported for types %s and %s",fullform(p1.type),fullform(p2.type));
            spitErr(TypeError, error_buffer);
            NEXT_INST;
        }
        if (c1 == Z_INT)
        {
            p3.type = Z_INT;
            if (!addition_overflows_i32(p1.i, p2.i))
            {
                p3.i = p1.i + p2.i;
                STACK.arr[STACK.size++] = p3;
                ip++; NEXT_INST;
            }
            if (addition_overflows_i64((int64_t)p1.i, (int64_t)p2.i))
            {
                spitErr(OverflowError, "Integer Overflow occurred");
                NEXT_INST;
            }
            p3.type = Z_INT64;
            p3.l = (int64_t)(p1.i) + (int64_t)(p2.i);
            STACK.arr[STACK.size++] = p3;
        }
        else if (c1 == Z_INT64)
        {
            if (addition_overflows_i64(p1.l, p2.l))
            {
                orgk = ip - program;
                spitErr(OverflowError, "Error overflow during solving expression.");
                NEXT_INST;
            }
            p3.type = Z_INT64;
            p3.l = p1.l + p2.l;
            STACK.arr[STACK.size++] = p3;
            ip++; 
            NEXT_INST;
        }
        else if (c1 == Z_LIST)
        {
            p3.type = Z_LIST;
            zlist* a = (zlist*)p1.ptr;
            zlist* b = (zlist *)p2.ptr;
            zlist* res = vm_alloc_zlist();
            zlist_resize(res,a->size + b->size);
            memcpy(res->arr,a->arr,a->size*sizeof(zobject));
            memcpy(res->arr+a->size,b->arr,b->size*sizeof(zobject));
            p3.ptr = (void *)res;
            zlist_push(&STACK,p3);
            DoThreshholdBusiness();
        }
        else if (c1 == Z_STR )
        {
            zstr* a = (zstr*)p1.ptr;
            zstr* b = (zstr*)p2.ptr;
            
            zstr* p = vm_alloc_zstr(a->len + b->len);
            memcpy(p->val, a->val, a->len);
            memcpy(p->val + a->len, b->val, b->len);
            p3 = zobj_from_str_ptr(p);
            STACK.arr[STACK.size++] = p3;
            DoThreshholdBusiness();
        }
        else if (c1 == Z_FLOAT)
        {
            if (addition_overflows_i64(p1.f, p2.f))
            {
                orgk = ip - program;
                spitErr(OverflowError, "Floating point overflow during addition");
                NEXT_INST;
            }
            p3.type = Z_FLOAT;
            p3.f = p1.f + p2.f;
            STACK.arr[STACK.size++] = p3;
        }
        else
        { 
            snprintf(error_buffer,100,"Operator '+' unsupported for types %s and %s",fullform(p1.type),fullform(p2.type));
            spitErr(TypeError, error_buffer);
            NEXT_INST;
        }
        ip++; NEXT_INST;
    }
    CASE_CP SMALLERTHAN:
    {
        orgk = ip - program;
        p2 = STACK.arr[--STACK.size];
        p1 = STACK.arr[--STACK.size];
        if(p1.type == p2.type && isNumeric(p1.type))
            c1 = p1.type;
        else if (p1.type == Z_OBJ)
        {
            invokeOperator("__smallerthan__", p1, 2, "<", &p2,true);
            NEXT_INST;
        }
        else if (isNumeric(p1.type) && isNumeric(p2.type))
        {
            if (p1.type == Z_FLOAT || p2.type == Z_FLOAT)
            c1 = Z_FLOAT;
            else if (p1.type == Z_INT64 || p2.type == Z_INT64)
            c1 = Z_INT64;
            else if (p1.type == Z_INT || p2.type == Z_INT)
            c1 = Z_INT;
            PromoteType(&p1, c1);
            PromoteType(&p2, c1);
        }
        else
        {
            snprintf(error_buffer,100,"Operator '<' unsupported for types %s and %s",fullform(p1.type),fullform(p2.type));
            spitErr(TypeError, error_buffer);
            NEXT_INST;
        }
        p3.type = Z_BOOL;
        if (p1.type == Z_INT)
            p3.i = (bool)(p1.i < p2.i);
        else if (p1.type == Z_INT64)
            p3.i = (bool)(p1.l < p2.l);
        else if (p1.type == Z_FLOAT)
            p3.i = (bool)(p1.f < p2.f);
        
        STACK.arr[STACK.size++] = p3;
        ip++; NEXT_INST;
    }
    CASE_CP GREATERTHAN:
    {
        zlist_fastpop(&STACK,&p2);
        zlist_fastpop(&STACK,&p1);
        if(p1.type == p2.type && isNumeric(p1.type))
            c1=p1.type;
        else if (p1.type == Z_OBJ)
        {
            invokeOperator("__greaterthan__", p1, 2, ">", &p2,true);
            NEXT_INST;
        }
        else if (isNumeric(p1.type) && isNumeric(p2.type))
        {
            if (p1.type == Z_FLOAT || p2.type == Z_FLOAT)
                c1 = Z_FLOAT;
            else if (p1.type == Z_INT64 || p2.type == Z_INT64)
                c1 = Z_INT64;
            else if (p1.type == Z_INT || p2.type == Z_INT)
                c1 = Z_INT;
            PromoteType(&p1, c1);
            PromoteType(&p2, c1);
        }
        else
        {
            orgk = ip - program; 
            snprintf(error_buffer,100,"Operator '>' unsupported for types %s and %s",fullform(p1.type),fullform(p2.type));
            spitErr(TypeError, error_buffer);
            NEXT_INST;
        }
        p3.type = Z_BOOL;
        if (p1.type == Z_INT)
            p3.i = (bool)(p1.i > p2.i);
        else if (p1.type == Z_INT64)
            p3.i = (bool)(p1.l > p2.l);
        else if (p1.type == Z_FLOAT)
            p3.i = (bool)(p1.f > p2.f);
        

        STACK.arr[STACK.size++] = p3;
        ip++; NEXT_INST;
    }
    CASE_CP SMOREQ:
    {
        orgk = ip - program;
        zlist_fastpop(&STACK,&p2);
        zlist_fastpop(&STACK,&p1);
                
        if(p1.type == p2.type && isNumeric(p1.type))
            c1 = p1.type;
        else if (p1.type == Z_OBJ)
        {
            invokeOperator("__smallerthaneq__", p1, 2, "<=", &p2,true);
            NEXT_INST;
        }
        else if (isNumeric(p1.type) && isNumeric(p2.type))
        {
            if (p1.type == Z_FLOAT || p2.type == Z_FLOAT)
            c1 = Z_FLOAT;
            else if (p1.type == Z_INT64 || p2.type == Z_INT64)
            c1 = Z_INT64;
            else if (p1.type == Z_INT || p2.type == Z_INT)
            c1 = Z_INT;
            PromoteType(&p1, c1);
            PromoteType(&p2, c1);
        }
        else
        { 
            snprintf(error_buffer,100,"Operator '<=' unsupported for types %s and %s",fullform(p1.type),fullform(p2.type));
            spitErr(TypeError, error_buffer);
            NEXT_INST;
        }
        p3.type = Z_BOOL;
        if (p1.type == Z_INT)
            p3.i = (bool)(p1.i <= p2.i);
        else if (p1.type == Z_INT64)
            p3.i = (bool)(p1.l <= p2.l);
        else if (p1.type == Z_FLOAT)
            p3.i = (bool)(p1.f <= p2.f);
        
        STACK.arr[STACK.size++] = p3;
        ip++; NEXT_INST;
    }
    CASE_CP GROREQ:
    {
        orgk = ip - program;
        zlist_fastpop(&STACK,&p2);
        zlist_fastpop(&STACK,&p1);
        if(p1.type == p2.type && isNumeric(p1.type))
            c1 = p1.type;
        else if (p1.type == Z_OBJ)
        {
            invokeOperator("__greaterthaneq__", p1, 2, ">=", &p2,true);
            NEXT_INST;
        }
        else if (isNumeric(p1.type) && isNumeric(p2.type))
        {
            if (p1.type == Z_FLOAT || p2.type == Z_FLOAT)
            c1 = Z_FLOAT;
            else if (p1.type == Z_INT64 || p2.type == Z_INT64)
            c1 = Z_INT64;
            else if (p1.type == Z_INT || p2.type == Z_INT)
            c1 = Z_INT;
            PromoteType(&p1, c1);
            PromoteType(&p2, c1);
        }
        else
        {
            snprintf(error_buffer,100,"Operator '>=' unsupported for types %s and %s",fullform(p1.type),fullform(p2.type));
            spitErr(TypeError, error_buffer);
            NEXT_INST;
        }
        p3.type = Z_BOOL;
        if (p1.type == Z_INT)
            p3.i = (bool)(p1.i >= p2.i);
        else if (p1.type == Z_INT64)
            p3.i = (bool)(p1.l >= p2.l);
        else if (p1.type == Z_FLOAT)
            p3.i = (bool)(p1.f >= p2.f);
        
        STACK.arr[STACK.size++] = p3;
        ip++; NEXT_INST;
    }
    CASE_CP EQ:
    {
        orgk = ip - program;
        zlist_fastpop(&STACK,&p2);
        zlist_fastpop(&STACK,&p1);
        if (p1.type == Z_INT && p2.type == Z_INT64)
            PromoteType(&p1, Z_INT64);
        else if (p1.type == Z_INT64 && p2.type == Z_INT)
            PromoteType(&p2, Z_INT64);
        if (p1.type == Z_OBJ && p2.type != Z_NIL)
        {
            if(invokeOperator("__eq__", p1, 2, "==", &p2, 0))
            NEXT_INST;
        }
        p3.type = Z_BOOL;
        p3.i = (bool)(zobject_equals(p1,p2));
        STACK.arr[STACK.size++] = p3;
        ip++; NEXT_INST;
    }
    CASE_CP NOT:
    {
        zlist_fastpop(&STACK,&p1);
        if (p1.type == Z_OBJ)
        {
            orgk = ip-program;
            invokeOperator("__not__", p1, 1, "!",NULL,true);
            NEXT_INST;
        }
        if (p1.type != Z_BOOL)
        {
            orgk = ip - program;
            snprintf(error_buffer,100,"Operator '!' unsupported for type %s",fullform(p1.type));
            spitErr(TypeError, error_buffer);
            NEXT_INST;
        }
        p1.i = (bool)!(p1.i);
        STACK.arr[STACK.size++] = p1;
        ip++; NEXT_INST;
    }
    CASE_CP NEG:
    {
        zlist_fastpop(&STACK,&p1);
        if (p1.type == Z_OBJ)
        {
            orgk = ip - program;
            invokeOperator("__neg__", p1, 1, "-",NULL,true);
            NEXT_INST;
        }
        if (!isNumeric(p1.type))
        {
            orgk = ip - program;
            snprintf(error_buffer,100,"Unary operator '-' unsupported for type %s",fullform(p1.type));
            spitErr(TypeError, error_buffer);
            NEXT_INST;
        };
        if (p1.type == Z_INT)
        {
            if (p1.i != INT_MIN)
            p1.i = -(p1.i);
            else
            {
                p1.type = Z_INT64;
                p1.l = -(int64_t)(INT_MIN);
            }
        }
        else if (p1.type == Z_INT64)
        {
            if (p1.l == LLONG_MIN) // taking negative of LLONG_MIN results in LLONG_MAX+1 so we have to avoid it
            {
                orgk = ip - program;
                spitErr(OverflowError, "Error negation of INT64_MIN causes overflow!");
                NEXT_INST;
            }
            else if (-p1.l == INT_MIN)
            {
                p1.type = Z_INT;
                p1.i = INT_MIN;
            }
            else
            p1.l = -p1.l;
        }
        else if (p1.type == Z_FLOAT)
        {
            p1.f = -p1.f;
        }
        STACK.arr[STACK.size++] = p1;
        ip++; NEXT_INST;
    }
    CASE_CP INDEX:
    {
        p1 = STACK.arr[--STACK.size];
        p2 = STACK.arr[--STACK.size];
        if (p2.type == Z_LIST)
        {
            if (p1.type != Z_INT && p1.type != Z_INT64)
            {
                orgk = ip - program;
                spitErr(TypeError, "Error index should be integer!");
                NEXT_INST;
            }
            PromoteType(&p1, Z_INT64);
            pl_ptr1 = (zlist *)p2.ptr;
            if (p1.l < 0 || p1.l >= pl_ptr1->size)
            {
                orgk = ip - program;
                spitErr(IndexError, "Index out of range!");
                NEXT_INST;
            }
            STACK.arr[STACK.size++] = pl_ptr1->arr[p1.l];
            ip++; 
            NEXT_INST;
        }
        else if (p2.type == Z_BYTEARR)
        {
            if (p1.type != Z_INT && p1.type != Z_INT64)
            {
                orgk = ip - program;
                spitErr(TypeError, "Error index should be integer!");
                NEXT_INST;
            }
            PromoteType(&p1, Z_INT64);
            if (p1.l < 0)
            {
                orgk = ip - program;
                spitErr(ValueError, "Error index cannot be negative!");
                NEXT_INST;
            }
            bt_ptr1 = (zbytearr*)p2.ptr;
            if ((size_t)p1.l >= bt_ptr1->size)
            {
                orgk = ip - program;
                spitErr(ValueError, "Error index is out of range!");
                NEXT_INST;
            }
            p3.type = 'm';
            p3.i = bt_ptr1->arr[p1.l];
            STACK.arr[STACK.size++] = p3;
            ip++; NEXT_INST;
        }
        else if (p2.type == Z_DICT)
        {
            zdict* d = (zdict *)p2.ptr;
            zobject res;
            if (!zdict_get(d,p1,&res))
            {
                orgk = ip - program;
                spitErr(KeyError, "Error key not found in the dictionary!");
                NEXT_INST;
            }
            STACK.arr[STACK.size++] = res;
        }
        else if (p2.type == Z_STR )
        {
            if (p1.type != Z_INT && p1.type != Z_INT64)
            {
            orgk = ip - program;
            spitErr(TypeError, "Error index for string should be an integer!");
            NEXT_INST;
            }
            PromoteType(&p1, Z_INT64);
            if (p1.l < 0)
            {
                orgk = ip - program;
                spitErr(ValueError, "Error index cannot be negative!");
                NEXT_INST;
            }
            char* s;
            size_t length;
            s = ((zstr*)p2.ptr) -> val;
            length = ((zstr*)p2.ptr) -> len;
            if ((size_t)p1.l >= length)
            {
                orgk = ip - program;
                spitErr(ValueError, "Error index is out of range!");
            }
            char c = s[p1.l];
            zstr* p = vm_alloc_zstr(1);
            p->val[0] = c;
            //zlist_push(&STACK,zobj_from_str_ptr(p));
            STACK.arr[STACK.size++] = zobj_from_str_ptr(p);
            DoThreshholdBusiness();
        }
        else if (p2.type == Z_OBJ)
        {
            orgk = ip - program;
            invokeOperator("__index__", p2, 2, "[]", &p1,true);
            NEXT_INST;
        }
        else
        {
            orgk = ip - program;
            snprintf(error_buffer,100,"Cannot index type %s",fullform(p2.type));
            spitErr(TypeError, error_buffer);
            NEXT_INST;
        }
        ip++;
        NEXT_INST;
    }
    CASE_CP INDEX_FAST:
    {
        bool a = *(++ip);
        bool b = *(++ip);
        ip+=1;
        memcpy(&i1,ip,4);
        ip+=4;
        memcpy(&i2,ip,4);
        ip+=4;
        //printf("a = %d, b = %d, i1 = %d, i2 = %d\n",a,b,i1,i2);
        i1 = a ? i1 : VEC_LAST(frames)+i1;
        i2 = b ? i2 : VEC_LAST(frames)+i2;

        p2 = STACK.arr[i1];
        p1 = STACK.arr[i2];

        if (p2.type == Z_LIST)
        {
            if (p1.type != Z_INT && p1.type != Z_INT64)
            {
                orgk = ip - program;
                spitErr(TypeError, "Error index should be integer!");
                NEXT_INST;
            }
            PromoteType(&p1, Z_INT64);
            pl_ptr1 = (zlist *)p2.ptr;
            if (p1.l < 0 || p1.l >= pl_ptr1->size)
            {
                orgk = ip - program;
                spitErr(IndexError, "Index out of range!");
                NEXT_INST;
            }
            zlist_push(&STACK,(pl_ptr1)->arr[p1.l]);
            //STACK.arr[STACK.size++] = pl_ptr1->arr[p1.l];
            NEXT_INST;
        }
        else if (p2.type == Z_BYTEARR)
        {
            if (p1.type != Z_INT && p1.type != Z_INT64)
            {
                orgk = ip - program;
                spitErr(TypeError, "Error index should be integer!");
                NEXT_INST;
            }
            PromoteType(&p1, Z_INT64);
            if (p1.l < 0)
            {
                orgk = ip - program;
                spitErr(ValueError, "Error index cannot be negative!");
                NEXT_INST;
            }
            bt_ptr1 = (zbytearr*)p2.ptr;
            if ((size_t)p1.l >= bt_ptr1->size)
            {
                orgk = ip - program;
                spitErr(ValueError, "Error index is out of range!");
                NEXT_INST;
            }
            p3.type = 'm';
            p3.i = bt_ptr1->arr[p1.l];
            zlist_push(&STACK,p3);
            NEXT_INST;
        }
        else if (p2.type == Z_DICT)
        {
            zdict* d = (zdict *)p2.ptr;
            zobject res;
            if (!zdict_get(d,p1,&res))
            {
                orgk = ip - program;
                spitErr(KeyError, "Key not found in dictionary");
                NEXT_INST;
            }
            zlist_push(&STACK,res);
        }
        else if (p2.type == Z_STR )
        {
            if (p1.type != Z_INT && p1.type != Z_INT64)
            {
                orgk = ip - program;
                spitErr(TypeError, "Error index for string should be an integer!");
                NEXT_INST;
            }
            PromoteType(&p1, Z_INT64);
            if (p1.l < 0)
            {
                orgk = ip - program;
                spitErr(ValueError, "Error index cannot be negative!");
                NEXT_INST;
            }
            char* s;
            size_t length;
            s = ((zstr*)p2.ptr) -> val;
            length = ((zstr*)p2.ptr) -> len;
            if ((size_t)p1.l >= length)
            {
                orgk = ip - program;
                spitErr(ValueError, "Error index is out of range!");
            }
            char c = s[p1.l];
            zstr* p = vm_alloc_zstr(1);
            p->val[0] = c;
            zlist_push(&STACK,zobj_from_str_ptr(p));
            //STACK.arr[STACK.size++] = zobj_from_str_ptr(p);
            DoThreshholdBusiness();
        }
        else if (p2.type == Z_OBJ)
        {
            orgk = ip - program;
            invokeOperator("__index__", p2, 2, "[]", &p1,true);
            NEXT_INST;
        }
        else
        {
            orgk = ip - program;
            snprintf(error_buffer,100,"Cannot index type %s",fullform(p2.type));
            spitErr(TypeError,error_buffer);
            NEXT_INST;
        }
        NEXT_INST;
    }
    CASE_CP LOADVAR_ADDINT32:
    {
        orgk = ip - program;
        bool a = *(++ip);
        memcpy(&i1,ip+1,4);
        ip+=5;
        memcpy(&i2,ip,4);
        ip+=4;
        i1 = a ? i1 : VEC_LAST(frames)+i1;
        p1 = STACK.arr[i1];
        if (p1.type == Z_OBJ)
        {
            invokeOperator("__add__", p1, 2, "+", &p2,true);
            NEXT_INST;
        }
        if(p1.type == Z_INT)
        {
            p3.type = Z_INT;
            if (!addition_overflows_i64(p1.i, i2))
            {
                p3.i = p1.i + i2;
                zlist_push(&STACK,p3);
                NEXT_INST;
            }
            if (addition_overflows_i64((int64_t)p1.i, (int64_t)i2))
            {
                spitErr(OverflowError, "Integer Overflow occurred");
                NEXT_INST;
            }
            p3.type = Z_INT64;
            p3.l = (int64_t)(p1.i) + (int64_t)(i2);
            zlist_push(&STACK,p3);
        }
        else if(p1.type == Z_INT64)
        {
            if (addition_overflows_i64(p1.l, (int64_t)i2))
            {
                orgk = ip - program;
                spitErr(OverflowError, "Error overflow during solving expression.");
                NEXT_INST;
            }
            p3.type = Z_INT64;
            p3.l = p1.l + i2;
            zlist_push(&STACK,p3);
            NEXT_INST;
        }
        else if(p1.type == Z_FLOAT)
        {
            if (addition_overflows_double(p1.f, (double)i2))
            {
                orgk = ip - program;
                spitErr(OverflowError, "Floating point overflow during addition");
                NEXT_INST;
            }
            p3.type = Z_FLOAT;
            p3.f = p1.f + i2;
            zlist_push(&STACK,p3);
        }
        else
        {
            snprintf(error_buffer,100,"Operator '+' unsupported for types %s and %s",fullform(p1.type),fullform(p2.type));
            spitErr(TypeError, error_buffer);
            NEXT_INST;
        }
        NEXT_INST;
    }
    CASE_CP LOADVAR_SUBINT32:
    {
        orgk = ip - program;
        bool a = *(++ip);
        memcpy(&i1,ip+1,4);
        ip+=5;
        memcpy(&i2,ip,4);
        ip+=4;
        i1 = a ? i1 : VEC_LAST(frames)+i1;
        p1 = STACK.arr[i1];
        if (p1.type == Z_OBJ)
        {
            invokeOperator("__sub__", p1, 2, "-", &p2,true);
            NEXT_INST;
        }
        if(p1.type == Z_INT)
        {
            p3.type = Z_INT;
            if (!subtraction_overflows_i32(p1.i, i2))
            {
                p3.i = p1.i - i2;
                zlist_push(&STACK,p3);
                NEXT_INST;
            }
            if (subtraction_overflows_i64((int64_t)p1.i, (int64_t)i2))
            {
                spitErr(OverflowError, "Integer Overflow occurred");
                NEXT_INST;
            }
            p3.type = Z_INT64;
            p3.l = (int64_t)(p1.i) - (int64_t)(i2);
            zlist_push(&STACK,p3);
        }
        else if(p1.type == Z_INT64)
        {
            if (subtraction_overflows_i64(p1.l, (int64_t)i2))
            {
                orgk = ip - program;
                spitErr(OverflowError, "Error overflow during solving expression.");
                NEXT_INST;
            }
            p3.type = Z_INT64;
            p3.l = p1.l - i2;
            zlist_push(&STACK,p3);
            NEXT_INST;
        }
        else if(p1.type == Z_FLOAT)
        {
            if (subtraction_overflows_double(p1.f, (double)i2))
            {
                orgk = ip - program;
                spitErr(OverflowError, "Floating point overflow during subtraction");
                NEXT_INST;
            }
            p3.type = Z_FLOAT;
            p3.f = p1.f - i2;
            zlist_push(&STACK,p3);
        }
        else
        { 
            snprintf(error_buffer,100,"Operator '-' unsupported for types %s and %s",fullform(p1.type),fullform(p2.type));
            spitErr(TypeError, error_buffer);
            NEXT_INST;
        }
        NEXT_INST;
    }
    CASE_CP NOTEQ:
    {
        zlist_fastpop(&STACK,&p2);
        zlist_fastpop(&STACK,&p1);
        if (p1.type == Z_OBJ && p2.type != Z_NIL)
        {
            invokeOperator("__noteq__", p1, 2, "!=", &p2, 0);
            NEXT_INST;
        }
        p3.type = Z_BOOL;
        if (p1.type == Z_INT && p2.type == Z_INT64)
            PromoteType(&p1, Z_INT64);
        else if (p1.type == Z_INT64 && p2.type == Z_INT)
            PromoteType(&p2, Z_INT64);
        p3.i = (bool)!(zobject_equals(p1,p2));
        //zlist_push(&STACK,p3);
        STACK.arr[STACK.size++] = p3;
        ip++; NEXT_INST;
    }
    CASE_CP IS:
    {
        zlist_fastpop(&STACK,&p2);
        zlist_fastpop(&STACK,&p1);
        if ((p1.type != Z_DICT && p1.type != Z_CLASS && p1.type != Z_LIST && p1.type != Z_OBJ && p1.type != Z_STR && p1.type != Z_MODULE && p1.type != Z_FUNC) || (p2.type != Z_CLASS && p2.type != Z_DICT && p2.type != Z_STR && p2.type != Z_FUNC && p2.type != Z_LIST && p2.type != Z_OBJ && p2.type != Z_MODULE))
        {
            orgk = ip - program;
            snprintf(error_buffer,100,"Operator 'is' unsupported for types %s and %s",fullform(p1.type),fullform(p2.type));
            spitErr(TypeError, error_buffer);
            NEXT_INST;
        }
        p3.type = Z_BOOL;
        p3.i = (bool)(p1.ptr == p2.ptr);
        STACK.arr[STACK.size++] = p3;
        ip++; NEXT_INST;
    }
    CASE_CP MUL:
    {
        zlist_fastpop(&STACK,&p2);
        zlist_fastpop(&STACK,&p1);
        if (p1.type == Z_OBJ)
        {
            invokeOperator("__mul__", p1, 2, "*", &p2,true);
            NEXT_INST;
        }
        zobject c;
        char t;
        if(p1.type==p2.type && isNumeric(p1.type))
            t = p1.type;
        else if (isNumeric(p1.type) && isNumeric(p2.type))
        {
            if (p1.type == Z_FLOAT || p2.type == Z_FLOAT)
            t = Z_FLOAT;
            else if (p1.type == Z_INT64 || p2.type == Z_INT64)
            t = Z_INT64;
            else if (p1.type == Z_INT || p2.type == Z_INT)
            t = Z_INT;
            PromoteType(&p1, t);
            PromoteType(&p2, t);
        }
        else if(p1.type==Z_LIST && (p2.type==Z_INT || p2.type == Z_INT64))
        {
            PromoteType(&p2,Z_INT64);
            if(p2.l < 0)
            {
            orgk = ip - program;
            spitErr(ValueError,"Cannot multiply list by a negative integer!");
            NEXT_INST;
            }
            zlist* src = (zlist*)p1.ptr;
            zlist* res = vm_alloc_zlist();
            if(src->size!=0)
            {
            for(size_t i=1;i<=(size_t)p2.l;i++)
            {
                size_t idx = res->size;
                zlist_resize(res,res->size+src->size);
                memcpy(res->arr+idx,src->arr,src->size*sizeof(zobject));
            }
            }
            p1.type = Z_LIST;
            p1.ptr = (void*)res;
            STACK.arr[STACK.size++] = p1;
            DoThreshholdBusiness();
            ++ip;
            NEXT_INST;
        }
        else if(p1.type==Z_STR && (p2.type==Z_INT || p2.type == Z_INT64))
        {
            PromoteType(&p2,Z_INT64);
            if(p2.l < 0)
            {
                orgk = ip - program;
                spitErr(ValueError,"Cannot multiply string by a negative integer!");
                NEXT_INST;
            }
            zstr* src = (zstr*)p1.ptr;
            zstr* res = vm_alloc_zstr(src->len * p2.l);
            if(src->len != 0)
            {
            for(size_t i=0;i<(size_t)p2.l;i++)
                memcpy(res->val+(i*src->len), src->val,src->len);
            }
            p1.type = Z_STR;
            p1.ptr = (void*)res;
            STACK.arr[STACK.size++] = p1;
            DoThreshholdBusiness();
            ++ip;
            NEXT_INST;
        }
        else
        {
            orgk = ip - program;
            snprintf(error_buffer,100,"Operator '*' unsupported for types %s and %s",fullform(p1.type),fullform(p2.type));
            spitErr(TypeError, error_buffer);
            NEXT_INST;
        }

        if (t == Z_INT)
        {
            c.type = Z_INT;
            if (!multiplication_overflows_i32(p1.i, p2.i))
            {
                c.i = p1.i * p2.i;
                //zlist_push(&STACK,c);
                STACK.arr[STACK.size++] = c;
                ip++; NEXT_INST;
            }
            orgk = ip - program;
            if (multiplication_overflows_i64((int64_t)p1.i, (int64_t)p2.i))
            {
                spitErr(OverflowError, "Overflow occurred");
                NEXT_INST;
            }
            c.type = Z_INT64;
            c.l = (int64_t)(p1.i) * (int64_t)(p2.i);
            //zlist_push(&STACK,c);
            STACK.arr[STACK.size++] = c;
        }
        else if (t == Z_FLOAT)
        {
            if (multiplication_overflows_double(p1.f, p2.f))
            {
            orgk = ip - program;
            spitErr(OverflowError, "Floating point overflow during multiplication");
            NEXT_INST;
            }
            c.type = Z_FLOAT;
            c.f = p1.f * p2.f;
            //zlist_push(&STACK,c);
            STACK.arr[STACK.size++] = c;
        }
        else if (t == Z_INT64)
        {
            if (multiplication_overflows_i64(p1.l, p2.l))
            {
                orgk = ip - program;
                spitErr(OverflowError, "Error overflow during solving expression.");
                NEXT_INST;
            }
            c.type = Z_INT64;
            c.l = p1.l * p2.l;
            //zlist_push(&STACK,c);
            STACK.arr[STACK.size++] = c;
        }
        ip++; NEXT_INST;
    }
    CASE_CP MEMB:
    {
        orgk = ip - program;
        zobject a;
        zlist_fastpop(&STACK,&a);
        ++ip;
        memcpy(&i1, ip, sizeof(int32_t));
        ip += 3;
        const char* mname = ((zstr*)vm_strings.arr[i1])->val;
        
        if (a.type == Z_MODULE)
        {
            zmodule* m = (zmodule*)a.ptr;
            
            if (!StrMap_get(&(m->members),mname,&p3))
            {
                snprintf(error_buffer,100,"Module has no member named %s",mname);
                spitErr(NameError,error_buffer);
                NEXT_INST;
            }
            STACK.arr[STACK.size++] = p3;
            ++ip;
            NEXT_INST;
        }
        else if(a.type == Z_OBJ)
        {
            zclass_object*ptr = (zclass_object*)a.ptr;
            zobject tmp;
            if (!StrMap_get(&(ptr->members),mname,&tmp))
            {
                snprintf(error_buffer,100,"Object has no member named %s",mname);
                spitErr(NameError,error_buffer);
                NEXT_INST;
            }
            STACK.arr[STACK.size++] = tmp;
        }
        else if(a.type == Z_CLASS)
        {
            zclass* ptr = (zclass* )a.ptr;
            zobject tmp;
            if (!StrMap_get(&(ptr->members),mname,&tmp))
            {
                snprintf(error_buffer,100,"Class has no member named %s",mname);
                spitErr(NameError, error_buffer);
                NEXT_INST;
            }
            STACK.arr[STACK.size++] = tmp;
        }
        else
        {
            snprintf(error_buffer,100, "Member operator unsupported for type %s",fullform(a.type));
            spitErr(TypeError,error_buffer);
            NEXT_INST;
        }
        ip++; NEXT_INST;
    }
    CASE_CP LOAD_FUNC:
    {
        ip += 1;
        int32_t p;
        memcpy(&p, ip, sizeof(int32_t));
        ip += 4;
        int32_t idx;
        memcpy(&idx, ip, sizeof(int32_t));
        ip += 4;
        zfun *fn = vm_alloc_zfun();
        fn->i = p;
        fn->args = *ip;
        fn->name = ((zstr*)vm_strings.arr[idx])->val;
        p1.type = Z_FUNC;
        p1.ptr = (void *)fn;
        ip++;
        i2 = (int32_t)*ip;
        zlist_resize(&(fn -> opt),i2);
        memcpy(fn->opt.arr,STACK.arr + STACK.size-i2,sizeof(zobject)*i2);
        zlist_erase_range(&STACK,STACK.size - i2 , STACK.size - 1);
        zlist_push(&STACK,p1);
        DoThreshholdBusiness();
        ip++; NEXT_INST;
    }
    CASE_CP LOAD_CO:
    {
        ip += 1;
        int32_t p;
        memcpy(&p, ip, sizeof(int32_t));
        ip += 4;
        int32_t idx;
        memcpy(&idx, ip, sizeof(int32_t));
        ip += 4;
        zobject co;
        zfun* fn = vm_alloc_coro();
        fn->args = *ip;
        fn->i = p;
        fn->name = "Coroutine";
        fn->_klass = NULL;
        co.type = 'g';
        co.ptr = (void*)fn;
        zlist_push(&STACK,co);
        ip++; NEXT_INST;
    }
    CASE_CP BUILD_CLASS:
    {
        ip += 1;
        int32_t N;
        memcpy(&N, ip, sizeof(int32_t));
        ip += 4;
        int32_t idx;
        memcpy(&idx, ip, sizeof(int32_t));
        ip += 3;
        zobject zclass_packed;
        zclass_packed.type = Z_CLASS;
        zclass* obj = vm_alloc_zclass();
        obj->name = ((zstr*)vm_strings.arr[idx])->val;
        values.size = 0;
        names.size = 0;
        for (int32_t i = 1; i <= N; i++)
        {
            zlist_fastpop(&STACK,&p1);
            if (p1.type == Z_FUNC)
            {
            zfun *ptr = (zfun *)p1.ptr;
            ptr->_klass = obj;
            }
            zlist_push(&values,p1);
        }
        for (int32_t i = 1; i <= N; i++)
        {
            zlist_fastpop(&STACK,&p1);
            zlist_push(&names,p1);
        }
        for (int32_t i = 0; i < N; i += 1)
        {
            char* propName = ((zstr*)names.arr[i].ptr)->val;
            StrMap_emplace(&(obj->members),propName,values.arr[i]);
        }
        zclass_packed.ptr = (void *)obj;
        zlist_push(&STACK,zclass_packed);
        ip++; NEXT_INST;
    }
    CASE_CP BUILD_DERIVED_CLASS:
    {
        orgk = ip - program;
        ip += 1;
        int32_t N;
        memcpy(&N, ip, sizeof(int32_t));
        ip += 4;
        memcpy(&i1, ip, sizeof(int32_t));
        ip += 3;
        // N is total new class members
        // i1 is idx of class name in strings array
        zobject zclass_packed;
        zclass_packed.type = Z_CLASS;
        zclass* d = vm_alloc_zclass();
        d->name = ((zstr*)vm_strings.arr[i1])->val; // strings are not deallocated until exit, so no problem
        names.size = 0;
        values.size = 0;
        for (int32_t i = 1; i <= N; i++)
        {
            zlist_fastpop(&STACK,&p1);
            if (p1.type == Z_FUNC)
            {
                zfun *ptr = (zfun *)p1.ptr;
                ptr->_klass = d;
            }
            zlist_push(&values,p1);
        }
        for (int32_t i = 1; i <= N; i++)
        {
            zlist_fastpop(&STACK,&p1);
            zlist_push(&names,p1);
        }
        zobject baseClass;
        zlist_fastpop(&STACK,&baseClass);
        if (baseClass.type != Z_CLASS)
        {
            spitErr(TypeError, "Error class can not be derived from object of non class type!");
            NEXT_INST;
        }
        zclass* Base = (zclass* )baseClass.ptr;

        for (int32_t i = 0; i < N; i += 1)
        {
            char* propName = ((zstr*)names.arr[i].ptr) -> val;
            StrMap_emplace(&(d->members),propName,values.arr[i]);
        }

        for (size_t it = 0;it < Base->members.capacity;it++)
        {
            if(Base->members.table[it].stat != SM_OCCUPIED)
                continue;
            SM_Slot e = Base->members.table[it];
            const char* n = e.key;
            if (strcmp(n , "super") == 0)//do not add base class's super to this class
                continue;
            zobject* ref1;
            if (!(ref1 = StrMap_getRef(&(d->members),n)))//member is not overriden in child
            {
                p1 = e.val;
                if (p1.type == Z_FUNC)
                {
                        zfun *p = (zfun *)p1.ptr;
                        zfun *rep = vm_alloc_zfun();
                        rep->args = p->args;
                        rep->i = p->i;
                        rep->_klass = d;
                        rep->name = p->name;
                        zlist_resize(&(rep->opt),p->opt.size);
                        memcpy(rep->opt.arr,p->opt.arr,sizeof(zobject)*p->opt.size);
                        p1.type = Z_FUNC;
                        p1.ptr = (void *)rep;
                }
                StrMap_set(&(d->members),e.key, p1); //override
            }
        }
        
        StrMap_emplace(&(d->members),"super",baseClass);
        zclass_packed.ptr = (void *)d;
        zlist_push(&STACK,zclass_packed);
        ip++; NEXT_INST;
    }
    CASE_CP JMPIFFALSENOPOP:
    {
        ip += 1;
        memcpy(&i1, ip, sizeof(int32_t));
        ip += 3;
        p1 = STACK.arr[STACK.size - 1];
        if (p1.type == Z_NIL || (p1.type == Z_BOOL && p1.i == 0))
        {
            ip = ip + i1 + 1;
            NEXT_INST;
        }
        zlist_fastpop(&STACK,&p1);
        ip++; NEXT_INST;
    }
    CASE_CP NOPOPJMPIF:
    {
        ip += 1;
        memcpy(&i1, ip, sizeof(int32_t));
        ip += 3;
        p1 = STACK.arr[STACK.size - 1];
        if (p1.type == Z_NIL || (p1.type == Z_BOOL && p1.i == 0))
        {
            zlist_fastpop(&STACK,&p1);
            ip++; NEXT_INST;
        }
        else
        {
            ip = ip + i1 + 1;
            NEXT_INST;
        }
    }
    CASE_CP LOAD_STR:
    {
        ip += 1;
        memcpy(&i1, ip, sizeof(int32_t));
        ip += 3;
        p1.type = Z_STR;
        p1.ptr = (void *)vm_strings.arr[i1];
        zlist_push(&STACK,p1);
        ip++; NEXT_INST;
    }
    CASE_CP CALLUDF:
    {
        orgk = ip - program;
        zobject fn;
        zlist_fastpop(&STACK,&fn);
        int32_t N = *(++ip);
        if (fn.type == Z_FUNC)
        {
            zfun *obj = (zfun *)fn.ptr;
            if ((size_t)N + obj->opt.size < obj->args || (size_t)N > obj->args)
            {
                //spitErr(ArgumentError, "Error function " + (string)obj->name + " takes " + to_string(obj->args) + " arguments," + to_string(N) + " given!");
                NEXT_INST;
            }
            ptr_vector_push(&callstack,ip + 1);
            sizet_vector_push(&frames,STACK.size - N);
            if (callstack.size >= 1000)
            {
                spitErr(MaxRecursionError, "Error max recursion limit 1000 reached.");
                NEXT_INST;
            }
            for (size_t i = obj->opt.size - (obj->args - N); i < obj->opt.size; i++)
                zlist_push(&STACK,obj->opt.arr[i]);
            ptr_vector_push(&executing,obj);
            ip = program + obj->i;
            NEXT_INST;
        }
        else if (fn.type == Z_NATIVE_FUNC)
        {
            znativefun *A = (znativefun *)fn.ptr;
            NativeFunPtr f = A->addr;
            p4.type = Z_NIL;
            zobject* args = STACK.arr + (STACK.size-N);
            if(A->signature)
            {
            size_t len = strlen(A->signature);
            if(len != N)
            {
                //spitErr(ArgumentError,(string)"Native function "+ (string)A->name + (string)" takes "+to_string(len)+" arguments, "+to_string(N)+" given!");
                NEXT_INST;
            }
            size_t i = 0;
            while(i<N)
            {
                if(args[i].type != A->signature[i])
                {
                    //spitErr(TypeError,"Argument "+to_string(i+1)+" to "+(string)A->name+(string)+"should be a "+(std::string)fullform(A->signature[i]));
                    NEXT_INST;
                }
                i+=1;
            }
            }

            p4 = f(&(STACK.arr[STACK.size - N]), N);
            if (p4.type == Z_ERROBJ)
            {
                //s1 = "Native Function:  " + *(string *)p4.ptr;
                //spitErr((zclass*)p4.ptr, s1);
                NEXT_INST;
            }
            if (strcmp(fullform(p4.type),"Unknown")==0 && p4.type != Z_NIL)
            {
                spitErr(ValueError, "Error invalid response from module!");
                NEXT_INST;
            }
            zlist_erase_range(&STACK,STACK.size-N,STACK.size-1);
            zlist_push(&STACK,p4);
        }
        else if (fn.type == Z_CLASS)
        {
            zclass_object* obj = vm_alloc_zclassobj(AS_CLASS(fn)); // instance of class
            const char* s1 = ((zclass*)fn.ptr) -> name;
            zobject construct;
            if (StrMap_get(&(obj->members),"__construct__", &construct))
            {
                if (construct.type == Z_FUNC)
                {
                    zfun* p = (zfun *)construct.ptr;
                    if ((size_t)N + p->opt.size + 1 < p->args || (size_t)N + 1 > p->args)
                    {
                        snprintf(error_buffer,100,"Constructor takes %zu arguments, %d given!",p->args - 1,N);
                        spitErr(ArgumentError,error_buffer);
                        NEXT_INST;
                    }
                    zobject r;
                    r.type = Z_OBJ;
                    r.ptr = (void *)obj;
                    ptr_vector_push(&callstack,ip + 1);
                    zlist_insert(&STACK,STACK.size - N,r);
                    sizet_vector_push(&frames,STACK.size-N-1);
                    for (size_t i = p->opt.size - (p->args - 1 - N); i < p->opt.size; i++)
                    {
                        zlist_push(&STACK,p->opt.arr[i]);
                    }
                    
                    ip = program + p->i;
                    ptr_vector_push(&executing,p);
                    DoThreshholdBusiness();
                    NEXT_INST;
                }
            else if (construct.type == Z_NATIVE_FUNC)
            {
                znativefun *M = (znativefun *)construct.ptr;
                zobject *args = NULL;
                zobject r;
                r.type = Z_OBJ;
                r.ptr = (void *)obj;
                zlist_insert(&STACK,STACK.size - N,r);
                args = &STACK.arr[STACK.size - (N + 1)];
                if(M->signature)
                {
                    size_t len = strlen(M->signature);
                    if(len != N+1)
                    {
                        //spitErr(ArgumentError,(string)"Native function "+ (string)M->name + (string)" takes "+to_string(len)+" arguments, "+to_string(N+1)+" given!");
                        NEXT_INST;
                    }
                    size_t i = 0;
                    while(i<N+1)
                    {
                        if(args[i].type != M->signature[i])
                        {
                            //spitErr(TypeError,"Argument "+to_string(i+1)+" to "+(string)M->name+(string)+"should be a "+(std::string)fullform(M->signature[i]));
                            NEXT_INST;
                        }
                        i+=1;
                    }
                }
                p4 = M->addr(args, N + 1);
                zlist_erase_range(&STACK,STACK.size-(N+1),STACK.size-1);
                if (p4.type == Z_ERROBJ)
                {
                    zclass_object* E = (zclass_object*)p4.ptr;
                    zobject msg;
                    StrMap_get(&(E->members),"msg",&msg);
                    char* errmsg = ((zstr*)msg.ptr) -> val;
                    //spitErr((zclass*)p4.ptr, s1+ "." + "__construct__:  " + errmsg);
                    NEXT_INST;
                }
                zlist_push(&STACK,r);
                DoThreshholdBusiness();
                ip++;
                NEXT_INST;
            }
            else
            {
                //spitErr(TypeError, "Error constructor of class " + (string)((zclass* )fn.ptr)->name + " is not a function!");
                NEXT_INST;
            }
            }
            else
            {
            if (N != 0)
            {
                //spitErr(ArgumentError, "Error constructor class " + (string)((zclass* )fn.ptr)->name + " takes 0 arguments!");
                NEXT_INST;
            }
            }
            zobject r;
            r.type = Z_OBJ;
            r.ptr = (void *)obj;
            zlist_push(&STACK,r);
            DoThreshholdBusiness();
        }
        else if (fn.type == 'g')
        {
            zfun* f = (zfun*)fn.ptr;
            if ((size_t)N != f->args)
            {
                //spitErr(ArgumentError, "Error coroutine " + *(string *)fn.ptr + " takes " + to_string(f->args) + " arguments," + to_string(N) + " given!");
                NEXT_INST;
            }
            Coroutine *g = vm_alloc_coro_obj();
            g->fun = f;
            g->curr = f->i;
            g->state = SUSPENDED;
            zobject* locals = STACK.arr + STACK.size - N;
            STACK.size -= N;
            zlist_resize(&(g->locals),N);
            if(N != 0)
                memcpy(g->locals.arr,locals,N*sizeof(zobject));
            g->giveValOnResume = false;
            zobject T;
            T.type = Z_COROUTINE_OBJ;
            T.ptr = g;
            zlist_push(&STACK,T);
            DoThreshholdBusiness();
        }
        else
        {
            //spitErr(TypeError, "Error type " +(std::string)fullform(fn.type) + " not callable!");
            NEXT_INST;
        }
        ip++; NEXT_INST;
    }
    CASE_CP CALL_DIRECT:
    {
        memcpy(&i1,++ip, 4);
        ip+=4;
        p1 = STACK.arr[i1];
        zfun* fn = (zfun*)p1.ptr;
        ptr_vector_push(&callstack,ip);
        sizet_vector_push(&frames,STACK.size - fn->args);
        if (callstack.size >= 1000)
        {
            spitErr(MaxRecursionError, "Error max recursion limit 1000 reached.");
            NEXT_INST;
        }
        ptr_vector_push(&executing,fn);
        ip = program + fn->i;
        NEXT_INST;
    }
    CASE_CP MOD:
    {
        zobject a,b;
        zlist_fastpop(&STACK,&b);
        zlist_fastpop(&STACK,&a);
        if (a.type == Z_OBJ)
        {
            invokeOperator("__mod__", a, 2, "%", &b,true);
            NEXT_INST;
        }
        zobject c;
        char t;
        if (isNumeric(a.type) && isNumeric(b.type))
        {
            if (a.type == Z_FLOAT || b.type == Z_FLOAT)
            {
                orgk = ip - program;
                spitErr(TypeError, "Error modulo operator % unsupported for floats!");
                NEXT_INST;
            }
            else if (a.type == Z_INT64 || b.type == Z_INT64)
            {
            t = Z_INT64;
            }
            else if (a.type == Z_INT || b.type == Z_INT)
            t = Z_INT;
            PromoteType(&a, t);
            PromoteType(&b, t);
        }
        else
        {
            orgk = ip - program;
            //spitErr(TypeError, "Error operator '%' unsupported for " +(std::string)fullform(a.type) + " and " +(std::string)fullform(b.type));
            NEXT_INST;
        }
        //

        if (t == Z_INT)
        {
            c.type = Z_INT;
            if (b.i == 0)
            {
                orgk = ip - program;
                spitErr(MathError, "Error modulo by zero");
                NEXT_INST;
            }
            if ((a.i == INT_MIN) && (b.i == -1))
            {
                /* handle error condition */
                c.type = Z_INT64;
                c.l = (int64_t)a.i % (int64_t)b.i;
                zlist_push(&STACK,c);
            }
            c.i = a.i % b.i;
            zlist_push(&STACK,c);
        }
        else if (t == Z_INT64)
        {
            c.type = Z_INT64;
            if (b.l == 0)
            {
            orgk = ip - program;
            spitErr(MathError, "Error modulo by zero");
            NEXT_INST;
            }
            if ((a.l == LLONG_MIN) && (b.l == -1))
            {
                orgk = ip - program;
                spitErr(OverflowError, "Error modulo of INT32_MIN by -1 causes overflow!");
                NEXT_INST;
            }
            c.l = a.l % b.l;
            zlist_push(&STACK,c);
        }
        ip++; NEXT_INST;
    }
    CASE_CP INPLACE_INC:
    {
        orgk = ip - program;
        ip += 1;
        memcpy(&i1, ip, sizeof(int32_t));
        ip += 3;
        char t = STACK.arr[VEC_LAST(frames) + i1].type;
        if (t == Z_INT)
        {
            if (STACK.arr[VEC_LAST(frames) + i1].i == INT_MAX)
            {
                STACK.arr[VEC_LAST(frames) + i1].l = (int64_t)INT_MAX + 1;
                STACK.arr[VEC_LAST(frames) + i1].type = Z_INT64;
            }
            else
            STACK.arr[VEC_LAST(frames) + i1].i += 1;
        }
        else if (t == Z_INT64)
        {
            if (STACK.arr[VEC_LAST(frames) + i1].l == LLONG_MAX)
            {
            spitErr(OverflowError, "Error numeric overflow");
            NEXT_INST;
            }
            STACK.arr[VEC_LAST(frames) + i1].l += 1;
        }
        else if (t == Z_FLOAT)
        {
            if (STACK.arr[VEC_LAST(frames) + i1].f == FLT_MAX)
            {
            spitErr(OverflowError, "Error numeric overflow");
            NEXT_INST;
            }
            STACK.arr[VEC_LAST(frames) + i1].f += 1;
        }
        else
        {
            //spitErr(TypeError, "Error cannot add numeric constant to type " +(std::string)fullform(t));
            NEXT_INST;
        }
        ip++; 
        NEXT_INST;
    }
    CASE_CP SUB:
    {
        zobject a,b;
        zlist_fastpop(&STACK,&b);
        zlist_fastpop(&STACK,&a);
        if (a.type == Z_OBJ)
        {
            invokeOperator("__sub__", a, 2, "-", &b,true);
            NEXT_INST;
        }
        zobject c;
        char t;
        if(a.type == b.type && isNumeric(a.type))
            t = a.type;
        else if (isNumeric(a.type) && isNumeric(b.type))
        {
            if (a.type == Z_FLOAT || b.type == Z_FLOAT)
            t = Z_FLOAT;
            else if (a.type == Z_INT64 || b.type == Z_INT64)
            t = Z_INT64;
            else if (a.type == Z_INT || b.type == Z_INT)
            t = Z_INT;
            PromoteType(&a, t);
            PromoteType(&b, t);
            
        }
        else
        {
            orgk = ip - program;
            //spitErr(TypeError, "Error operator '-' unsupported for " +(std::string)fullform(a.type) + " and " +(std::string)fullform(b.type));
            NEXT_INST;
        }

        //
        if (t == Z_INT)
        {
            c.type = Z_INT;
            if (!subtraction_overflows_i32(a.i, b.i))
            {
                c.i = a.i - b.i;
                STACK.arr[STACK.size++] = c;
                ip++; NEXT_INST;
            }
            if (subtraction_overflows_i64((int64_t)a.i, (int64_t)b.i))
            {
                orgk = ip - program;
                spitErr(OverflowError, "Overflow occurred");
            }
            c.type = Z_INT64;
            c.l = (int64_t)(a.i) - (int64_t)(b.i);
            STACK.arr[STACK.size++] = c;
        }
        else if (t == Z_FLOAT)
        {
            if (subtraction_overflows_double(a.f, b.f))
            {
                orgk = ip - program;
                spitErr(OverflowError, "Floating point overflow during subtraction");
                NEXT_INST;
            }
            c.type = Z_FLOAT;
            c.f = a.f - b.f;
            STACK.arr[STACK.size++] = c;
        }
        else if (t == Z_INT64)
        {
            if (subtraction_overflows_i64(a.l, b.l))
            {
            orgk = ip - program;
            spitErr(OverflowError, "Error overflow during solving expression.");
            NEXT_INST;
            }
            c.type = Z_INT64;
            c.l = a.l - b.l;
            STACK.arr[STACK.size++] = c;
        }
        ip++; NEXT_INST;
    }
    CASE_CP DIV:
    {
        zobject b,a;
        zlist_fastpop(&STACK,&b);
        zlist_fastpop(&STACK,&a);
        if (a.type == Z_OBJ)
        {
            invokeOperator("__div__", a, 2, "/", &b,true);
            NEXT_INST;
        }
        zobject c;
        char t = 0;
        if(a.type == b.type && isNumeric(a.type))
            t = a.type;
        else if (isNumeric(a.type) && isNumeric(b.type))
        {
            if (a.type == Z_FLOAT || b.type == Z_FLOAT)
            t = Z_FLOAT;
            else if (a.type == Z_INT64 || b.type == Z_INT64)
            t = Z_INT64;
            else if (a.type == Z_INT || b.type == Z_INT)
            t = Z_INT;
            PromoteType(&a, t);
            PromoteType(&b, t);
        }
        else
        {
            orgk = ip - program;
            //spitErr(TypeError, "Error operator '/' unsupported for " +(std::string)fullform(a.type) + " and " +(std::string)fullform(b.type));
            NEXT_INST;
        }

        if (t == Z_INT)
        {
            if (b.i == 0)
            {
                orgk = ip - program;
                spitErr(MathError, "Error division by zero");
                NEXT_INST;
            }
            c.type = Z_INT;
            if (!division_overflows_i32(a.i, b.i))
            {
                c.i = a.i / b.i;
                STACK.arr[STACK.size++] = c;
                ip++; NEXT_INST;
            }
            if (division_overflows_i64((int64_t)a.i, (int64_t)b.i))
            {
                orgk = ip - program;
                spitErr(OverflowError, "Overflow occurred");
                NEXT_INST;
            }
            c.type = Z_INT64;
            c.l = (int64_t)(a.i) / (int64_t)(b.i);
            STACK.arr[STACK.size++] = c;
            ip++; NEXT_INST;
        }
        else if (t == Z_FLOAT)
        {
            if (b.f == 0)
            {
            orgk = ip - program;
            spitErr(MathError, "Error division by zero");
            NEXT_INST;
            }

            c.type = Z_FLOAT;
            c.f = a.f / b.f;
            STACK.arr[STACK.size++] = c;
        }
        else if (t == Z_INT64)
        {
            if (b.l == 0)
            {
                orgk = ip - program;
                spitErr(MathError, "Error division by zero");
                NEXT_INST;
            }
            if (division_overflows_i64(a.l, b.l))
            {
                orgk = ip - program;
                spitErr(OverflowError, "Error overflow during solving expression.");
                NEXT_INST;
            }
            c.type = Z_INT64;
            c.l = a.l / b.l;
            STACK.arr[STACK.size++] = c;
        }
        ip++; NEXT_INST;
    }
    CASE_CP JMP:
    {
        ip += 1;
        memcpy(&i1, ip, sizeof(int32_t));
        ip += 3;
        int32_t where = ip - program + i1 + 1;
        ip = program + where - 1;
        ip++; NEXT_INST;
    }
    CASE_CP JMPNPOPSTACK:
    {
        ip += 1;
        int32_t N;
        memcpy(&N, ip, sizeof(int32_t));
        ip += 4;
        zlist_erase_range(&STACK,STACK.size - N,STACK.size - 1);  
        memcpy(&i1, ip, sizeof(int32_t));
        ip += 3;
        int32_t where = ip - program + i1;
        ip = program + where;
        ip++; NEXT_INST;
    }
    CASE_CP CMP_JMPIFFALSE:
    {
        orgk = ip - program;
        uint8_t op = *(++ip);
        ip+=1;
        memcpy(&i1, ip, sizeof(int32_t));
        ip += 4;
        int32_t where = ip - program + i1 ;
        zlist_fastpop(&STACK,&p2);
        zlist_fastpop(&STACK,&p1);
        if (p1.type == Z_INT && p2.type == Z_INT64)
            PromoteType(&p1, Z_INT64);
        else if (p1.type == Z_INT64 && p2.type == Z_INT)
            PromoteType(&p2, Z_INT64);
        if (p1.type == Z_OBJ && p2.type != Z_NIL)
        {
            zclass_object* obj = (zclass_object*)p1.ptr;
            StrMap_get(&(obj->members),"__eq__",&p3);
            if(p3.type == Z_FUNC || p3.type == Z_NATIVE_FUNC)
            {
                zobject args[] = {p1,p2};
                vm_call_object(&p3, args, 2,&p4);
                if(p4.type != Z_BOOL)
                {
                    spitErr(TypeError, "__eq__ method must return a boolean!");
                    NEXT_INST;
                }
                if(p4.i == 0)
                    ip = program+where;
                NEXT_INST;
            }
        }
        bool truth = zobject_equals(p1,p2);
        if(!truth)
            ip = program+where;
        NEXT_INST;
    }
    CASE_CP THROW:
    {
        orgk = ip - program;
        zlist_fastpop(&STACK,&p3); //val
        if(p3.type != Z_OBJ)
        {
            //spitErr(TypeError,"Value of type "+(std::string)fullform(p3.type)+" not throwable!");
            NEXT_INST;
        }
        zclass_object* ki = (zclass_object*)p3.ptr;
        zobject msg;
        if( !StrMap_get(&(ki->members),"msg",&msg) || msg.type!=Z_STR)
        {
            spitErr(ThrowError,"Object does not have member 'msg' or it is not a string!");
            NEXT_INST;
        }
        if (except_targ.size == 0)
        {
            ///IMPORTANT
            spitErr(ki->_klass,((zstr*)(msg.ptr))->val );
            NEXT_INST;
        }
        ip = VEC_LAST(except_targ);
        i1 = STACK.size - VEC_LAST(try_stack_cleanup);
        STACK.size -= i1;
        i1 = frames.size - VEC_LAST(try_limit_cleanup);
        frames.size -= i1;
        STACK.arr[STACK.size++] = p3;
        except_targ.size--;
        try_stack_cleanup.size--;
        try_limit_cleanup.size--;
        NEXT_INST;
    }
    CASE_CP ONERR_GOTO:
    {
        ip += 1;
        memcpy(&i1, ip, 4);
        ptr_vector_push(&except_targ,(uint8_t *)program + i1);
        sizet_vector_push(&try_stack_cleanup,STACK.size);
        sizet_vector_push(&try_limit_cleanup,frames.size);
        ip += 4;
        NEXT_INST;
    }
    CASE_CP POP_EXCEP_TARG:
    {
        except_targ.size--;
        try_stack_cleanup.size--;
        ip++; 
        NEXT_INST;
    }
    CASE_CP GOTO:
    {
        ip += 1;
        memcpy(&i1, ip, sizeof(int32_t));
        ip = program + i1 - 1;
        ip++; 
        NEXT_INST;
    }
    CASE_CP GOTONPSTACK:
    {
        ip += 1;
        memcpy(&i1, ip, sizeof(int32_t));
        ip += 4;
        STACK.size -= i1;
        memcpy(&i1, ip, sizeof(int32_t));
        ip = program + i1;
        NEXT_INST;
    }
    CASE_CP GOTOIFFALSE:
    {
        ip += 1;
        memcpy(&i1, ip, sizeof(int32_t));
        ip += 3;
        zlist_fastpop(&STACK,&p1);
        if (p1.type == Z_NIL || (p1.type == Z_BOOL && p1.i == 0))
        {
            ip = program + i1;
            NEXT_INST;
        }
        ip++; NEXT_INST;
    }
    CASE_CP SELFMEMB:
    {
        orgk = ip - program;
        zobject a = STACK.arr[VEC_LAST(frames)];
        ++ip;
        memcpy(&i1, ip, sizeof(int32_t));
        ip += 3;
        const char* mname = ((zstr*)vm_strings.arr[i1])->val;//optimize
        if(a.type == Z_OBJ)
        {
            zclass_object*ptr = (zclass_object*)a.ptr;
            zobject tmp;
            if (!StrMap_get(&(ptr->members),mname,&tmp))
            {
                snprintf(error_buffer,100,"Object 'self' has no member named %s",mname);
                spitErr(NameError, error_buffer);
                NEXT_INST;
            }
            zlist_push(&STACK,tmp);
        }
        else
        {
            spitErr(TypeError, "self is not an object, you fucked up!");
            NEXT_INST;
        }
        ip++; NEXT_INST;
    }
    CASE_CP ASSIGNSELFMEMB:
    {
        orgk = ip - program;
        zobject val;
        zlist_fastpop(&STACK,&val);
        zobject Parent = STACK.arr[VEC_LAST(frames)];
        ip++;
        memcpy(&i1, ip, 4);
        ip += 3;
        const char* s1 = ((zstr*)vm_strings.arr[i1])->val;
        
        if (Parent.type != Z_OBJ)
        {
            //spitErr(TypeError, "Cannot access variable "+(string)s1+" ,self is not a class object!");
            NEXT_INST;
        }
        zclass_object*ptr = (zclass_object*)Parent.ptr;
        zobject* ref;
        if (! ( ref = StrMap_getRef(&(ptr->members),s1) ))
        {
            snprintf(error_buffer,100,"Object has no member named %s",s1); 
            spitErr(NameError, error_buffer);
            NEXT_INST;
        }
        *ref = val;
        ip++; NEXT_INST;
    }
    CASE_CP GC:
    {
        mark();
        collectGarbage();
        ip++; NEXT_INST;
    }
    CASE_CP OP_EXIT:
    {
        #ifndef ISTHREADED
            break;
        #endif
    }
    #ifndef ISTHREADED
    default:
    {
    // unknown opcode
    // Faulty bytecode
    fprintf(stderr,"An InternalError occurred.Error Code: 14\n");
    exit(1);
    }

    } // end switch statement
    ip += 1;

} // end while loop
#endif


if (STACK.size != 0 && panic)
{
    fprintf(stderr,"An InternalError occurred.Error Code: 15\n");
    printf("STACK.size = %zu\n",STACK.size);
    exit(1);
}
zlist_destroy(&names);
zlist_destroy(&values);
/*printf("--Counts--\n");
for(size_t i=0;i<sizeof(targets)/sizeof(void*);i++)
    if(counts[i]!=0)
        printf("%zu: %zu\n",i,counts[i]);*/
} // end function interpret
void vm_destroy()
{
    // call the unload() function in each module
    typedef void (*unload)(void);
    for (size_t i=0; i< module_handles.size; i++)
    {
        #ifdef _WIN32
        unload ufn = (unload)GetProcAddress(module_handles.arr[i], "unload");
        if (ufn)
            ufn();
        #else
        unload ufn = (unload)dlsym(module_handles.arr[i], "unload");
        if (ufn)
            ufn();
        #endif
    }
    STACK.size = 0;
    vm_important.size = 0;
    mark(); // clearing the STACK and marking objects will result in all objects being deleted
    // which is what we want
    collectGarbage();
    zlist_destroy(&STACK);
    zlist_destroy(&aux);
    for(size_t i = 0;i < vm_strings.size ; i++)
    {
        free(((zstr*)vm_strings.arr[i])->val);
        free(vm_strings.arr[i]);
    }
    free(vm_strings.arr);
    for(size_t i = 0;i < module_handles.size; i++)
    {
        #ifdef _WIN32
             FreeLibrary(module_handles.arr[i]);
        #else
            dlclose(module_handles.arr[i]);
        #endif
    }
    extern bmap methods;
    bmap_destroy(&methods);
    sizet_vector_destroy(&frames);
    sizet_vector_destroy(&try_limit_cleanup);
    sizet_vector_destroy(&try_stack_cleanup);
    ptr_vector_destroy(&callstack);
    ptr_vector_destroy(&executing);
    ptr_vector_destroy(&vm_builtin);
    ptr_vector_destroy(&except_targ);
    ptr_vector_destroy(&vm_important);
    ptr_vector_destroy(&module_handles);
    mem_map_destroy(&memory);
}


//////////
zobject zobj_from_str(const char* str)// makes deep copy of str
{
  size_t len = strlen(str);
  zstr* ptr = vm_alloc_zstr(len);
  memcpy(ptr->val,str,len);
  zobject ret;
  ret.type = 's';
  ret.ptr = (void*)ptr;
  return ret;
}
zobject z_err(zclass* errKlass,const char* des)
{
  zobject ret;
  zclass_object* p = vm_alloc_zclassobj(errKlass);
  StrMap_set(&(p->members),"msg",zobj_from_str(des)); // OPTIMIZE!
  ret.type = Z_ERROBJ;//indicates an object in thrown state
  ret.ptr = (void*) p;
  return ret;
}
zlist* vm_alloc_zlist()
{
  zlist* p = (zlist*)malloc(sizeof(zlist));
  if (!p)
  {
    fprintf(stderr,"allocList(): error allocating memory!\n");
    exit(0);
  }
  zlist_init(p);
  allocated += sizeof(zlist);
  mem_info m;
  m.type = Z_LIST;
  m.ismarked = false;
  mem_map_emplace(&memory,(void *)p, m);
  return p;
}
zbytearr* vm_alloc_zbytearr()
{
  zbytearr* p = (zbytearr*)malloc(sizeof(zbytearr));
  if (!p)
  {
    fprintf(stderr,"allocByteArray(): error allocating memory!\n");
    exit(0);
  }
  zbytearr_init(p);
  allocated += sizeof(zbytearr);
  mem_info m;
  m.type = Z_BYTEARR;
  m.ismarked = false;
  mem_map_emplace(&memory,(void *)p, m);
  return p;
}
/*uint8_t* vm_alloc_raw(size_t len)
{
  //len is the number of bytes
  uint8_t* p = (uint8_t*)malloc(len*sizeof(uint8_t));
  if (!p)
  {
    fprintf(stderr,"allocRaw(): error allocating memory!\n");
    exit(0);
  }
  allocated += len;
  MemInfo m;
  m.type = Z_RAW;
  m.isMarked = false;
  m.size = len;
  memory.emplace((void *)p, m);
  return p;
}
uint8_t* vm_realloc_raw(void* prev,size_t newsize)
{
  //newsize is the number of bytes
  uint8_t* p = (uint8_t*)realloc(prev,sizeof(uint8_t)*newsize);
  if (!p)
  {
    fprintf(stderr,"realloc_raw(): error allocating memory!\n");
    exit(0);
  }
  if(p == prev) // previous block expanded
  {
    memory[prev].size = newsize;
    return p;
  }
  else  //new block allocated previous freed
  {
    allocated -= memory[prev].size;
    memory.erase(prev);
    allocated += newsize;
    MemInfo m;
    m.type = Z_RAW;
    m.isMarked = false;
    m.size = newsize;
    memory.emplace((void *)p, m);
    return p;
  }
}*/

zstr* vm_alloc_zstr(size_t len)
{
  zstr* str = (zstr*)malloc(sizeof(zstr));
  char* buffer = (char*)malloc(sizeof(char)*(len+1));
  if (!buffer || !str)
  {
    fprintf(stderr,"alloc_zstr(): error allocating memory!\n");
    exit(0);
  }
  //The string owns this buffer
  //it will get deleted when string gets deleted
  buffer[len] = 0;
  str->val = buffer;
  str->len = len;

  allocated += len + 1 + sizeof(zstr);
  mem_info m;
  m.type = Z_STR;
  m.ismarked = false;
  mem_map_emplace(&memory,(void *)str, m);
  return str;
}
zclass* vm_alloc_zclass()
{
  zclass* p = (zclass*)malloc(sizeof(zclass));
  if (!p)
  {
    fprintf(stderr,"allocKlass(): error allocating memory!\n");
    exit(0);
  }
  StrMap_init(&(p->members));
  allocated += sizeof(zclass);
  mem_info m;
  m.type = Z_CLASS;
  m.ismarked = false;
  mem_map_emplace(&memory,(void *)p, m);
  return p;
}
zmodule* vm_alloc_zmodule()
{
  zmodule* p = (zmodule*)malloc(sizeof(zmodule));
  if (!p)
  {
    fprintf(stderr,"allocModule(): error allocating memory!\n");
    exit(0);
  }
  StrMap_init(&(p->members));
  allocated += sizeof(zmodule);
  mem_info m;
  m.type = Z_MODULE;
  m.ismarked = false;
  mem_map_emplace(&memory,(void *)p, m);
  return p;
}
zclass_object* vm_alloc_zclassobj(zclass* k)
{
  zclass_object* p = (zclass_object*)malloc(sizeof(zclass_object));
  if (!p)
  {
    fprintf(stderr,"allocKlassObject(): error allocating memory!\n");
    exit(0);
  }
  StrMap_init(&(p->members));
  StrMap_assign(&(p->members),&(k->members));
  p -> _klass = k;
  allocated += sizeof(zclass_object);
  mem_info m;
  m.type = Z_OBJ;
  m.ismarked = false;
  mem_map_emplace(&memory,(void *)p, m);
  return p;
}
Coroutine* vm_alloc_coro_obj()//allocates coroutine object
{
  Coroutine* p = (Coroutine*)malloc(sizeof(Coroutine));
  if (!p)
  {
    fprintf(stderr,"allocCoObj(): error allocating memory!\n");
    exit(0);
  }
  allocated += sizeof(Coroutine);
  mem_info m;
  m.type = 'z';
  m.ismarked = false;
  mem_map_emplace(&memory,(void *)p, m);
  zlist_init(&(p->locals));
  return p;
}
zfun* vm_alloc_zfun()
{
  zfun* p = (zfun*)malloc(sizeof(zfun));
  if (!p)
  {
    fprintf(stderr,"alloczfun(): error allocating memory!\n");
    exit(0);
  }
  p->_klass = NULL;
  allocated += sizeof(zfun);
  mem_info m;
  m.type = Z_FUNC;
  m.ismarked = false;
  mem_map_emplace(&memory,(void *)p, m);
  zlist_init(&(p->opt));
  return p;
}
zfun* vm_alloc_coro() //coroutine can be represented by zfun
{
  zfun* p = (zfun*)malloc(sizeof(zfun));
  if (!p)
  {
    fprintf(stderr,"allocCoroutine(): error allocating memory!\n");
    exit(0);
  }
  p->_klass = NULL;
  allocated += sizeof(zfun);
  mem_info m;
  m.type = 'g';
  m.ismarked = false;
  mem_map_emplace(&memory,(void *)p, m);
  return p;
}
zfile * vm_alloc_zfile()
{
  zfile* p = (zfile*)malloc(sizeof(zfile));
  if (!p)
  {
    fprintf(stderr,"alloczfile(): error allocating memory!\n");
    exit(0);
  }
  allocated += sizeof(zlist);
  mem_info m;
  m.type = Z_FILESTREAM;
  m.ismarked = false;
  mem_map_emplace(&memory,(void *)p, m);
  return p;
}
zdict* vm_alloc_zdict()
{
  zdict *p = (zdict*)malloc(sizeof(zdict));
  if (!p)
  {
    fprintf(stderr,"allocDict(): error allocating memory!\n");
    exit(0);
  }
  zdict_init(p);
  allocated += sizeof(zdict);
  mem_info m;
  m.type = Z_DICT;
  m.ismarked = false;
  mem_map_emplace(&memory,(void *)p, m);
  return p;
}
znativefun* vm_alloc_znativefun()
{
  znativefun *p = (znativefun*)malloc(sizeof(znativefun));
  if (!p)
  {
    fprintf(stderr,"allocNativeFun(): error allocating memory!\n");
    exit(0);
  }
  p->signature = NULL;
  allocated += sizeof(znativefun);
  mem_info m;
  m.type = Z_NATIVE_FUNC;
  m.ismarked = false;
  mem_map_emplace(&memory,(void *)p, m);
  return p;
}
//callObject also behaves as a kind of try/catch since v0.31
bool vm_call_object(zobject* obj,zobject* args,int N,zobject* rr)
{
  static char buffer[1024];
  if(obj->type == Z_FUNC)
  {
     zfun* fn = (zfun*)obj->ptr;
     if ((size_t)N + fn->opt.size < fn->args || (size_t)N > fn->args)
     {
      static char buffer[1024];
      snprintf(buffer,1024,"Function %s takes %zu arguments, %d given!",fn->name,fn->args,N);
      *rr = z_err(ArgumentError, buffer);
      return false;
     }
     uint8_t* prev = ip;
     ptr_vector_push(&callstack,NULL);
     sizet_vector_push(&frames,STACK.size);
     ptr_vector_push(&executing,fn);
     for(int i=0;i<N;i++)
       zlist_push(&STACK,args[i]);
     for(size_t i = fn->opt.size - (fn->args - N); i < fn->opt.size; i++)
       zlist_push(&STACK,fn->opt.arr[i]);
     bool a = viaCO;
     viaCO = true;
     interpret(fn->i,false);
     viaCO = a;
     ip = prev;
     zlist_fastpop(&STACK,rr);
     return (rr->type != Z_ERROBJ);
  }
  else if (obj->type == Z_NATIVE_FUNC)
  {
    znativefun *A = (znativefun *)obj->ptr;
    NativeFunPtr f = A->addr;
    zobject p4;
    if(A->signature)
    {
      size_t len = strlen(A->signature);
      if(len != N)
      {
        snprintf(buffer,1024,"Native function %s takes %zu arguments, %d given!",A->name,len,N);       
        *rr = z_err(ArgumentError,buffer);
        return false;
      }
      size_t i = 0;
      while(i<N)
      {
        if(args[i].type != A->signature[i])
        {
          snprintf(buffer,1024,"Argument %zu to %s must be a %s\n",i+1,A->name,fullform(A->signature[i]));
          *rr = z_err(TypeError,buffer);
          return false;
        }
        i+=1;
      }
    }
    p4 = f(args, N);

    if (p4.type == Z_ERROBJ)
    {
      *rr = p4;
      return false;
    }
    if (strcmp(fullform(p4.type),"Unknown")==0 && p4.type != Z_NIL)
    {
      *rr = z_err(ValueError, "Invalid response from native function!");
      return false;
    }
    *rr = p4;
    return true;
  }
  *rr = z_err(TypeError,"Object not callable!");
  return false;
}
void vm_mark_important(void* mem)
{
    mem_info tmp;
    if(mem_map_get(&memory,mem,&tmp))
        ptr_vector_push(&vm_important,mem);
  
}
void vm_unmark_important(void* mem)
{
  int idx = -1;
  if((idx = ptr_vector_search(&vm_important,mem)) != -1)
    ;//TODO 
}
