#include "zuko.h"
#include "vm.h"
#include "zobject.h"

using namespace std;


inline void PromoteType(ZObject &a, char t)
{
  if (a.type == Z_INT)
  {
    if (t == Z_INT64) // promote to int64_t
    {
      a.type = Z_INT64;
      a.l = (int64_t)a.i;
    }
    else if (t == Z_FLOAT)
    {
      a.type = Z_FLOAT;
      a.f = (double)a.i;
    }
  }
  else if (a.type == Z_FLOAT)
  {
    if (t == Z_INT64) // promote to int64_t
    {
      a.type = Z_INT64;
      a.l = (int64_t)a.f;
    }
    else if (t == Z_INT)
    {
      a.type = Z_INT;
      a.f = (int32_t)a.f;
    }
    else if (t == Z_FLOAT)
      return;
  }
  else if (a.type == Z_INT64)
  {
    if (t == Z_FLOAT) // only this one is needed
    {
      a.type = Z_FLOAT;
      a.f = (double)a.l;
    }
  }
}

string fullform(char t)
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
    return "ZDict";
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

extern bool REPL_MODE;
void REPL();
//Error classes
Klass* Error;
Klass* TypeError;
Klass* ValueError;
Klass* MathError; 
Klass* NameError;
Klass* IndexError;
Klass* ArgumentError;
Klass* FileIOError;
Klass* KeyError;
Klass* OverflowError;
Klass* FileOpenError;
Klass* FileSeekError; 
Klass* ImportError;
Klass* ThrowError;
Klass* MaxRecursionError;
Klass* AccessError;


VM::VM()
{
  ZList_init(&aux);
  ZList_init(&STACK);
}
void VM::load(vector<uint8_t>& bytecode,ProgramInfo& p)
{
    program = &bytecode[0];
    program_size = bytecode.size();
    GC_THRESHHOLD = 4196;//chosen at random
    LineNumberTable = &p.LineNumberTable;
    files = &p.files;
    sources = &p.sources;
    nil.type = Z_NIL;

    api.a1 = &vm_allocList;
    api.a2 = &vm_allocDict;
    api.a3 = &vm_allocString;
    //    api.a4 = &allocMutString; RIP
    api.a5 = &vm_alloczfile;
    api.a6 = &vm_allocKlass;
    api.a7 = &vm_allocKlassObject;
    api.a8 = &vm_allocNativeFunObj;
    api.a9 = &vm_allocModule;
    api.a10 = &vm_allocByteArray;
    api.a11 = &vm_callObject;
    api.a12 = &vm_markImportant;
    api.a13 = &vm_unmarkImportant;
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
    api.k16 = AccessError;
    srand(time(0));
}
size_t VM::spitErr(Klass* e, string msg) // used to show a runtime error
{
ZObject p1;
if (except_targ.size() != 0)//check if error catching is enabled
{
    size_t T = STACK.size - tryStackCleanup.back();
    STACK.size -= T;
    KlassObject *E = vm_allocKlassObject(e);
    ZStr* s = vm_allocString(msg.length());
    memcpy(s->val,&msg[0],msg.length());
    StrMap_set(&(E->members),"msg",ZObjFromStrPtr(s));
    p1.type = Z_OBJ;
    p1.ptr = (void*)E;
    ZList_push(&STACK,p1);
    T = frames.size() - tryLimitCleanup.back();
    frames.erase(frames.end() - T, frames.end());
    k = except_targ.back();
    except_targ.pop_back();
    tryStackCleanup.pop_back();
    tryLimitCleanup.pop_back();
    return k - program;
}
if(viaCO) //interpret was called via callObject, wrap the error in a nice object
//remove all values from stack starting from frame made by callObject
{
    while(callstack.back() != NULL)//there must be a null
    {
    frames.pop_back();
    callstack.pop_back();
    }
    STACK.size = frames.back();
    KlassObject *E = vm_allocKlassObject(e);
    ZStr* s = vm_allocString(msg.length());
    memcpy(s->val,&msg[0],msg.length());
    StrMap_set(&(E->members),"msg",ZObjFromStrPtr(s));
    p1.type = Z_ERROBJ;
    p1.ptr = (void*)E;
    ZList_push(&STACK,p1);
    this->k = program + program_size - 1;
    return k -program;
}
//Error catching is not enabled
size_t line_num = (*LineNumberTable)[orgk].ln;
string &filename = (*files)[(*LineNumberTable)[orgk].fileIndex];
string type = e->name;
fprintf(stderr,"\nFile %s\n", filename.c_str());
fprintf(stderr,"%s at line %zu\n", type.c_str(), line_num);
auto it = std::find(files->begin(), files->end(), filename);
size_t i = it - files->begin();

string source_code = (*sources)[i];
size_t l = 1;
string line = "";
size_t k = 0;

while (l <= line_num)
{
    if (source_code[k] == '\n')
    l += 1;
    else if (l == line_num)
    line += source_code[k];
    k += 1;
    if (k >= source_code.length())
    break;
}
fprintf(stderr,"%s\n", lstrip(line).c_str());
fprintf(stderr,"%s\n", msg.c_str());

if (callstack.size() != 0 && e != MaxRecursionError) // printing stack trace for max recursion is stupid
{
    fprintf(stderr,"<stack trace>\n");
    while (callstack.size() != 0) // print stack trace
    {
    size_t L = callstack.back() - program;
    
    L -= 1;
    while (L>0 && LineNumberTable->find(L) == LineNumberTable->end())
    {
        L -= 1; // L is now the index of CALLUDF opcode now
    }
    // which is ofcourse present in the LineNumberTable
    fprintf(stderr,"  --by %s line %zu\n", (*files)[(*LineNumberTable)[L].fileIndex].c_str(), (*LineNumberTable)[L].ln);
    callstack.pop_back();
    }
}

this->k = program+program_size-1;//set instruction pointer to last instruction
//which is always OP_EXIT

if(!REPL_MODE)//nothing can be done, clear stack and exit
    ZList_resize(&STACK,0);

return this->k - program;

}
inline void VM::DoThreshholdBusiness()
{
if (allocated > GC_THRESHHOLD)
{
    mark();
    collectGarbage();
}
}

inline static bool isHeapObj(const ZObject &obj)
{
if (obj.type != Z_INT && obj.type != Z_INT64 && obj.type != Z_FLOAT && obj.type != Z_NIL && obj.type != Z_BYTE && obj.type != 'p' && obj.type != Z_BOOL)
    return true; // all other objects are on heap
return false;
}
void VM::markV2(const ZObject &obj)
{
aux.size = 0;
std::unordered_map<void *, MemInfo>::iterator it;
if (isHeapObj(obj) && (it = memory.find(obj.ptr)) != memory.end() && !(it->second.isMarked))
{
    // mark object alive and push it
    it->second.isMarked = true;
    ZList_push(&aux,obj);
}
// if an object is on aux then
//  - it is a heap object
//  - it was unmarked but marked before pushing
//  - known to VM's memory pool i.e not from outside (any native module or something)

// After following above constraints,aux will not have any duplicates or already marked objects
ZObject curr;
while (aux.size != 0)
{
    ZList_fastpop(&aux,&curr);
    // for each curr object we get ,it was already marked alive before pushing onto aux
    // so we only process objects referenced by it and not curr itself
    if (curr.type == Z_DICT)
    {
    ZDict &d = *(ZDict *)curr.ptr;
    for (size_t i=0;i<d.capacity;i++)
    {
        if(d.table[i].stat != OCCUPIED)
            continue;
        // if e.first or e.second is a heap object and known to our memory pool
        // add it to aux
        if (isHeapObj(d.table[i].key) && (it = memory.find(d.table[i].key.ptr)) != memory.end() && !(it->second.isMarked))
        {
            it->second.isMarked = true;
            ZList_push(&aux,d.table[i].key);
        }
        if (isHeapObj(d.table[i].val) && (it = memory.find(d.table[i].val.ptr)) != memory.end() && !(it->second.isMarked))
        {
            it->second.isMarked = true;
            ZList_push(&aux,d.table[i].val);
        }
    }
    }
    else if (curr.type == Z_LIST)
    {
        ZList &d = *(ZList *)curr.ptr;
        for (size_t i = 0;i<d.size;i++)
        {
            ZObject e = d.arr[i];
            if (isHeapObj(e) && (it = memory.find(e.ptr)) != memory.end() && !(it->second.isMarked))
            {
            it->second.isMarked = true;
            ZList_push(&aux,e);
            }
        }
    }
    else if (curr.type == 'z') // coroutine object
    {
        Coroutine *g = (Coroutine *)curr.ptr;
        for (size_t i = 0;i<g->locals.size;i++)
        {
            ZObject e = g->locals.arr[i];
            if (isHeapObj(e) && (it = memory.find(e.ptr)) != memory.end() && !(it->second.isMarked))
            {
            it->second.isMarked = true;
            ZList_push(&aux,e);
            }
        }
        FunObject *gf = g->fun;
        it = memory.find((void *)gf);
        if (it != memory.end() && !(it->second.isMarked))
        {
            ZObject tmp;
            tmp.type = Z_FUNC;
            tmp.ptr = (void *)gf;
            it->second.isMarked = true;
            ZList_push(&aux,tmp);
        }
    }
    else if (curr.type == Z_MODULE)
    {
        Module *k = (Module *)curr.ptr;
        for (size_t idx=0; idx<  k->members.capacity;idx++)
        {
            if(k->members.table[idx].stat!= SM_OCCUPIED)
              continue;
            auto& e = k->members.table[idx];
            if (isHeapObj(e.val) && (it = memory.find(e.val.ptr)) != memory.end() && !(it->second.isMarked))
            {
            it->second.isMarked = true;
            ZList_push(&aux,e.val);
            }
        }
    }
    else if (curr.type == Z_CLASS)
    {
        Klass *k = (Klass *)curr.ptr;
        for (size_t idx = 0; idx < k->members.capacity;idx++)
        {
            if(k->members.table[idx].stat != SM_OCCUPIED)
              continue;
            SM_Slot& e = k->members.table[idx];
            if (isHeapObj(e.val) && (it = memory.find(e.val.ptr)) != memory.end() && !(it->second.isMarked))
            {
                it->second.isMarked = true;
                ZList_push(&aux,e.val);
            }
            if( (it = memory.find((void*)e.key)) != memory.end() )
                it->second.isMarked = true;
            
        }
        for (size_t idx = 0; idx < k->privateMembers.capacity;idx++)
        {
            if(k->privateMembers.table[idx].stat != SM_OCCUPIED)
              continue;
            SM_Slot& e = k->privateMembers.table[idx];
            if (isHeapObj(e.val) && (it = memory.find(e.val.ptr)) != memory.end() && !(it->second.isMarked))
            {
            it->second.isMarked = true;
            ZList_push(&aux,e.val);
            }
            if( (it = memory.find((void*)e.key)) != memory.end() )
            it->second.isMarked = true;
        }
    }
    else if (curr.type == Z_NATIVE_FUNC)
    {
        NativeFunction *fn = (NativeFunction *)curr.ptr;
        it = memory.find((void *)fn->klass);
        if (it != memory.end() && !(it->second.isMarked))
        {
            ZObject tmp;
            tmp.type = Z_CLASS;
            tmp.ptr = (void *)fn->klass;
            it->second.isMarked = true;
            ZList_push(&aux,tmp);
        }
    }
    else if (curr.type == Z_OBJ)
    {
        KlassObject *k = (KlassObject *)curr.ptr;
        Klass *kk = k->klass;
        it = memory.find((void *)kk);
        if (it != memory.end() && !(it->second.isMarked))
        {
            ZObject tmp;
            tmp.type = Z_CLASS;
            tmp.ptr = (void *)kk;
            it->second.isMarked = true;
            ZList_push(&aux,tmp);
        }
        for (size_t idx = 0; idx < k->members.capacity;idx++)
        {
            if(k->members.table[idx].stat != SM_OCCUPIED)
              continue;
            SM_Slot& e = k->members.table[idx];
            if (isHeapObj(e.val) && (it = memory.find(e.val.ptr)) != memory.end() && !(it->second.isMarked))
            {
                it->second.isMarked = true;
                ZList_push(&aux,e.val);
            }
        }
        for (size_t idx = 0; idx < k->privateMembers.capacity;idx++)
        {
            if(k->privateMembers.table[idx].stat != SM_OCCUPIED)
                continue;
            SM_Slot& e = k->privateMembers.table[idx];
            if (isHeapObj(e.val) && (it = memory.find(e.val.ptr)) != memory.end() && !(it->second.isMarked))
            {
                it->second.isMarked = true;
                ZList_push(&aux,e.val);
            }
        }
    }
    else if (curr.type == Z_FUNC || curr.type == 'g')
    {
        Klass *k = ((FunObject *)curr.ptr)->klass;
        it = memory.find((void *)k);
        if (it != memory.end() && !(it->second.isMarked))
        {
            ZObject tmp;
            tmp.type = Z_CLASS;
            tmp.ptr = (void *)k;
            it->second.isMarked = true;
            ZList_push(&aux,tmp);
        }
        for (size_t i = 0;i< ((FunObject *)curr.ptr)->opt.size;i++ )
        {
            ZObject e = ((FunObject*)curr.ptr)->opt.arr[i];
            if (isHeapObj(e) && (it = memory.find(e.ptr)) != memory.end() && !(it->second.isMarked))
            {
            it->second.isMarked = true;
            ZList_push(&aux,e);
            }
        }
    }
} // end while loop
}
void VM::mark()
{
    for (size_t i=0;i<STACK.size;i++)
        markV2(STACK.arr[i]);
    for(auto e: important)
    {
        ZObject p;
        p.ptr = e;
        p.type = memory[e].type;
        markV2(p);
    }
}

void VM::collectGarbage()
{
size_t pre = allocated;
vector<void*> toFree;

for (auto e : memory)
{
    MemInfo m = e.second;
    
    if (m.isMarked)
    memory[e.first].isMarked = false;
    else
    {
    //call destructor of unmarked objects
    if (m.type == Z_OBJ)
    {
        KlassObject *obj = (KlassObject *)e.first;
        ZObject dummy;
        dummy.type = Z_OBJ;
        dummy.ptr = e.first;
        ZObject cb;
        if (StrMap_get(&(obj->members),"__del__",&cb))
        {
        ZObject p1 = cb;
        if(p1.type == Z_NATIVE_FUNC || p1.type == Z_FUNC)
        {
            ZObject rr;
            vm_callObject(&p1,&dummy,1,&rr);
        }
        }
    }
    toFree.push_back(e.first);
    }
}
for (auto e : toFree)
{
    MemInfo m = memory[e];
    
    if (m.type == Z_LIST)
    {
    ZList_destroy((ZList*)e);
    delete (ZList *)e;
    allocated -= sizeof(ZList);
    }
    else if (m.type == Z_MODULE)
    {
    Module* m = (Module*)e;
    StrMap_destroy(&(m->members));
    delete (Module *)e;
    allocated -= sizeof(Module);
    }
    else if (m.type == Z_CLASS)
    {
    Klass* k = (Klass*)e;
    StrMap_destroy(&(k->members));
    StrMap_destroy(&(k->privateMembers));
    delete (Klass *)e;
    allocated -= sizeof(Klass);
    }
    else if (m.type == Z_OBJ)
    {
    KlassObject* k = (KlassObject*)e;
    StrMap_destroy(&(k->members));
    StrMap_destroy(&(k->privateMembers));  
    delete (KlassObject *)e;
    allocated -= sizeof(KlassObject);
    }
    else if (m.type == Z_DICT)
    {
    ZDict_destroy((ZDict*)e);
    delete (ZDict *)e;
    allocated -= sizeof(ZDict);
    }
    else if (m.type == Z_FILESTREAM)
    {
    zfile* zf = (zfile*)e;
    if(zf->open)
        fclose(zf->fp);
    delete (zfile *)e;
    allocated -= sizeof(zfile);
    }
    else if (m.type == Z_FUNC)
    {
    ZList_destroy(&((FunObject*)e) ->opt);
    delete (FunObject *)e;
    allocated -= sizeof(FunObject);
    }
    else if(m.type == Z_COROUTINE)
    {
    delete (FunObject *)e;
    allocated -= sizeof(FunObject);
    }
    else if (m.type == 'z')
    {
    ZList_destroy(&((Coroutine*)e)->locals);
    delete (Coroutine *)e;
    allocated -= sizeof(Coroutine);
    }
    else if (m.type == Z_ERROBJ)
    {
    delete (KlassObject *)e;
    allocated -= sizeof(KlassObject);
    }
    else if (m.type == Z_STR)
    {
    ZStr* p = (ZStr*)e;
    allocated -= sizeof(ZStr) + p->len + 1;
    delete[] p->val;
    delete p;
    }      
    else if (m.type == Z_NATIVE_FUNC)
    {
    delete (NativeFunction *)e;
    allocated -= sizeof(NativeFunction);
    }
    else if(m.type == Z_BYTEARR)
    {
    ZByteArr_destroy((ZByteArr*)e);
    delete (ZByteArr*)e;
    allocated -= sizeof(ZByteArr);
    }
    memory.erase(e);
}
size_t recycled = pre - allocated;
// printf("recycled %lld bytes\n",recycled);
if (recycled < 4196) // a gc cycle is expensive so running it to collect not even 4196 bytes is useless
{
    // if this is the case we update the threshhold so that next time we collect more bytes
    // the number 4196 was picked randomly and it seems to work well
    GC_THRESHHOLD *= 2;
}
}
bool VM::invokeOperator(string meth, ZObject A, size_t args, const char* op, ZObject *rhs, bool raiseErrOnNF) // check if the object has the specified operator overloaded and prepares to call it by updating callstack and frames
{
    //Operators invoked by this function are either unary or binary
    //So at max 1 argument is given that is ZObject rhs (non-NULL means usable)
    //raiseErrOnNF = whether to raise error on not found or not ?
    KlassObject *obj = (KlassObject *)A.ptr;
    ZObject p3;
    static string s1;// to avoid multiple object creation across multiple invokeCalls
    if (StrMap_get(&(obj->members),meth.c_str(),&p3))
    {
        if (p3.type == Z_FUNC)
        {
            FunObject *fn = (FunObject *)p3.ptr;
            if (fn->opt.size != 0)
                spitErr(ArgumentError, "Optional parameters not allowed for operator functions!");
            if (fn->args == args)
            {
                callstack.push_back(k + 1);
                if (callstack.size() >= 1000)
                {
                    spitErr(MaxRecursionError, "Max recursion limit 1000 reached.");
                    return false;
                }
                executing.push_back(fn);
                frames.push_back(STACK.size);
                ZList_push(&STACK,A);
                if (rhs != NULL)
                    ZList_push(&STACK,*rhs);
                k = program + fn->i;
                return true;
            }
        }
        else if (p3.type == Z_NATIVE_FUNC)
        {
            NativeFunction *fn = (NativeFunction*)p3.ptr;
            NativeFunPtr M = fn->addr;
            ZObject rr; 
            ZObject argArr[2] = {A, (!rhs) ? nil : *rhs};
            // do typechecking 
            if(fn->signature)
            {
                size_t len = strlen(fn->signature);
                if(len != 2)
                {
                    spitErr(ArgumentError,(string)"Native function "+ (string)fn->name + (string)" takes "+to_string(len)+" arguments, 2 given!");
                    return false;
                }
                size_t i = 0;
                while(i<2)
                {
                if(argArr[i].type != fn->signature[i])
                {
                    spitErr(TypeError,"Argument "+to_string(i+1)+" to "+(string)fn->name+(string)+"should be a "+fullform(fn->signature[i]));
                    return false;
                }
                i+=1;
            }
        }
        rr = M(argArr, args);
        if (rr.type == Z_ERROBJ)
        {
            KlassObject *E = (KlassObject*)rr.ptr;
            ZObject msg;
            StrMap_get(&(E->members),"msg",&msg);
            s1 = meth + "():  " + (string)((ZStr*)msg.ptr)->val;
            spitErr(E->klass, s1);
            return false;
        }
        ZList_push(&STACK,rr);
        k++;
        return true;
        }
    }
    if (!raiseErrOnNF)
        return false;
    if (args == 2)
        spitErr(TypeError, "Operator '" + (string)op + "' unsupported for types " + fullform(A.type) + " and " + fullform(rhs->type));
    else
        spitErr(TypeError, "Operator '" + (string)op + "' unsupported for type " + fullform(A.type));
    return false;
}
void VM::interpret(size_t offset , bool panic) //by default panic if stack is not empty when finished
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
        &&JMPIF,
        &&EQ,
        &&NOTEQ,
        &&SMALLERTHAN,
        &&GREATERTHAN,
        &&CALLUDF,
        &&INPLACE_INC,
        &&LOAD_STR,
        &&JMPIFFALSE,
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
        NULL, //unused for now
        &&CALLFORVAL,
        &&INC_GLOBAL,
        NULL, //unused for now
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
        &&LOAD_BYTE
    };
    #endif
#endif
ZObject p1;
ZObject p2;
ZObject p3;
ZObject p4;
string s1;
string s2;
char c1;
//ZList pl1;      // zuko list 1
ZList* pl_ptr1; // zuko list pointer 1
//ZDict pd1;
ZObject alwaysi32;
ZObject alwaysByte;
vector<ZObject> values;
vector<ZObject> names;
alwaysi32.type = Z_INT;
alwaysByte.type = Z_BYTE;
int32_t* alwaysi32ptr = &alwaysi32.i;
ZDict *pd_ptr1;
ZByteArr* bt_ptr1;
k = program + offset;

#ifndef ISTHREADED
uint8_t inst;
while (*k != OP_EXIT)
{
    inst = *k;
    switch (inst)
    {
#else
    NEXT_INST;
#endif
    CASE_CP LOAD_GLOBAL:
    {
    k += 1;
    memcpy(&i1, k, 4);
    ZList_push(&STACK,STACK.arr[i1]);
    k += 4;
    NEXT_INST;
    }
    CASE_CP LOAD_LOCAL:
    {
    k += 1;
    memcpy(&i1, k, 4);
    ZList_push(&STACK,STACK.arr[frames.back() + i1]);
    k += 4;
    NEXT_INST;
    }     
    CASE_CP INC_GLOBAL:
    {
    orgk = k - program;
    k += 1;
    memcpy(&i1, k, 4);
    k += 3;
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
        if (STACK.arr[i1].f == FLT_MAX)
        {
        spitErr(OverflowError, "Numeric overflow");
        NEXT_INST;
        }
        STACK.arr[i1].f += 1;
    }
    else
    {
        spitErr(TypeError, "Cannot add numeric constant to type " + fullform(c1));
        NEXT_INST;
    }
    k++; NEXT_INST;
    }  
    CASE_CP LOAD:
    {
    orgk = k - program;
    k += 1;
    c1 = *k;
    k += 1;
    if (c1 == Z_LIST)
    {
        int32_t listSize;
        memcpy(&listSize, k, sizeof(int32_t));
        k += 3;
        pl_ptr1 = vm_allocList();
        ZList_resize(pl_ptr1,listSize);
        memcpy(pl_ptr1->arr,STACK.arr + STACK.size-listSize,listSize*sizeof(ZObject));
        ZList_eraseRange(&STACK,STACK.size - listSize,STACK.size - 1);
        p1.type = Z_LIST;
        p1.ptr = (void *)pl_ptr1;
        ZList_push(&STACK,p1);
        DoThreshholdBusiness();
    }
    else if (c1 == Z_DICT)
    {
        memcpy(&i1, k, sizeof(int32_t));
        //i1 is dictionary size
        k += 3;
        pd_ptr1 = vm_allocDict();
        while (i1 != 0)
        {
        ZList_fastpop(&STACK,&p1);
        ZList_fastpop(&STACK,&p2);
        if(p2.type!=Z_INT && p2.type!=Z_INT64 && p2.type!=Z_FLOAT && p2.type!=Z_STR  && p2.type!=Z_BYTE && p2.type!=Z_BOOL)
        {
            spitErr(TypeError,"Key of type "+fullform(p2.type)+" not allowed.");
            NEXT_INST;
        }
        if (ZDict_get(pd_ptr1,p2,&p4))
        {
            spitErr(ValueError, "Duplicate keys in dictionary not allowed!");
            NEXT_INST;
        }
        ZDict_set(pd_ptr1,p2,p1);
        i1 -= 1;
        }
        p1.ptr = (void *)pd_ptr1;
        p1.type = Z_DICT;
        ZList_push(&STACK,p1);
        DoThreshholdBusiness();
    }
    else if (c1 == 'v')
    {
        memcpy(&i1, k, sizeof(int32_t));
        k += 3;
        ZList_push(&STACK,STACK.arr[frames.back() + i1]);
    }
    k++; NEXT_INST;
    }
    CASE_CP LOAD_CONST:
    {
    k += 1;
    memcpy(&i1, k, 4);
    k += 4;
    ZList_push(&STACK,constants[i1]);
    NEXT_INST;
    }
    CASE_CP LOAD_INT32:
    {
    k += 1;
    memcpy(alwaysi32ptr, k, 4);
    k += 4;
    ZList_push(&STACK,alwaysi32);
    NEXT_INST;
    }
    CASE_CP LOAD_BYTE:
    {
    k += 1;
    alwaysByte.i = *k;
    ZList_push(&STACK,alwaysByte);
    k++; NEXT_INST;
    }
    CASE_CP ASSIGN:
    {
    k += 1;
    memcpy(&i1, k, 4);
    k += 4;
    ZList_fastpop(&STACK,&p1);
    STACK.arr[frames.back() + i1] = p1;
    NEXT_INST;
    }
    CASE_CP ASSIGN_GLOBAL:
    {
    k += 1;
    memcpy(&i1, k, 4);
    k += 4;
    ZList_fastpop(&STACK,&p1);
    STACK.arr[i1] = p1;
    NEXT_INST;
    }
    CASE_CP ASSIGNINDEX:
    {
    orgk = k - program;
    ZList_fastpop(&STACK,&p1);
    ZList_fastpop(&STACK,&p2);
    ZList_fastpop(&STACK,&p3);
    if (p3.type == Z_LIST)
    {
        pl_ptr1 = (ZList *)p3.ptr;
        int64_t idx = 0;
        if (p1.type == Z_INT)
        idx = p1.i;
        else if (p1.type == Z_INT64)
        idx = p1.l;
        else
        {
        spitErr(TypeError, "Error type " + fullform(p1.type) + " cannot be used to index list!");
        NEXT_INST;
        }
        if (idx < 0 || idx > (int64_t)pl_ptr1->size)
        {
        spitErr(ValueError, "Error index " + ZObjectToStr(p1) + " out of range for list of size " + to_string(pl_ptr1->size));
        NEXT_INST;
        }
        (*pl_ptr1).arr[idx] = p2;
    }
    else if (p3.type == Z_BYTEARR)
    {
        bt_ptr1 = (ZByteArr*)p3.ptr;
        int64_t idx = 0;
        if (p1.type == Z_INT)
        idx = p1.i;
        else if (p1.type == Z_INT64)
        idx = p1.l;
        else
        {
        spitErr(TypeError, "Error type " + fullform(p1.type) + " cannot be used to index bytearray!");
        NEXT_INST;
        }
        if (idx < 0 || idx > (int64_t)bt_ptr1->size)
        {
        spitErr(IndexError, "Error index " + ZObjectToStr(p1) + " out of range for bytearray of size " + to_string(pl_ptr1->size));
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
        pd_ptr1 = (ZDict *)p3.ptr;
        ZDict_set(pd_ptr1,p1,p2);
    }
    else
    {
        spitErr(TypeError, "Error indexed assignment unsupported for type " + fullform(p3.type));
        NEXT_INST;
    }

    k++; NEXT_INST;
    }
    CASE_CP CALL:
    {
    orgk = k - program;
    k += 1;
    memcpy(&i1, k, 4);
    k += 4;
    int32_t howmany = *k;
    p1=builtin[i1](STACK.arr+STACK.size - howmany, howmany);
    if (p1.type == Z_ERROBJ)
    {
        KlassObject* E = (KlassObject*)p1.ptr;
        ZObject msg;
        StrMap_get(&(E->members),"msg",&msg);
        spitErr(E->klass, AS_STD_STR(msg));
        NEXT_INST;
    }
//        ZList_eraseRange(&STACK,STACK.size - howmany,STACK.size - 1);
    STACK.size -= howmany;
    k++;
    NEXT_INST;
    }
    CASE_CP CALLFORVAL:
    {
    orgk = k - program;
    k += 1;
    memcpy(&i1, k, 4);
    k += 4;
    i2 = *k;
    p1 = builtin[i1](STACK.arr+STACK.size - i2, i2);
    if (p1.type == Z_ERROBJ)
    {
        KlassObject* E = (KlassObject*)p1.ptr;
        ZObject msg;
        StrMap_get(&(E->members),"msg",&msg);
        spitErr(E->klass, AS_STD_STR(msg));
        NEXT_INST;
    }
    ZList_eraseRange(&STACK,STACK.size - i2,STACK.size - 1);
    ZList_push(&STACK,p1);
    k++; NEXT_INST;
    }
    CASE_CP CALLMETHOD:
    {
    orgk = k - program;
    k++;
    memcpy(&i1, k, 4);
    k += 4;
    const char* method_name = strings[i1].val;//OPTIMIZE
    i2 = *k;
    p1 = STACK.arr[STACK.size-i2-1]; // Parent object from which member is being called
    if (p1.type == Z_MODULE)
    {
        Module *m = (Module *)p1.ptr;
        if (!StrMap_get(&(m->members),method_name,&p3))
        {
        spitErr(NameError, "Error the module has no member " + (string)method_name + "!");
        NEXT_INST;
        }
        if (p3.type == Z_NATIVE_FUNC)
        {
        NativeFunction *fn = (NativeFunction *)p3.ptr;
        NativeFunPtr f = fn->addr;
        ZObject *argArr = &STACK.arr[STACK.size-i2];
        if(fn->signature)
        {
            size_t len = strlen(fn->signature);
            if(len != i2)
            {
            spitErr(ArgumentError,(string)"Native function "+ (string)fn->name + (string)" takes "+to_string(len)+" arguments, "+to_string(i2)+" given!");
            NEXT_INST;
            }
            size_t i = 0;
            while(i<i2)
            {
            if(argArr[i].type != fn->signature[i])
            {
                spitErr(TypeError,"Argument "+to_string(i+1)+" to "+(string)fn->name+(string)+"should be a "+fullform(fn->signature[i]));
                NEXT_INST;
            }
            i+=1;
            }
        }
        p4 = f(argArr, i2);

        if (p4.type == Z_ERROBJ)
        {
            // The module raised an error
            KlassObject* E = (KlassObject*)p4.ptr;
            ZObject msg;
            StrMap_get(&(E->members),"msg",&msg);
            s1 = (string)method_name + "():  " +  AS_STD_STR(msg);
            spitErr(E->klass,s1);
            NEXT_INST;
        }
        if (fullform(p4.type) == "Unknown" && p4.type != Z_NIL)
        {
            spitErr(ValueError, "Error invalid response from module!");
            NEXT_INST;
        }
        ZList_eraseRange(&STACK,STACK.size-i2-1,STACK.size-1);
        ZList_push(&STACK,p4);
        }
        else if (p3.type == Z_CLASS)
        {
        KlassObject *obj = vm_allocKlassObject((Klass*)p3.ptr); // instance of class
        ZObject r;
        r.type = Z_OBJ;
        r.ptr = (void *)obj;
        STACK.arr[STACK.size-i2-1] = r;
        ZObject construct;
        if (StrMap_get(&(obj->members),"__construct__",&construct))
        {
            if (construct.type != Z_NATIVE_FUNC)
            {
            spitErr(TypeError, "Error constructor of module's class " + (string)((Klass *)p3.ptr)->name + " is not a native function!");
            NEXT_INST;
            }
            NativeFunction *fn = (NativeFunction *)construct.ptr;
            NativeFunPtr p = fn->addr;

            ZObject *args = &STACK.arr[STACK.size-i2-1];
            if(fn->signature)
            {
            size_t len = strlen(fn->signature);
            if(len != i2)
            {
                spitErr(ArgumentError,(string)"Native function "+ (string)fn->name + (string)" takes "+to_string(len)+" arguments, "+to_string(i2)+" given!");
                NEXT_INST;
            }
            size_t i = 0;
            while(i<i2)
            {
                if(args[i].type != fn->signature[i])
                {
                spitErr(TypeError,"Argument "+to_string(i+1)+" to "+(string)fn->name+(string)+"should be a "+fullform(fn->signature[i]));
                NEXT_INST;
                }
                i+=1;
            }
            }
            p1 = p(args, i2 + 1);
            if (p1.type == Z_ERROBJ)
            {
            // The module raised an error
            KlassObject* E = (KlassObject*)p1.ptr;
            ZObject msg;
            StrMap_get(&(E->members),"msg",&msg);
            s1 = (string)method_name + "():  " +  AS_STD_STR(msg);
            spitErr(E->klass, s1);
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
        ZObject callmethod(string, ZObject *, int32_t);
        p3 = callmethod(method_name, &STACK.arr[STACK.size-i2-1], i2 + 1);
        if (p3.type == Z_ERROBJ)
        {
        KlassObject* E = (KlassObject*)p3.ptr;
        ZObject msg;
        StrMap_get(&(E->members),"msg",&msg);
        spitErr(E->klass, *(string *)(msg.ptr));
        NEXT_INST;
        }
        ZList_eraseRange(&STACK,STACK.size-i2-1,STACK.size-1);
        ZList_push(&STACK,p3);
    }
    else if (p1.type == Z_OBJ)
    {
        KlassObject *obj = (KlassObject *)p1.ptr;
        ZObject tmp;

        if (!StrMap_get(&(obj->members),method_name,&tmp))
        {
        if (StrMap_get(&(obj->privateMembers),method_name,&tmp))
        {
            FunObject *p = executing.back();
            if (p == NULL)
            {
            spitErr(NameError, "Error " + (string)method_name + " is private member of object!");
            NEXT_INST;
            }
            if (p->klass == obj->klass)
            p4 = tmp;
            else
            {
            spitErr(NameError, "Error " + (string)method_name + " is private member of object!");
            NEXT_INST;
            }
        }
        else
        {
            spitErr(NameError, "Error object has no member " + (string)method_name);
            NEXT_INST;
        }
        }
        else
          p4 = tmp;
        if (p4.type == Z_FUNC)
        {
            FunObject *memFun = (FunObject *)p4.ptr;
            if ((size_t)i2 + 1 + memFun->opt.size < memFun->args || (size_t)i2 + 1 > memFun->args)
            {
                spitErr(ArgumentError, "Error function " + (string)memFun->name + " takes " + to_string(memFun->args - 1) + " arguments," + to_string(i2) + " given!");
                NEXT_INST;
            }
            callstack.push_back(k + 1);
            if (callstack.size() >= 1000)
            {
                spitErr(MaxRecursionError, "Error max recursion limit 1000 reached.");
                NEXT_INST;
            }
            executing.push_back(memFun);
            frames.push_back(STACK.size-i2-1);
            // add default arguments
            for (size_t i = memFun->opt.size - (memFun->args - 1 - (size_t)i2); i < (memFun->opt.size); i++)
            {
                ZList_push(&STACK,memFun->opt.arr[i]);
            }
        //
            k = program + memFun->i;
            NEXT_INST;
        }
        else if (p4.type == Z_NATIVE_FUNC)
        {
        NativeFunction *fn = (NativeFunction *)p4.ptr;
        NativeFunPtr R = fn->addr;
        ZObject *args = &STACK.arr[STACK.size-i2-1];
        ZObject rr;
        if(fn->signature)
        {
            size_t len = strlen(fn->signature);
            if(len != i2+1)
            {
            spitErr(ArgumentError,(string)"Native function "+ (string)fn->name + (string)" takes "+to_string(len)+" arguments, "+to_string(i2)+" given!");
            NEXT_INST;
            }
            size_t i = 0;
            while(i<i2+1)
            {
            if(args[i].type != fn->signature[i])
            {
                spitErr(TypeError,"Argument "+to_string(i+1)+" to "+(string)fn->name+(string)+"should be a "+fullform(fn->signature[i]));
                NEXT_INST;
            }
            i+=1;
            }
        }
        rr = R(args, i2 + 1);
        if (rr.type == Z_ERROBJ)
        {
            KlassObject* E = (KlassObject *)rr.ptr;
            ZObject msg;
            StrMap_get(&(E->members),"msg",&msg);
            spitErr(E->klass, (string)fn->name + ": " + AS_STD_STR(msg));
            NEXT_INST;
        }
        ZList_eraseRange(&STACK,STACK.size-i2-1,STACK.size-1);
        ZList_push(&STACK,rr);
        }
        else
        {
        spitErr(NameError, "Error member " + (string)method_name + " of object is not callable!");
        NEXT_INST;
        }
    }
    else if (p1.type == 'z')
    {
        if(i2!=0)
           p4 = STACK.arr[STACK.size - 1];
        Coroutine *g = (Coroutine *)p1.ptr;
        s1 = method_name;
        if (s1 == "isAlive")
        {
        if(i2!=0)
        {
            spitErr(NameError,"Error coroutine member isAlive() takes 0 arguments!");
            NEXT_INST;
        }
        ZObject isAlive;
        isAlive.type = Z_BOOL;
        isAlive.i = (g->state != STOPPED);
        ZObject tmp;
        ZList_fastpop(&STACK,&tmp);
        ZList_push(&STACK,isAlive);
        k++;
        NEXT_INST;
        }
        if (s1 != "resume")
        {
        spitErr(NameError, "Error couroutine object has no member " + s1);
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
        callstack.push_back(k + 1);
        if (callstack.size() >= 1000)
        {
        spitErr(MaxRecursionError, "Error max recursion limit 1000 reached.");
        NEXT_INST;
        }
        executing.push_back(NULL);
        frames.push_back(STACK.size-i2);
        ZList_insertList(&STACK,STACK.size,&(g->locals));
        g->state = RUNNING;
        g->giveValOnResume = false;
        k = program + g->curr;
        NEXT_INST;
    }
    else
    {
        spitErr(TypeError, "Error member call not supported for type " + fullform(p1.type));
        NEXT_INST;
    }
    k++; NEXT_INST;
    }
    CASE_CP ASSIGNMEMB:
    {
    orgk = k - program;
    ZList_fastpop(&STACK,&p2);//value
    ZList_fastpop(&STACK,&p1);//parent
    k++;
    memcpy(&i1, k, 4);
    k += 3;
    const char* memberName = strings[i1].val;        
    if (p1.type != Z_OBJ)
    {
        spitErr(TypeError, "Error member assignment unsupported for " + fullform(p1.type));
        NEXT_INST;
    }
    KlassObject *ptr = (KlassObject *)p1.ptr;
    ZObject* ref;
    if (!(ref = StrMap_getRef(&(ptr->members),memberName)))
    {
        if (( ref = StrMap_getRef(&(ptr->privateMembers),memberName)))
        {
        // private member
        FunObject *A = executing.back();
        if (A == NULL || A->klass != ptr->klass)
        {
            spitErr(AccessError, "Error cannot access private member " + s1 + " of class " + ptr->klass->name + "'s object!");
            NEXT_INST;
        }
        *ref = p2;
        k += 1;
        NEXT_INST;
        }
        spitErr(NameError, "Error the object has no member named " + s1);
        NEXT_INST;
    }
    *ref = p2;
    k++; 
    NEXT_INST;
    }
    CASE_CP IMPORT:
    {
    orgk = k - program;
    k += 1;
    memcpy(&i1, k, sizeof(int32_t));
    k += 3;
    s1 = strings[i1].val;
    typedef ZObject (*initFun)();
    typedef int (*apiFun)(apiFuncions *,int);
    #ifdef _WIN32
        s1 = "C:\\zuko\\modules\\" + s1 + ".dll";
        HINSTANCE module = LoadLibraryA(s1.c_str());
        if (!module)
        {
        spitErr(ImportError, "LoadLibrary() returned " + to_string(GetLastError()));
        NEXT_INST;
        }
        apiFun a = (apiFun)GetProcAddress(module, "api_setup");
        initFun f = (initFun)GetProcAddress(module, "init");
        
    #endif
    #ifdef __linux__
        s2 = "./modules/"+s1+".so";
        void* module = dlopen(s2.c_str(),RTLD_LAZY);
        if(!module)
        {
        s1 = "/opt/zuko/modules/" + s1 + ".so";
        module = dlopen(s1.c_str(), RTLD_LAZY);
        if (!module)
        {
            spitErr(ImportError, (std::string)(dlerror()));
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
    moduleHandles.push_back(module);
    ZList_push(&STACK,p1);
    k++; NEXT_INST;
    }
    CASE_CP OP_RETURN:
    {
    k = callstack.back();
    callstack.pop_back();
    executing.pop_back();
    p1 = STACK.arr[STACK.size - 1];
    STACK.size = frames.back()+1;
    STACK.arr[frames.back()] = p1;
    frames.pop_back();
    if(!k)
        return;//return from interpret function
    NEXT_INST;
    }
    CASE_CP YIELD:
    {
    executing.pop_back();
    ZList_fastpop(&STACK,&p1);
    ZObject* locals = STACK.arr + frames.back();
    size_t total = STACK.size - frames.back();
    STACK.size = frames.back();
    ZList_fastpop(&STACK,&p2);//genObj
    Coroutine *g = (Coroutine *)p2.ptr;
    ZList_resize(&(g->locals),total);
    if(total != 0)
        memcpy(g->locals.arr,locals,total*sizeof(ZObject));
    g->curr = k - program + 1;
    g->state = SUSPENDED;
    g->giveValOnResume = false;
    frames.pop_back();
    ZList_push(&STACK,p1);
    k = callstack[callstack.size() - 1] - 1;
    callstack.pop_back();
    k++; 
    NEXT_INST;
    }
    CASE_CP YIELD_AND_EXPECTVAL:
    {
    executing.pop_back();
    ZList_fastpop(&STACK,&p2);
    ZObject* locals = STACK.arr + frames.back();
    size_t total = STACK.size - frames.back();
    STACK.size = frames.back();
    ZList_fastpop(&STACK,&p1);
    Coroutine *g = (Coroutine *)p1.ptr;
    ZList_resize(&(g->locals),total);
    if(total != 0)
        memcpy(g->locals.arr,locals,total*sizeof(ZObject));
    g->curr = k - program + 1;
    g->state = SUSPENDED;
    g->giveValOnResume = true;
    frames.pop_back();
    ZList_push(&STACK,p2);
    k = callstack[callstack.size() - 1] - 1;
    callstack.pop_back();
    k++; NEXT_INST;
    }
    CASE_CP LOAD_NIL:
    {
    p1.type = Z_NIL;
    ZList_push(&STACK,p1);
    k++; NEXT_INST;
    }
    CASE_CP CO_STOP:
    {
    executing.pop_back();
    ZObject val;
    ZList_fastpop(&STACK,&val);
    //we don't really need to save the coroutines locals anymore
    STACK.size = frames.back();
    ZObject genObj;
    ZList_fastpop(&STACK,&genObj);
    Coroutine *g = (Coroutine *)genObj.ptr;
    g->locals.size = 0;
    g->curr = k - program + 1;
    g->state = STOPPED;
    frames.pop_back();
    ZList_push(&STACK,val);
    k = callstack[callstack.size() - 1] - 1;
    callstack.pop_back();
    k++; NEXT_INST;
    }
    CASE_CP POP_STACK:
    {
    --STACK.size;
    k++;
    NEXT_INST;
    }
    CASE_CP NPOP_STACK:
    {
    k += 1;
    memcpy(&i1, k, 4);
    k += 4;
    STACK.size -= i1;
    NEXT_INST;
    }
    CASE_CP LOAD_TRUE:
    {
    p1.type = Z_BOOL;
    p1.i = 1;
    ZList_push(&STACK,p1);
    k++; NEXT_INST;
    }
    CASE_CP LOAD_FALSE:
    {
    p1.type = Z_BOOL;
    p1.i = 0;
    ZList_push(&STACK,p1);
    k++; NEXT_INST;
    }
    CASE_CP LSHIFT:
    {
    orgk = k - program;
    ZList_fastpop(&STACK,&p2);
    ZList_fastpop(&STACK,&p1);
    if (p1.type == Z_OBJ)
    {
        invokeOperator("__lshift__", p1, 2, "<<", &p2);
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
        spitErr(TypeError, "Error operator << unsupported for type " + fullform(p1.type));
        NEXT_INST;
    }
    ZList_push(&STACK,p3);
    k++; NEXT_INST;
    }
    CASE_CP RSHIFT:
    {
    orgk = k - program;
    ZList_fastpop(&STACK,&p2);
    ZList_fastpop(&STACK,&p1);
    if (p1.type == Z_OBJ)
    {
        invokeOperator("__rshift__", p1, 2, ">>", &p2);
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
        spitErr(TypeError, "Error operator >> unsupported for type " + fullform(p1.type));
        NEXT_INST;
    }
    ZList_push(&STACK,p3);
    k++; NEXT_INST;
    }
    CASE_CP BITWISE_AND:
    {
    orgk = k - program;
    ZList_fastpop(&STACK,&p2);
    ZList_fastpop(&STACK,&p1);
    if (p1.type == Z_OBJ)
    {
        invokeOperator("__bitwiseand__", p1, 2, "&", &p2);
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
        spitErr(TypeError, "Error operator & unsupported for type " + fullform(p1.type));
        NEXT_INST;
    }
    ZList_push(&STACK,p3);
    k++; NEXT_INST;
    }
    CASE_CP BITWISE_OR:
    {
    orgk = k - program;
    ZList_fastpop(&STACK,&p2);
    ZList_fastpop(&STACK,&p1);
    if (p1.type == Z_OBJ)
    {
        invokeOperator("__bitwiseor__", p1, 2, "|", &p2);
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
        spitErr(TypeError, "Error operator | unsupported for type " + fullform(p1.type));
        NEXT_INST;
    }
    ZList_push(&STACK,p3);
    k++; NEXT_INST;
    }
    CASE_CP COMPLEMENT:
    {
    orgk = k - program;
    ZList_fastpop(&STACK,&p1);
    if (p1.type == Z_OBJ)
    {
        invokeOperator("__complement__", p1, 1, "~");
        NEXT_INST;
    }
    if (p1.type == Z_INT)
    {
        p2.type = Z_INT;
        p2.i = ~p1.i;
        ZList_push(&STACK,p2);
    }
    else if (p1.type == Z_INT64)
    {
        p2.type = Z_INT64;
        p2.l = ~p1.l;
        ZList_push(&STACK,p2);
    }
    else if (p1.type == Z_BYTE)
    {
        
        p2.type = Z_BYTE;
        uint8_t p = p1.i;
        p = ~p;
        p2.i = p; 
        ZList_push(&STACK,p2);
    }
    else
    {
        spitErr(TypeError, "Error operator '~' unsupported for type " + fullform(p1.type));
        NEXT_INST;
    }
    k++; NEXT_INST;
    }
    CASE_CP XOR:
    {
    orgk = k - program;
    ZList_fastpop(&STACK,&p2);
    ZList_fastpop(&STACK,&p1);
    if (p1.type == Z_OBJ)
    {
        invokeOperator("__xor__", p1, 2, "^", &p2);
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
        spitErr(TypeError, "Error operator '^' unsupported for type " + fullform(p1.type));
        NEXT_INST;
    }
    ZList_push(&STACK,p3);
    k++; NEXT_INST;
    }
    CASE_CP ADD:
    {
    orgk = k - program;
    ZList_fastpop(&STACK,&p2);
    ZList_fastpop(&STACK,&p1);
    if (p1.type == Z_OBJ)
    {
        invokeOperator("__add__", p1, 2, "+", &p2);
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
        PromoteType(p1, c1);
        PromoteType(p2, c1);
    }
    else
    {
        spitErr(TypeError, "Error operator '+' unsupported for " + fullform(p1.type) + " and " + fullform(p2.type));
        NEXT_INST;
    }
    if (c1 == Z_LIST)
    {
        p3.type = Z_LIST;
        ZList* a = (ZList*)p1.ptr;
        ZList* b = (ZList *)p2.ptr;
        ZList* res = vm_allocList();
        ZList_resize(res,a->size + b->size);
        memcpy(res->arr,a->arr,a->size*sizeof(ZObject));
        memcpy(res->arr+a->size,b->arr,b->size*sizeof(ZObject));
        p3.ptr = (void *)res;
        ZList_push(&STACK,p3);
        DoThreshholdBusiness();
    }
    else if (c1 == Z_STR )
    {
        ZStr* a = (ZStr*)p1.ptr;
        ZStr* b = (ZStr*)p2.ptr;
        
        ZStr* p = vm_allocString(a->len + b->len);
        memcpy(p->val, a->val, a->len);
        memcpy(p->val + a->len, b->val, b->len);
        p3 = ZObjFromStrPtr(p);
        STACK.arr[STACK.size++] = p3;
        DoThreshholdBusiness();
    }
    else if (c1 == Z_INT)
    {
        p3.type = Z_INT;
        if (!addition_overflows(p1.i, p1.i))
        {
        p3.i = p1.i + p2.i;
        STACK.arr[STACK.size++] = p3;
        k++; NEXT_INST;
        }
        if (addition_overflows((int64_t)p1.i, (int64_t)p2.i))
        {
        spitErr(OverflowError, "Integer Overflow occurred");
        NEXT_INST;
        }
        p3.type = Z_INT64;
        p3.l = (int64_t)(p1.i) + (int64_t)(p2.i);
        STACK.arr[STACK.size++] = p3;
    }
    else if (c1 == Z_FLOAT)
    {
        if (addition_overflows(p1.f, p2.f))
        {
        orgk = k - program;
        spitErr(OverflowError, "Floating point overflow during addition");
        NEXT_INST;
        }
        p3.type = Z_FLOAT;
        p3.f = p1.f + p2.f;
        STACK.arr[STACK.size++] = p3;
    }
    else if (c1 == Z_INT64)
    {
        if (addition_overflows(p1.l, p2.l))
        {
        orgk = k - program;
        spitErr(OverflowError, "Error overflow during solving expression.");
        NEXT_INST;
        }
        p3.type = Z_INT64;
        p3.l = p1.l + p2.l;
        STACK.arr[STACK.size++] = p3;
        k++; NEXT_INST;
    }
    else
    {
        spitErr(TypeError, "Error operator '+' unsupported for types " + fullform(p1.type) + " and " + fullform(p2.type));
        NEXT_INST;
    }
    k++; NEXT_INST;
    }
    CASE_CP SMALLERTHAN:
    {
    orgk = k - program;
    ZList_fastpop(&STACK,&p2);
    ZList_fastpop(&STACK,&p1);
    if(p1.type == p2.type && isNumeric(p1.type))
        c1 = p1.type;
    else if (p1.type == Z_OBJ)
    {
        invokeOperator("__smallerthan__", p1, 2, "<", &p2);
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
        PromoteType(p1, c1);
        PromoteType(p2, c1);
    }
    else
    {
        spitErr(TypeError, "Error operator '<' unsupported for types " + fullform(p1.type) + " and " + fullform(p2.type));
        NEXT_INST;
    }
    p3.type = Z_BOOL;
    if (p1.type == Z_INT)
        p3.i = (bool)(p1.i < p2.i);
    else if (p1.type == Z_INT64)
        p3.i = (bool)(p1.l < p2.l);
    else if (p1.type == Z_FLOAT)
        p3.i = (bool)(p1.f < p2.f);
    
    ZList_push(&STACK,p3);
    k++; NEXT_INST;
    }
    CASE_CP GREATERTHAN:
    {
    ZList_fastpop(&STACK,&p2);
    ZList_fastpop(&STACK,&p1);
    if(p1.type == p2.type && isNumeric(p1.type))
        c1=p1.type;
    else if (p1.type == Z_OBJ)
    {
        invokeOperator("__greaterthan__", p1, 2, ">", &p2);
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
        PromoteType(p1, c1);
        PromoteType(p2, c1);
    }
    else
    {
        orgk = k - program;
        spitErr(TypeError, "Error operator '>' unsupported for types " + fullform(p1.type) + " and " + fullform(p2.type));
        NEXT_INST;
    }
    p3.type = Z_BOOL;
    if (p1.type == Z_INT)
        p3.i = (bool)(p1.i > p2.i);
    else if (p1.type == Z_INT64)
        p3.i = (bool)(p1.l > p2.l);
    else if (p1.type == Z_FLOAT)
        p3.i = (bool)(p1.f > p2.f);
    
    ZList_push(&STACK,p3);
    k++; NEXT_INST;
    }
    CASE_CP SMOREQ:
    {
    orgk = k - program;
    ZList_fastpop(&STACK,&p2);
    ZList_fastpop(&STACK,&p1);
            
    if(p1.type == p2.type && isNumeric(p1.type))
        c1 = p1.type;
    else if (p1.type == Z_OBJ)
    {
        invokeOperator("__smallerthaneq__", p1, 2, "<=", &p2);
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
        PromoteType(p1, c1);
        PromoteType(p2, c1);
    }
    else
    {
        spitErr(TypeError, "Error operator '<=' unsupported for types " + fullform(p1.type) + " and " + fullform(p2.type));
        NEXT_INST;
    }
    p3.type = Z_BOOL;
    if (p1.type == Z_INT)
        p3.i = (bool)(p1.i <= p2.i);
    else if (p1.type == Z_INT64)
        p3.i = (bool)(p1.l <= p2.l);
    else if (p1.type == Z_FLOAT)
        p3.i = (bool)(p1.f <= p2.f);
    
    //ZList_push(&STACK,p3);
    STACK.arr[STACK.size++] = p3;
    k++; NEXT_INST;
    }
    CASE_CP GROREQ:
    {
    orgk = k - program;
    ZList_fastpop(&STACK,&p2);
    ZList_fastpop(&STACK,&p1);
    if(p1.type == p2.type && isNumeric(p1.type))
        c1 = p1.type;
    else if (p1.type == Z_OBJ)
    {
        invokeOperator("__greaterthaneq__", p1, 2, ">=", &p2);
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
        PromoteType(p1, c1);
        PromoteType(p2, c1);
    }
    else
    {
        spitErr(TypeError, "Error operator '>=' unsupported for types " + fullform(p1.type) + " and " + fullform(p2.type));
        NEXT_INST;
    }
    p3.type = Z_BOOL;
    if (p1.type == Z_INT)
        p3.i = (bool)(p1.i >= p2.i);
    else if (p1.type == Z_INT64)
        p3.i = (bool)(p1.l >= p2.l);
    else if (p1.type == Z_FLOAT)
        p3.i = (bool)(p1.f >= p2.f);
    
    ZList_push(&STACK,p3);
    k++; NEXT_INST;
    }
    CASE_CP EQ:
    {
    orgk = k - program;
    ZList_fastpop(&STACK,&p2);
    ZList_fastpop(&STACK,&p1);
    if (p1.type == Z_INT && p2.type == Z_INT64)
        PromoteType(p1, Z_INT64);
    else if (p1.type == Z_INT64 && p2.type == Z_INT)
        PromoteType(p2, Z_INT64);
    if (p1.type == Z_OBJ && p2.type != Z_NIL)
    {
        if(invokeOperator("__eq__", p1, 2, "==", &p2, 0))
        NEXT_INST;
    }
    p3.type = Z_BOOL;
    p3.i = (bool)(ZObject_equals(p1,p2));
    //IMPORTANT1  ZList_push(&STACK,p3);
    STACK.arr[STACK.size++] = p3;
    k++; NEXT_INST;
    }
    CASE_CP NOT:
    {
    ZList_fastpop(&STACK,&p1);
    if (p1.type == Z_OBJ)
    {
        orgk = k-program;
        invokeOperator("__not__", p1, 1, "!");
        NEXT_INST;
    }
    if (p1.type != Z_BOOL)
    {
        orgk = k - program;
        spitErr(TypeError, "Error operator '!' unsupported for type " + fullform(p1.type));
        NEXT_INST;
    }
    p1.i = (bool)!(p1.i);
    ZList_push(&STACK,p1);
    k++; NEXT_INST;
    }
    CASE_CP NEG:
    {
    ZList_fastpop(&STACK,&p1);
    if (p1.type == Z_OBJ)
    {
        orgk = k - program;
        invokeOperator("__neg__", p1, 1, "-");
        NEXT_INST;
    }
    if (!isNumeric(p1.type))
    {
        orgk = k - program;
        spitErr(TypeError, "Error unary operator '-' unsupported for type " + fullform(p1.type));
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
        orgk = k - program;
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
    ZList_push(&STACK,p1);
    k++; NEXT_INST;
    }
    CASE_CP INDEX:
    {
    ZList_fastpop(&STACK,&p1);
    ZList_fastpop(&STACK,&p2);
    if (p2.type == Z_LIST)
    {
        if (p1.type != Z_INT && p1.type != Z_INT64)
        {
        orgk = k - program;
        spitErr(TypeError, "Error index should be integer!");
        NEXT_INST;
        }
        PromoteType(p1, Z_INT64);
        if (p1.l < 0)
        {
        orgk = k - program;
        spitErr(ValueError, "Error index cannot be negative!");
        NEXT_INST;
        }

        pl_ptr1 = (ZList *)p2.ptr;
        if ((size_t)p1.l >= pl_ptr1->size)
        {
        orgk = k - program;
        spitErr(ValueError, "Error index is out of range!");
        NEXT_INST;
        }
        ZList_push(&STACK,(*pl_ptr1).arr[p1.l]);
        k++; NEXT_INST;
    }
    else if (p2.type == Z_BYTEARR)
    {
        if (p1.type != Z_INT && p1.type != Z_INT64)
        {
        orgk = k - program;
        spitErr(TypeError, "Error index should be integer!");
        NEXT_INST;
        }
        PromoteType(p1, Z_INT64);
        if (p1.l < 0)
        {
        orgk = k - program;
        spitErr(ValueError, "Error index cannot be negative!");
        NEXT_INST;
        }

        bt_ptr1 = (ZByteArr*)p2.ptr;
        if ((size_t)p1.l >= bt_ptr1->size)
        {
        orgk = k - program;
        spitErr(ValueError, "Error index is out of range!");
        NEXT_INST;
        }
        p3.type = 'm';
        p3.i = bt_ptr1->arr[p1.l];
        ZList_push(&STACK,p3);
        k++; NEXT_INST;
    }
    else if (p2.type == Z_DICT)
    {
        ZDict* d = (ZDict *)p2.ptr;
        ZObject res;
        if (!ZDict_get(d,p1,&res))
        {
        orgk = k - program;
        spitErr(KeyError, "Error key " + ZObjectToStr(p1) + " not found in the dictionary!");
        NEXT_INST;
        }
        ZList_push(&STACK,res);
    }
    else if (p2.type == Z_STR )
    {
        if (p1.type != Z_INT && p1.type != Z_INT64)
        {
        orgk = k - program;
        spitErr(TypeError, "Error index for string should be an integer!");
        NEXT_INST;
        }
        PromoteType(p1, Z_INT64);
        if (p1.l < 0)
        {
        orgk = k - program;
        spitErr(ValueError, "Error index cannot be negative!");
        NEXT_INST;
        }
        char* s;
        size_t length;
        s = ((ZStr*)p2.ptr) -> val;
        length = ((ZStr*)p2.ptr) -> len;
        if ((size_t)p1.l >= length)
        {
        orgk = k - program;
        spitErr(ValueError, "Error index is out of range!");
        }
        char c = s[p1.l];
        ZStr* p = vm_allocString(1);
        p->val[0] = c;
        ZList_push(&STACK,ZObjFromStrPtr(p));
        DoThreshholdBusiness();
    }
    else if (p2.type == Z_OBJ)
    {
        orgk = k - program;
        invokeOperator("__index__", p2, 2, "[]", &p1);
        NEXT_INST;
    }
    else
    {
        orgk = k - program;
        spitErr(TypeError, "Error operator '[]' unsupported for type " + fullform(p2.type));
        NEXT_INST;
    }
    k++;
    NEXT_INST;
    }
    CASE_CP NOTEQ:
    {

    ZList_fastpop(&STACK,&p2);
    ZList_fastpop(&STACK,&p1);
    if (p1.type == Z_OBJ && p2.type != Z_NIL)
    {
        invokeOperator("__noteq__", p1, 2, "!=", &p2, 0);
        NEXT_INST;
    }
    p3.type = Z_BOOL;
    if (p1.type == Z_INT && p2.type == Z_INT64)
        PromoteType(p1, Z_INT64);
    else if (p1.type == Z_INT64 && p2.type == Z_INT)
        PromoteType(p2, Z_INT64);
    p3.i = (bool)!(ZObject_equals(p1,p2));
    ZList_push(&STACK,p3);
    k++; NEXT_INST;
    }
    CASE_CP IS:
    {

    ZList_fastpop(&STACK,&p2);
    ZList_fastpop(&STACK,&p1);
    if ((p1.type != Z_DICT && p1.type != Z_CLASS && p1.type != Z_LIST && p1.type != Z_OBJ && p1.type != Z_STR && p1.type != Z_MODULE && p1.type != Z_FUNC) || (p2.type != Z_CLASS && p2.type != Z_DICT && p2.type != Z_STR && p2.type != Z_FUNC && p2.type != Z_LIST && p2.type != Z_OBJ && p2.type != Z_MODULE))
    {
        orgk = k - program;
        spitErr(TypeError, "Error operator 'is' unsupported for types " + fullform(p1.type) + " and " + fullform(p2.type));
        NEXT_INST;
    }
    p3.type = Z_BOOL;
    p3.i = (bool)(p1.ptr == p2.ptr);
    ZList_push(&STACK,p3);
    k++; NEXT_INST;
    }
    CASE_CP MUL:
    {
    ZList_fastpop(&STACK,&p2);
    ZList_fastpop(&STACK,&p1);
    if (p1.type == Z_OBJ)
    {
        invokeOperator("__mul__", p1, 2, "*", &p2);
        NEXT_INST;
    }
    ZObject c;
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
        PromoteType(p1, t);
        PromoteType(p2, t);
    }
    else if(p1.type==Z_LIST && (p2.type==Z_INT || p2.type == Z_INT64))
    {
        PromoteType(p2,Z_INT64);
        if(p2.l < 0)
        {
        orgk = k - program;
        spitErr(ValueError,"Cannot multiply list by a negative integer!");
        NEXT_INST;
        }
        ZList* src = (ZList*)p1.ptr;
        ZList* res = vm_allocList();
        if(src->size!=0)
        {
        for(size_t i=1;i<=(size_t)p2.l;i++)
        {
            size_t idx = res->size;
            ZList_resize(res,res->size+src->size);
            memcpy(res->arr+idx,src->arr,src->size*sizeof(ZObject));
//              res->insert(res->end(),src->begin(),src->end())
        }
        }
        p1.type = Z_LIST;
        p1.ptr = (void*)res;
        ZList_push(&STACK,p1);
        DoThreshholdBusiness();
        ++k;
        NEXT_INST;
    }
    else if(p1.type==Z_STR && (p2.type==Z_INT || p2.type == Z_INT64))
    {
        PromoteType(p2,Z_INT64);
        if(p2.l < 0)
        {
        orgk = k - program;
        spitErr(ValueError,"Cannot multiply string by a negative integer!");
        NEXT_INST;
        }
        ZStr* src = (ZStr*)p1.ptr;
        ZStr* res = vm_allocString(src->len * p2.l);
        if(src->len != 0)
        {
        for(size_t i=0;i<(size_t)p2.l;i++)
            memcpy(res->val+(i*src->len), src->val,src->len);
        }
        p1.type = Z_STR;
        p1.ptr = (void*)res;
        ZList_push(&STACK,p1);
        DoThreshholdBusiness();
        ++k;
        NEXT_INST;
    }
    else
    {
        orgk = k - program;
        spitErr(TypeError, "Error operator '*' unsupported for " + fullform(p1.type) + " and " + fullform(p2.type));
        NEXT_INST;
    }

    if (t == Z_INT)
    {
        c.type = Z_INT;
        if (!multiplication_overflows(p1.i, p2.i))
        {
        c.i = p1.i * p2.i;
        ZList_push(&STACK,c);
        k++; NEXT_INST;
        }
        orgk = k - program;
        if (multiplication_overflows((int64_t)p1.i, (int64_t)p2.i))
        {
        spitErr(OverflowError, "Overflow occurred");
        NEXT_INST;
        }
        c.type = Z_INT64;
        c.l = (int64_t)(p1.i) * (int64_t)(p2.i);
        ZList_push(&STACK,c);
    }
    else if (t == Z_FLOAT)
    {
        if (multiplication_overflows(p1.f, p2.f))
        {
        orgk = k - program;
        spitErr(OverflowError, "Floating point overflow during multiplication");
        NEXT_INST;
        }
        c.type = Z_FLOAT;
        c.f = p1.f * p2.f;
        ZList_push(&STACK,c);
    }
    else if (t == Z_INT64)
    {
        if (multiplication_overflows(p1.l, p2.l))
        {
        orgk = k - program;
        spitErr(OverflowError, "Error overflow during solving expression.");
        NEXT_INST;
        }
        c.type = Z_INT64;
        c.l = p1.l * p2.l;
        ZList_push(&STACK,c);
    }
    k++; NEXT_INST;
    }
    CASE_CP MEMB:
    {
    orgk = k - program;
    ZObject a;
    ZList_fastpop(&STACK,&a);
    ++k;
    memcpy(&i1, k, sizeof(int32_t));
    k += 3;
    const char* mname = strings[i1].val;
    
    if (a.type == Z_MODULE)
    {
        Module *m = (Module *)a.ptr;
        
        if (!StrMap_get(&(m->members),mname,&p3))
        {
        spitErr(NameError, "Error module object has no member named '" + (string)mname + "' ");
        NEXT_INST;
        }
        ZList_push(&STACK,p3);
        ++k;
        NEXT_INST;
    }
    else if(a.type == Z_OBJ)
    {
        KlassObject *ptr = (KlassObject *)a.ptr;
        ZObject tmp;
        if (!StrMap_get(&(ptr->members),mname,&tmp))
        {
            if(StrMap_get(&(ptr->privateMembers),mname,&tmp))
            {
            FunObject *A = executing.back();
            if (A == NULL || ptr->klass != A->klass)
            {
                spitErr(AccessError, "Error cannot access private member " + (string)mname + " of class " + ptr->klass->name + "'s object!");
                NEXT_INST;
            }
            ZList_push(&STACK,tmp);
            k += 1;
            NEXT_INST;
            }
            else
            {
            spitErr(NameError, "Error object has no member named " + (string)mname);
            NEXT_INST;
            }
        }
        
        ZList_push(&STACK,tmp);
    }
    else if(a.type == Z_CLASS)
    {
        Klass *ptr = (Klass *)a.ptr;
        ZObject tmp;
        if (!StrMap_get(&(ptr->members),mname,&tmp))
        {
            if (StrMap_get(&(ptr->privateMembers),mname,&tmp))
            {
            FunObject *A = executing.back();
            if (A == NULL)
            {
                spitErr(AccessError, "Error cannot access private member " + (string)mname + " of class " + ptr->name + "!");
                NEXT_INST;
            }
            if (ptr != A->klass)
            {
                spitErr(AccessError, "Error cannot access private member " + (string)mname + " of class " + ptr->name + "!");
                NEXT_INST;
            }
            ZList_push(&STACK,tmp);
            k += 1;
            NEXT_INST;
            }
            else
            {
            spitErr(NameError, "Error class has no member named " + (string)mname);
            NEXT_INST;
            }
        }
        ZList_push(&STACK,tmp);
    }
    else
    {
        spitErr(TypeError, "Error member operator unsupported for type "+fullform(a.type));
        NEXT_INST;
    }
    k++; NEXT_INST;
    }
    CASE_CP LOAD_FUNC:
    {
    k += 1;
    int32_t p;
    memcpy(&p, k, sizeof(int32_t));
    k += 4;
    int32_t idx;
    memcpy(&idx, k, sizeof(int32_t));
    k += 4;
    FunObject *fn = vm_allocFunObject();
    fn->i = p;
    fn->args = *k;
    fn->name = strings[idx].val;
    p1.type = Z_FUNC;
    p1.ptr = (void *)fn;
    k++;
    i2 = (int32_t)*k;
    ZList_resize(&(fn -> opt),i2);
    memcpy(fn->opt.arr,STACK.arr + STACK.size-i2,sizeof(ZObject)*i2);
    ZList_eraseRange(&STACK,STACK.size - i2 , STACK.size - 1);
    ZList_push(&STACK,p1);
    DoThreshholdBusiness();
    k++; NEXT_INST;
    }
    CASE_CP LOAD_CO:
    {
    k += 1;
    int32_t p;
    memcpy(&p, k, sizeof(int32_t));
    k += 4;
    int32_t idx;
    memcpy(&idx, k, sizeof(int32_t));
    k += 4;
    ZObject co;
    FunObject* fn = vm_allocCoroutine();
    fn->args = *k;
    fn->i = p;
    fn->name = "Coroutine";
    fn->klass = nullptr;
    co.type = 'g';
    co.ptr = (void*)fn;
    ZList_push(&STACK,co);
    k++; NEXT_INST;
    }
    CASE_CP BUILD_CLASS:
    {
    k += 1;
    int32_t N;
    memcpy(&N, k, sizeof(int32_t));
    k += 4;
    int32_t idx;
    memcpy(&idx, k, sizeof(int32_t));
    k += 3;
    ZObject klass;
    klass.type = Z_CLASS;
    Klass *obj = vm_allocKlass();
    obj->name = strings[idx].val;
    values.clear();
    names.clear();
    for (int32_t i = 1; i <= N; i++)
    {
        ZList_fastpop(&STACK,&p1);
        if (p1.type == Z_FUNC)
        {
        FunObject *ptr = (FunObject *)p1.ptr;
        ptr->klass = obj;
        }
        values.push_back(p1);
    }
    for (int32_t i = 1; i <= N; i++)
    {
        ZList_fastpop(&STACK,&p1);
        names.push_back(p1);
    }
    for (int32_t i = 0; i < N; i += 1)
    {
        char* propName = ((ZStr*)names[i].ptr)->val;
        if (propName[0] == '@')
        StrMap_emplace(&(obj->privateMembers),propName+1,values[i]);
        else
        StrMap_emplace(&(obj->members),propName,values[i]);
    }
    klass.ptr = (void *)obj;
    ZList_push(&STACK,klass);
    k++; NEXT_INST;
    }
    CASE_CP BUILD_DERIVED_CLASS:
    {
    orgk = k - program;
    k += 1;
    int32_t N;
    memcpy(&N, k, sizeof(int32_t));
    k += 4;
    memcpy(&i1, k, sizeof(int32_t));
    k += 3;
    // N is total new class members
    // i1 is idx of class name in strings array
    ZObject klass;
    klass.type = Z_CLASS;
    Klass *d = vm_allocKlass();
    d->name = strings[i1].val; // strings are not deallocated until exit, so no problem
    names.clear();
    values.clear();
    for (int32_t i = 1; i <= N; i++)
    {
        ZList_fastpop(&STACK,&p1);
        if (p1.type == Z_FUNC)
        {
        FunObject *ptr = (FunObject *)p1.ptr;
        ptr->klass = d;
        }
        values.push_back(p1);
    }
    for (int32_t i = 1; i <= N; i++)
    {
        ZList_fastpop(&STACK,&p1);
        names.push_back(p1);
    }
    ZObject baseClass;
    ZList_fastpop(&STACK,&baseClass);
    if (baseClass.type != Z_CLASS)
    {
        spitErr(TypeError, "Error class can not be derived from object of non class type!");
        NEXT_INST;
    }
    Klass *Base = (Klass *)baseClass.ptr;

    for (int32_t i = 0; i < N; i += 1)
    {
        char* propName = ((ZStr*)names[i].ptr) -> val;
        if (propName[0] == '@')
        StrMap_emplace(&(d->privateMembers),propName+1,values[i]);
        else
        StrMap_emplace(&(d->members),propName,values[i]);
    }

    for (size_t it = 0;it < Base->members.capacity;it++)
    {
        if(Base->members.table[it].stat != SM_OCCUPIED)
            continue;
        auto& e = Base->members.table[it];
        const char* n = e.key;
        if (strcmp(n , "super") == 0)//do not add base class's super to this class
            continue;
        ZObject* ref1;
        if (!(ref1 = StrMap_getRef(&(d->members),n)))//member is not overriden in child
        {
            if (!(ref1 = StrMap_getRef(&(d->privateMembers),n)))
            {
                p1 = e.val;
                if (p1.type == Z_FUNC)
                {
                    FunObject *p = (FunObject *)p1.ptr;
                    FunObject *rep = vm_allocFunObject();
                    rep->args = p->args;
                    rep->i = p->i;
                    rep->klass = d;
                    rep->name = p->name;
                    ZList_resize(&(rep->opt),p->opt.size);
                    memcpy(rep->opt.arr,p->opt.arr,sizeof(ZObject)*p->opt.size);
                    p1.type = Z_FUNC;
                    p1.ptr = (void *)rep;
                }
                StrMap_set(&(d->members),e.key, p1); //override
            }
        }
    }
    for (size_t it = 0;it < Base->privateMembers.capacity;it++)
    {
        if(Base->privateMembers.table[it].stat != SM_OCCUPIED)
            continue;
        auto& e = Base->privateMembers.table[it];
        const char* n = e.key;
        ZObject* ref;
        if (!(ref = StrMap_getRef(&(d->privateMembers),n)))
        {
        if (!(ref = StrMap_getRef(&(d->members),n)))
        {
            p1 = e.val;
            if (p1.type == Z_FUNC)
            {
            FunObject *p = (FunObject *)p1.ptr;
            FunObject *rep = vm_allocFunObject();
            rep->args = p->args;
            rep->i = p->i;
            rep->klass = d;
            rep->name = p->name;
            ZList_resize(&(rep->opt),p->opt.size);
            memcpy(rep->opt.arr,p->opt.arr,sizeof(ZObject)*p->opt.size);
            p1.type = Z_FUNC;
            p1.ptr = (void *)rep;
            }
            StrMap_emplace(&(d->privateMembers),n, p1);
        }
        }
    }

    StrMap_emplace(&(d->members),"super",baseClass);
    klass.ptr = (void *)d;
    ZList_push(&STACK,klass);
    k++; NEXT_INST;
    }
    CASE_CP JMPIFFALSENOPOP:
    {
    k += 1;
    memcpy(&i1, k, sizeof(int32_t));
    k += 3;
    p1 = STACK.arr[STACK.size - 1];
    if (p1.type == Z_NIL || (p1.type == Z_BOOL && p1.i == 0))
    {
        k = k + i1 + 1;
        NEXT_INST;
    }
    ZList_fastpop(&STACK,&p1);
    k++; NEXT_INST;
    }
    CASE_CP NOPOPJMPIF:
    {
    k += 1;
    memcpy(&i1, k, sizeof(int32_t));
    k += 3;
    p1 = STACK.arr[STACK.size - 1];
    if (p1.type == Z_NIL || (p1.type == Z_BOOL && p1.i == 0))
    {
        ZList_fastpop(&STACK,&p1);
        k++; NEXT_INST;
    }
    else
    {
        k = k + i1 + 1;
        NEXT_INST;
    }
    }
    CASE_CP LOAD_STR:
    {
    k += 1;
    memcpy(&i1, k, sizeof(int32_t));
    k += 3;
    p1.type = Z_STR;
    p1.ptr = (void *)&strings[i1];
    ZList_push(&STACK,p1);
    k++; NEXT_INST;
    }
    CASE_CP CALLUDF:
    {
    orgk = k - program;
    ZObject fn;
    ZList_fastpop(&STACK,&fn);
    k += 1;
    int32_t N = *k;
    if (fn.type == Z_FUNC)
    {
        FunObject *obj = (FunObject *)fn.ptr;
        if ((size_t)N + obj->opt.size < obj->args || (size_t)N > obj->args)
        {
        spitErr(ArgumentError, "Error function " + (string)obj->name + " takes " + to_string(obj->args) + " arguments," + to_string(N) + " given!");
        NEXT_INST;
        }
        callstack.push_back(k + 1);
        frames.push_back(STACK.size - N);
        if (callstack.size() >= 1000)
        {
        spitErr(MaxRecursionError, "Error max recursion limit 1000 reached.");
        NEXT_INST;
        }
        for (size_t i = obj->opt.size - (obj->args - N); i < obj->opt.size; i++)
            ZList_push(&STACK,obj->opt.arr[i]);
        executing.push_back(obj);
        k = program + obj->i;
        NEXT_INST;
    }
    else if (fn.type == Z_NATIVE_FUNC)
    {
        NativeFunction *A = (NativeFunction *)fn.ptr;
        NativeFunPtr f = A->addr;
        p4.type = Z_NIL;
        ZObject* args = STACK.arr + (STACK.size-N);
        if(A->signature)
        {
        size_t len = strlen(A->signature);
        if(len != N)
        {
            spitErr(ArgumentError,(string)"Native function "+ (string)A->name + (string)" takes "+to_string(len)+" arguments, "+to_string(N)+" given!");
            NEXT_INST;
        }
        size_t i = 0;
        while(i<N)
        {
            if(args[i].type != A->signature[i])
            {
            spitErr(TypeError,"Argument "+to_string(i+1)+" to "+(string)A->name+(string)+"should be a "+fullform(A->signature[i]));
            NEXT_INST;
            }
            i+=1;
        }
        }
        p4 = f(&(STACK.arr[STACK.size - N]), N);
        if (p4.type == Z_ERROBJ)
        {
        s1 = "Native Function:  " + *(string *)p4.ptr;
        spitErr((Klass*)p4.ptr, s1);
        NEXT_INST;
        }
        if (fullform(p4.type) == "Unknown" && p4.type != Z_NIL)
        {
        spitErr(ValueError, "Error invalid response from module!");
        NEXT_INST;
        }
        ZList_eraseRange(&STACK,STACK.size-N,STACK.size-1);
        ZList_push(&STACK,p4);
    }
    else if (fn.type == Z_CLASS)
    {

        KlassObject *obj = vm_allocKlassObject(AS_KLASS(fn)); // instance of class


        s1 = ((Klass*)fn.ptr) -> name;
        
        ZObject construct;
        if (StrMap_get(&(obj->members),"__construct__", &construct))
        {
        if (construct.type == Z_FUNC)
        {
            FunObject *p = (FunObject *)construct.ptr;
            if ((size_t)N + p->opt.size + 1 < p->args || (size_t)N + 1 > p->args)
            {
            spitErr(ArgumentError, "Error constructor of class " + (string)((Klass *)fn.ptr)->name + " takes " + to_string(p->args - 1) + " arguments," + to_string(N) + " given!");
            NEXT_INST;
            }
            ZObject r;
            r.type = Z_OBJ;
            r.ptr = (void *)obj;
            callstack.push_back(k + 1);
            ZList_insert(&STACK,STACK.size - N,r);
            frames.push_back(STACK.size-N-1);
            for (size_t i = p->opt.size - (p->args - 1 - N); i < p->opt.size; i++)
            {
            ZList_push(&STACK,p->opt.arr[i]);
            }
            k = program + p->i;
            executing.push_back(p);
            DoThreshholdBusiness();
            NEXT_INST;
        }
        else if (construct.type == Z_NATIVE_FUNC)
        {
            NativeFunction *M = (NativeFunction *)construct.ptr;
            ZObject *args = NULL;
            ZObject r;
            r.type = Z_OBJ;
            r.ptr = (void *)obj;
            ZList_insert(&STACK,STACK.size - N,r);
            args = &STACK.arr[STACK.size - (N + 1)];
            if(M->signature)
            {
            size_t len = strlen(M->signature);
            if(len != N+1)
            {
                spitErr(ArgumentError,(string)"Native function "+ (string)M->name + (string)" takes "+to_string(len)+" arguments, "+to_string(N+1)+" given!");
                NEXT_INST;
            }
            size_t i = 0;
            while(i<N+1)
            {
                if(args[i].type != M->signature[i])
                {
                spitErr(TypeError,"Argument "+to_string(i+1)+" to "+(string)M->name+(string)+"should be a "+fullform(M->signature[i]));
                NEXT_INST;
                }
                i+=1;
            }
            }
            p4 = M->addr(args, N + 1);
            ZList_eraseRange(&STACK,STACK.size-(N+1),STACK.size-1);
            if (p4.type == Z_ERROBJ)
            {
            KlassObject* E = (KlassObject*)p4.ptr;
            ZObject msg;
            StrMap_get(&(E->members),"msg",&msg);
            char* errmsg = ((ZStr*)msg.ptr) -> val;
            spitErr((Klass*)p4.ptr, s1+ "." + "__construct__:  " + errmsg);
            NEXT_INST;
            }
            ZList_push(&STACK,r);
            DoThreshholdBusiness();
            k++;
            NEXT_INST;
        }
        else
        {
            spitErr(TypeError, "Error constructor of class " + (string)((Klass *)fn.ptr)->name + " is not a function!");
            NEXT_INST;
        }
        }
        else
        {
        if (N != 0)
        {
            spitErr(ArgumentError, "Error constructor class " + (string)((Klass *)fn.ptr)->name + " takes 0 arguments!");
            NEXT_INST;
        }
        }
        ZObject r;
        r.type = Z_OBJ;
        r.ptr = (void *)obj;
        ZList_push(&STACK,r);
        DoThreshholdBusiness();
    }
    else if (fn.type == 'g')
    {
        FunObject* f = (FunObject*)fn.ptr;
        if ((size_t)N != f->args)
        {
        spitErr(ArgumentError, "Error coroutine " + *(string *)fn.ptr + " takes " + to_string(f->args) + " arguments," + to_string(N) + " given!");
        NEXT_INST;
        }
        Coroutine *g = vm_allocCoObj();
        g->fun = f;
        g->curr = f->i;
        g->state = SUSPENDED;
        ZObject* locals = STACK.arr + STACK.size - N;
        STACK.size -= N;
        ZList_resize(&(g->locals),N);
        if(N != 0)
            memcpy(g->locals.arr,locals,N*sizeof(ZObject));
        g->giveValOnResume = false;
        ZObject T;
        T.type = Z_COROUTINE_OBJ;
        T.ptr = g;
        ZList_push(&STACK,T);
        DoThreshholdBusiness();
    }
    else
    {
        spitErr(TypeError, "Error type " + fullform(fn.type) + " not callable!");
        NEXT_INST;
    }
    k++; NEXT_INST;
    }
    CASE_CP MOD:
    {
    ZObject a,b;
    ZList_fastpop(&STACK,&b);
    ZList_fastpop(&STACK,&a);
    if (a.type == Z_OBJ)
    {
        invokeOperator("__mod__", a, 2, "%", &b);
        NEXT_INST;
    }
    ZObject c;
    char t;
    if (isNumeric(a.type) && isNumeric(b.type))
    {
        if (a.type == Z_FLOAT || b.type == Z_FLOAT)
        {
        orgk = k - program;
        spitErr(TypeError, "Error modulo operator % unsupported for floats!");
        NEXT_INST;
        }
        else if (a.type == Z_INT64 || b.type == Z_INT64)
        {
        t = Z_INT64;
        }
        else if (a.type == Z_INT || b.type == Z_INT)
        t = Z_INT;
        PromoteType(a, t);
        PromoteType(b, t);
    }
    else
    {
        orgk = k - program;
        spitErr(TypeError, "Error operator '%' unsupported for " + fullform(a.type) + " and " + fullform(b.type));
        NEXT_INST;
    }
    //

    if (t == Z_INT)
    {
        c.type = Z_INT;
        if (b.i == 0)
        {
        orgk = k - program;
        spitErr(MathError, "Error modulo by zero");
        NEXT_INST;
        }
        if ((a.i == INT_MIN) && (b.i == -1))
        {
        /* handle error condition */
        c.type = Z_INT64;
        c.l = (int64_t)a.i % (int64_t)b.i;
        ZList_push(&STACK,c);
        }
        c.i = a.i % b.i;
        ZList_push(&STACK,c);
    }
    else if (t == Z_INT64)
    {
        c.type = Z_INT64;
        if (b.l == 0)
        {
        orgk = k - program;
        spitErr(MathError, "Error modulo by zero");
        NEXT_INST;
        }
        if ((a.l == LLONG_MIN) && (b.l == -1))
        {
        orgk = k - program;
        spitErr(OverflowError, "Error modulo of INT32_MIN by -1 causes overflow!");
        NEXT_INST;
        }
        c.l = a.l % b.l;
        ZList_push(&STACK,c);
    }
    k++; NEXT_INST;
    }
    CASE_CP INPLACE_INC:
    {
    orgk = k - program;
    k += 1;
    memcpy(&i1, k, sizeof(int32_t));
    k += 3;
    char t = STACK.arr[frames.back() + i1].type;
    if (t == Z_INT)
    {
        if (STACK.arr[frames.back() + i1].i == INT_MAX)
        {
        STACK.arr[frames.back() + i1].l = (int64_t)INT_MAX + 1;
        STACK.arr[frames.back() + i1].type = Z_INT64;
        }
        else
        STACK.arr[frames.back() + i1].i += 1;
    }
    else if (t == Z_INT64)
    {
        if (STACK.arr[frames.back() + i1].l == LLONG_MAX)
        {
        spitErr(OverflowError, "Error numeric overflow");
        NEXT_INST;
        }
        STACK.arr[frames.back() + i1].l += 1;
    }
    else if (t == Z_FLOAT)
    {
        if (STACK.arr[frames.back() + i1].f == FLT_MAX)
        {
        spitErr(OverflowError, "Error numeric overflow");
        NEXT_INST;
        }
        STACK.arr[frames.back() + i1].f += 1;
    }
    else
    {
        spitErr(TypeError, "Error cannot add numeric constant to type " + fullform(t));
        NEXT_INST;
    }
    k++; NEXT_INST;
    }
    CASE_CP SUB:
    {
    ZObject a,b;
    ZList_fastpop(&STACK,&b);
    ZList_fastpop(&STACK,&a);
    if (a.type == Z_OBJ)
    {
        invokeOperator("__sub__", a, 2, "-", &b);
        NEXT_INST;
    }
    ZObject c;
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
        PromoteType(a, t);
        PromoteType(b, t);
        
    }
    else
    {
        orgk = k - program;
        spitErr(TypeError, "Error operator '-' unsupported for " + fullform(a.type) + " and " + fullform(b.type));
        NEXT_INST;
    }

    //
    if (t == Z_INT)
    {
        c.type = Z_INT;
        if (!subtraction_overflows(a.i, b.i))
        {
        c.i = a.i - b.i;
        STACK.arr[STACK.size++] = c;
        k++; NEXT_INST;
        }
        if (subtraction_overflows((int64_t)a.i, (int64_t)b.i))
        {
        orgk = k - program;
        spitErr(OverflowError, "Overflow occurred");
        }
        c.type = Z_INT64;
        c.l = (int64_t)(a.i) - (int64_t)(b.i);
        STACK.arr[STACK.size++] = c;
    }
    else if (t == Z_FLOAT)
    {
        if (subtraction_overflows(a.f, b.f))
        {
        orgk = k - program;
        spitErr(OverflowError, "Floating point overflow during subtraction");
        NEXT_INST;
        }
        c.type = Z_FLOAT;
        c.f = a.f - b.f;
        STACK.arr[STACK.size++] = c;
    }
    else if (t == Z_INT64)
    {
        if (subtraction_overflows(a.l, b.l))
        {
        orgk = k - program;
        spitErr(OverflowError, "Error overflow during solving expression.");
        NEXT_INST;
        }
        c.type = Z_INT64;
        c.l = a.l - b.l;
        STACK.arr[STACK.size++] = c;
    }
    k++; NEXT_INST;
    }
    CASE_CP DIV:
    {
    ZObject b,a;
    ZList_fastpop(&STACK,&b);
    ZList_fastpop(&STACK,&a);
    if (a.type == Z_OBJ)
    {
        invokeOperator("__div__", a, 2, "/", &b);
        NEXT_INST;
    }
    ZObject c;
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
        PromoteType(a, t);
        PromoteType(b, t);
    }
    else
    {
        orgk = k - program;
        spitErr(TypeError, "Error operator '/' unsupported for " + fullform(a.type) + " and " + fullform(b.type));
        NEXT_INST;
    }

    if (t == Z_INT)
    {
        if (b.i == 0)
        {
        orgk = k - program;
        spitErr(MathError, "Error division by zero");
        NEXT_INST;
        }
        c.type = Z_INT;
        if (!division_overflows(a.i, b.i))
        {
        c.i = a.i / b.i;
        STACK.arr[STACK.size++] = c;
        k++; NEXT_INST;
        }
        if (division_overflows((int64_t)a.i, (int64_t)b.i))
        {
        orgk = k - program;
        spitErr(OverflowError, "Overflow occurred");
        NEXT_INST;
        }
        c.type = Z_INT64;
        c.l = (int64_t)(a.i) / (int64_t)(b.i);
        STACK.arr[STACK.size++] = c;
        k++; NEXT_INST;
    }
    else if (t == Z_FLOAT)
    {
        if (b.f == 0)
        {
        orgk = k - program;
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
        orgk = k - program;
        spitErr(MathError, "Error division by zero");
        NEXT_INST;
        }
        if (division_overflows(a.l, b.l))
        {
        orgk = k - program;
        spitErr(OverflowError, "Error overflow during solving expression.");
        NEXT_INST;
        }
        c.type = Z_INT64;
        c.l = a.l / b.l;
        STACK.arr[STACK.size++] = c;
    }
    k++; NEXT_INST;
    }
    CASE_CP JMP:
    {
    k += 1;
    memcpy(&i1, k, sizeof(int32_t));
    k += 3;
    int32_t where = k - program + i1 + 1;
    k = program + where - 1;
    k++; NEXT_INST;
    }
    CASE_CP JMPNPOPSTACK:
    {
    k += 1;
    int32_t N;
    memcpy(&N, k, sizeof(int32_t));
    k += 4;
    ZList_eraseRange(&STACK,STACK.size - N,STACK.size - 1);  
    memcpy(&i1, k, sizeof(int32_t));
    k += 3;
    int32_t where = k - program + i1;
    k = program + where;
    k++; NEXT_INST;
    }
    CASE_CP JMPIF:
    {
    k += 1;
    memcpy(&i1, k, sizeof(int32_t));
    k += 3;
    int32_t where = k - program + i1 + 1;
    ZList_fastpop(&STACK,&p1);
    if (p1.i && p1.type == Z_BOOL)
        k = program + where - 1;
    k++; 
    NEXT_INST;
    }
    CASE_CP THROW:
    {
    orgk = k - program;
    ZList_fastpop(&STACK,&p3); //val
    if(p3.type != Z_OBJ)
    {
        spitErr(TypeError,"Value of type "+fullform(p3.type)+" not throwable!");
        NEXT_INST;
    }
    KlassObject* ki = (KlassObject*)p3.ptr;
    ZObject msg;
    if( !StrMap_get(&(ki->members),"msg",&msg) || msg.type!=Z_STR)
    {
        spitErr(ThrowError,"Object does not have member 'msg' or it is not a string!");
        NEXT_INST;
    }
    if (except_targ.size() == 0)
    {
        ///IMPORTANT
        spitErr(ki->klass,((ZStr*)(msg.ptr))->val );
        NEXT_INST;
    }
    k = except_targ.back();
    i1 = STACK.size - tryStackCleanup.back();
    STACK.size -= i1;
    i1 = frames.size() - tryLimitCleanup.back();
    frames.erase(frames.end() - i1, frames.end());
    STACK.arr[STACK.size++] = p3;
    except_targ.pop_back();
    tryStackCleanup.pop_back();
    tryLimitCleanup.pop_back();
    NEXT_INST;
    }
    CASE_CP ONERR_GOTO:
    {
    k += 1;
    memcpy(&i1, k, 4);
    except_targ.push_back((uint8_t *)program + i1);
    tryStackCleanup.push_back(STACK.size);
    tryLimitCleanup.push_back(frames.size());
    k += 4;
    NEXT_INST;
    }
    CASE_CP POP_EXCEP_TARG:
    {
    except_targ.pop_back();
    tryStackCleanup.pop_back();
    k++; NEXT_INST;
    }
    CASE_CP GOTO:
    {
    k += 1;
    memcpy(&i1, k, sizeof(int32_t));
    k = program + i1 - 1;
    k++; 
    NEXT_INST;
    }
    CASE_CP GOTONPSTACK:
    {
    k += 1;
    memcpy(&i1, k, sizeof(int32_t));
    k += 4;
    STACK.size -= i1;
    memcpy(&i1, k, sizeof(int32_t));
    k = program + i1;
    NEXT_INST;
    }
    CASE_CP JMPIFFALSE:
    {

    k += 1;
    memcpy(&i1, k, sizeof(int32_t));
    k += 3;
    ZList_fastpop(&STACK,&p1);
    if (p1.type == Z_NIL || (p1.type == Z_BOOL && p1.i == 0))
    {
        k = k + i1 + 1;
        NEXT_INST;
    }
    k++; NEXT_INST;
    }
    CASE_CP SELFMEMB:
    {
    orgk = k - program;
    ZObject a = STACK.arr[frames.back()];
    ++k;
    memcpy(&i1, k, sizeof(int32_t));
    k += 3;
    const char* mname = strings[i1].val;//optimize
    if(a.type == Z_OBJ)
    {
        KlassObject *ptr = (KlassObject *)a.ptr;
        ZObject tmp;
        if (!StrMap_get(&(ptr->members),mname,&tmp))
        {
            if (StrMap_get(&(ptr->privateMembers),mname,&tmp))
            {
            FunObject *A = executing.back();
            if (A == NULL || A->klass != ptr->klass)
            {
                spitErr(AccessError, "Cannot access private member " + (string)mname + " of class " + ptr->klass->name + "'s object!");
                NEXT_INST;
            }
            ZList_push(&STACK,tmp);
            k += 1;
            NEXT_INST;
            }
            else
            {
            spitErr(NameError, "Object 'self' has no member named " + (string)mname);
            NEXT_INST;
            }
        }
        ZList_push(&STACK,tmp);
    }
    else
    {
        spitErr(TypeError, "Can not access member "+(string)mname+", self not an object!");
        NEXT_INST;
    }
    k++; NEXT_INST;
    }
    CASE_CP ASSIGNSELFMEMB:
    {
    orgk = k - program;
    ZObject val;
    ZList_fastpop(&STACK,&val);
    ZObject Parent = STACK.arr[frames.back()];
    k++;
    memcpy(&i1, k, 4);
    k += 3;
    const char* s1 = strings[i1].val;
    
    if (Parent.type != Z_OBJ)
    {
        spitErr(TypeError, "Cannot access variable "+(string)s1+" ,self is not a class object!");
        NEXT_INST;
    }
    KlassObject *ptr = (KlassObject *)Parent.ptr;
    ZObject* ref;
    if (! ( ref = StrMap_getRef(&(ptr->members),s1) ))
    {
        if (( ref = StrMap_getRef(&(ptr->privateMembers),s1) ))
        {
        // private member
        FunObject *A = executing.back();
        if (A == NULL || A->klass != ptr->klass)
        {
            spitErr(AccessError, "Cannot access private member " + (string)s1 + " of class " + ptr->klass->name + "'s object!");
            NEXT_INST;
        }
        *ref = val;
        k += 1;
        NEXT_INST;
        }          
        spitErr(NameError, "Object has no member named " + (string)s1);
        NEXT_INST;
    }
    *ref = val;
    k++; NEXT_INST;
    }
    CASE_CP GC:
    {
    mark();
    collectGarbage();
    k++; NEXT_INST;
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
    k += 1;

} // end while loop
#endif


if (STACK.size != 0 && panic)
{
    fprintf(stderr,"An InternalError occurred.Error Code: 15\n");

    exit(1);
}

} // end function interpret
VM::~VM()
{

vector<void*> toerase;
//Call destructors of all objects in memory pool
for (auto it=memory.begin();it!=memory.end();)
{
    MemInfo m = (*it).second;
    //call destructor for each object only once
    if (m.type == Z_OBJ)
    {
    KlassObject *obj = (KlassObject *)(*it).first;
    ZObject dummy;
    dummy.type = Z_OBJ;
    dummy.ptr = (*it).first;
    ZObject rr;
    ZObject p1;
    if (StrMap_get(&(obj->members),"__del__",&p1))
    {
        if(p1.type == Z_FUNC || p1.type==Z_NATIVE_FUNC)
        {
        vm_callObject(&p1,&dummy,1,&rr);
        }
    }
    toerase.push_back((*it).first);
    it = memory.erase(it);
    continue;
    }
    else
    ++it;
}
// all deletors called
// now unload all modules
typedef void (*unload)(void);
for (auto e : moduleHandles)
{
    #ifdef _WIN32
    unload ufn = (unload)GetProcAddress(e, "unload");
    if (ufn)
    ufn();
    FreeLibrary(e);
    #else
    unload ufn = (unload)dlsym(e, "unload");
    if (ufn)
        ufn();
    dlclose(e);
    #endif
}
for (auto e : toerase) // deleting these here, because collectGarbage will call deletors too
// and we have already done that
{
    KlassObject* p = (KlassObject*)e;
    StrMap_destroy(&(p->members));
    StrMap_destroy(&(p->privateMembers));
    delete (KlassObject*)e;
}
if(constants)
    delete[] constants;
STACK.size = 0;
important.clear();
mark(); // clearing the STACK and marking objects will result in all objects being deleted
// which is what we want
collectGarbage();
ZList_destroy(&STACK);
ZList_destroy(&aux);
for(auto e: strings)
{
    delete[] e.val;
}
}
VM vm;
//////////
ZObject ZObjFromStr(const char* str)// makes deep copy of str
{
  size_t len = strlen(str);
  ZStr* ptr = vm_allocString(len);
  memcpy(ptr->val,str,len);
  ZObject ret;
  ret.type = 's';
  ret.ptr = (void*)ptr;
  return ret;
}
ZObject Z_Err(Klass* errKlass,const char* des)
{
  ZObject ret;
  KlassObject* p = vm_allocKlassObject(errKlass);
  StrMap_set(&(p->members),"msg",ZObjFromStr(des)); // OPTIMIZE!
  ret.type = Z_ERROBJ;//indicates an object in thrown state
  ret.ptr = (void*) p;
  return ret;
}
ZList* vm_allocList()
{
  ZList *p = new(nothrow) ZList;
  if (!p)
  {
    fprintf(stderr,"allocList(): error allocating memory!\n");
    exit(0);
  }
  ZList_init(p);
  vm.allocated += sizeof(ZList);
  MemInfo m;
  m.type = Z_LIST;
  m.isMarked = false;
  vm.memory.emplace((void *)p, m);
  return p;
}
ZByteArr* vm_allocByteArray()
{
  auto p = new(nothrow) ZByteArr;
  if (!p)
  {
    fprintf(stderr,"allocByteArray(): error allocating memory!\n");
    exit(0);
  }
  ZByteArr_init(p);
  vm.allocated += sizeof(std::vector<uint8_t>);
  MemInfo m;
  m.type = Z_BYTEARR;
  m.isMarked = false;
  vm.memory.emplace((void *)p, m);
  return p;
}
uint8_t* vm_allocRaw(size_t len)
{
  // the stupid user who failed to send a valid
  // length, will likely fail to add NULL check
  // why degrade performance?
  //if(len == 0)
  // return NULL;

  uint8_t* p = new(nothrow) uint8_t[len];
  if (!p)
  {
    fprintf(stderr,"allocRaw(): error allocating memory!\n");
    exit(0);
  }
  vm.allocated += sizeof(len);
  MemInfo m;
  m.type = Z_RAW;
  m.isMarked = false;
  vm.memory.emplace((void *)p, m);
  return p;
}
ZStr* vm_allocString(size_t len)
{
  /*
  if(len != 0)
    return NULL;*/

  ZStr* str = new(nothrow) ZStr;
  char* buffer = new(nothrow) char[len+1];
  if (!buffer || !str)
  {
    fprintf(stderr,"allocString(): error allocating memory!\n");
    exit(0);
  }
  //The string owns this buffer
  //it will get deleted when string gets deleted
  buffer[len] = 0;
  str->val = buffer;
  str->len = len;

  vm.allocated += len + 1 + sizeof(ZStr);
  MemInfo m;
  m.type = Z_STR;
  m.isMarked = false;
  vm.memory.emplace((void *)str, m);
  return str;
}

Klass* vm_allocKlass()
{
  Klass* p = new(nothrow) Klass;
  if (!p)
  {
    fprintf(stderr,"allocKlass(): error allocating memory!\n");
    exit(0);
  }
  StrMap_init(&(p->members));
  StrMap_init(&(p->privateMembers));

  vm.allocated += sizeof(Klass);
  MemInfo m;
  m.type = Z_CLASS;
  m.isMarked = false;
  vm.memory.emplace((void *)p, m);
  return p;
}
Module* vm_allocModule()
{
  Module *p = new(nothrow) Module;
  if (!p)
  {
    fprintf(stderr,"allocModule(): error allocating memory!\n");
    exit(0);
  }
  StrMap_init(&(p->members));
  vm.allocated += sizeof(Module);
  MemInfo m;
  m.type = Z_MODULE;
  m.isMarked = false;
  vm.memory.emplace((void *)p, m);
  return p;
}
KlassObject* vm_allocKlassObject(Klass* k)
{
  KlassObject *p = new KlassObject;
  if (!p)
  {
    fprintf(stderr,"allocKlassObject(): error allocating memory!\n");
    exit(0);
  }
  StrMap_init(&(p->members));
  StrMap_init(&(p->privateMembers));
  StrMap_assign(&(p->members),&(k->members));
  StrMap_assign(&(p->privateMembers),&(k->privateMembers));
  p -> klass = k;
  vm.allocated += sizeof(KlassObject);
  MemInfo m;
  m.type = Z_OBJ;
  m.isMarked = false;
  vm.memory.emplace((void *)p, m);
  return p;
}
Coroutine* vm_allocCoObj()//allocates coroutine object
{
  Coroutine *p = new Coroutine;
  if (!p)
  {
    fprintf(stderr,"allocCoObj(): error allocating memory!\n");
    exit(0);
  }
  vm.allocated += sizeof(Coroutine);
  MemInfo m;
  m.type = 'z';
  m.isMarked = false;
  vm.memory.emplace((void *)p, m);
  ZList_init(&(p->locals));
  return p;
}
FunObject* vm_allocFunObject()
{
  FunObject *p = new FunObject;
  if (!p)
  {
    fprintf(stderr,"allocFunObject(): error allocating memory!\n");
    exit(0);
  }
  p->klass = NULL;
  vm.allocated += sizeof(FunObject);
  MemInfo m;
  m.type = Z_FUNC;
  m.isMarked = false;
  vm.memory.emplace((void *)p, m);
  ZList_init(&(p->opt));
  return p;
}
FunObject* vm_allocCoroutine() //coroutine can be represented by FunObject
{
  FunObject *p = new FunObject;
  if (!p)
  {
    fprintf(stderr,"allocCoroutine(): error allocating memory!\n");
    exit(0);
  }
  p->klass = NULL;
  vm.allocated += sizeof(FunObject);
  MemInfo m;
  m.type = 'g';
  m.isMarked = false;
  vm.memory.emplace((void *)p, m);
  return p;
}
zfile * vm_alloczfile()
{

  zfile *p = new zfile;
  if (!p)
  {
    fprintf(stderr,"alloczfile(): error allocating memory!\n");
    exit(0);
  }
  vm.allocated += sizeof(ZList);
  MemInfo m;
  m.type = Z_FILESTREAM;
  m.isMarked = false;
  vm.memory.emplace((void *)p, m);
  return p;
}
ZDict* vm_allocDict()
{
  ZDict *p = new ZDict;
  if (!p)
  {
    fprintf(stderr,"allocDict(): error allocating memory!\n");
    exit(0);
  }
  ZDict_init(p);
  vm.allocated += sizeof(ZDict);
  MemInfo m;
  m.type = Z_DICT;
  m.isMarked = false;
  vm.memory.emplace((void *)p, m);
  return p;
}
NativeFunction* vm_allocNativeFunObj()
{
  NativeFunction *p = new NativeFunction;
  if (!p)
  {
    fprintf(stderr,"allocNativeFun(): error allocating memory!\n");
    exit(0);
  }
  p->signature = NULL;
  vm.allocated += sizeof(NativeFunction);
  MemInfo m;
  m.type = Z_NATIVE_FUNC;
  m.isMarked = false;
  vm.memory.emplace((void *)p, m);
  return p;
}
//callObject also behaves as a kind of try/catch since v0.31
bool vm_callObject(ZObject* obj,ZObject* args,int N,ZObject* rr)
{
  static char buffer[1024];
  if(obj->type == Z_FUNC)
  {
     FunObject* fn = (FunObject*)obj->ptr;
     if ((size_t)N + fn->opt.size < fn->args || (size_t)N > fn->args)
     {
      static char buffer[1024];
      snprintf(buffer,1024,"Function %s takes %zu arguments, %d given!",fn->name,fn->args,N);
      *rr = Z_Err(ArgumentError, buffer);
      return false;
     }
     uint8_t* prev = vm.k;
     vm.callstack.push_back(NULL);
     vm.frames.push_back(vm.STACK.size);
     vm.executing.push_back(fn);
     for(int i=0;i<N;i++)
       ZList_push(&vm.STACK,args[i]);
     for(size_t i = fn->opt.size - (fn->args - N); i < fn->opt.size; i++)
       ZList_push(&vm.STACK,fn->opt.arr[i]);
     bool a = vm.viaCO;
     vm.viaCO = true;
     vm.interpret(fn->i,false);
     vm.viaCO = a;
     vm.k = prev;
     ZList_fastpop(&vm.STACK,rr);
     return (rr->type != Z_ERROBJ);
  }
  else if (obj->type == Z_NATIVE_FUNC)
  {
    NativeFunction *A = (NativeFunction *)obj->ptr;
    NativeFunPtr f = A->addr;
    ZObject p4;
    if(A->signature)
    {
      size_t len = strlen(A->signature);
      if(len != N)
      {
        snprintf(buffer,1024,"Native function %s takes %zu arguments, %d given!",A->name,len,N);       
        *rr = Z_Err(ArgumentError,buffer);
        return false;
      }
      size_t i = 0;
      while(i<N)
      {
        if(args[i].type != A->signature[i])
        {
          snprintf(buffer,1024,"Argument %zu to %s must be a %s\n",i+1,A->name,fullform(A->signature[i]).c_str());
          *rr = Z_Err(TypeError,buffer);
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
    if (fullform(p4.type) == "Unknown" && p4.type != Z_NIL)
    {
      *rr = Z_Err(ValueError, "Invalid response from native function!");
      return false;
    }
    *rr = p4;
    return true;
  }
  *rr = Z_Err(TypeError,"Object not callable!");
  return false;
}
void vm_markImportant(void* mem)
{
  if(vm.memory.find(mem)!=vm.memory.end())
    vm.important.push_back(mem);
  
}
void vm_unmarkImportant(void* mem)
{
  std::vector<void*>::iterator it;
  if((it = find(vm.important.begin(),vm.important.end(),mem))!=vm.important.end())
    vm.important.erase(it); 
}
