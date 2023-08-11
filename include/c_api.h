//All C friendly code is here
#ifndef PLTC_API_H
#define PLTC_API_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

//
#ifdef __cplusplus
extern "C"
{
#endif
    //Types for different plutonium datatypes
    #define PLT_LIST 'j'
    #define PLT_DICT 'a'
    #define PLT_INT 'i'
    #define PLT_INT64 'l'
    #define PLT_FLOAT 'f'
    #define PLT_BYTE  'm'
    #define PLT_NATIVE_FUNC 'y'//native function
    #define PLT_MODULE 'q'
    #define PLT_STR 's'
    #define PLT_FILESTREAM 'u'
    #define PLT_NIL 'n'
    #define PLT_OBJ 'o' //objects created using plutonium code
    #define PLT_CLASS 'v' //Plutonium code class
    #define PLT_BOOL 'b'
    #define PLT_POINTER 'p' //just a pointer that means something only to c++ code,the interpreter just stores it
    #define PLT_FUNC 'w' //plutonium code function
    #define PLT_COROUTINE 'g'
    #define PLT_COROUTINE_OBJ 'z'
    #define PLT_ERROBJ 'e' //same as an object,just in thrown state
    #define PLT_BYTEARR 'c'

   
    //C friendly structs
    typedef struct PltObject
    {
        union
        {
            double f;
            int64_t l;
            int32_t i;
            void* ptr;
        };
        char type;
    }PltObject;
    
    typedef struct FileObject
    {
      FILE* fp;
      bool open;
    }FileObject;
    typedef PltObject(*NativeFunPtr)(PltObject*,int);
    
    extern PltObject nil;
    extern PltObject Error;
    extern PltObject TypeError;
    extern PltObject ValueError;
    extern PltObject MathError; 
    extern PltObject NameError;
    extern PltObject IndexError;
    extern PltObject ArgumentError;
    extern PltObject FileIOError;
    extern PltObject KeyError;
    extern PltObject OverflowError;
    extern PltObject FileOpenError;
    extern PltObject FileSeekError; 
    extern PltObject ImportError;
    extern PltObject ThrowError;
    extern PltObject MaxRecursionError;
    extern PltObject AccessError;
    //Dictionary
    PltObject allocDict();
    PltObject dictGet(PltObject,PltObject,bool*);
    bool dictSet(PltObject,PltObject,PltObject);
    size_t dictSize(PltObject);
    void* newDictIter(PltObject);
    void freeDictIter(void*);
    void advanceDictIter(PltObject,void**);
    void setDictIterValue(void*);
    PltObject getDictIterValue(void*);
    PltObject getDictIterKey(void*);
    
    //Bytearray
    PltObject allocBytearray();
    void btPush(PltObject,uint8_t);
    bool btPop(PltObject,PltObject*);
    size_t btSize(PltObject);
    void btResize(PltObject,size_t);
    uint8_t* btAsArray(PltObject);
    //Plutonium list
    PltObject allocList();
    void listPush(PltObject,PltObject);
    bool listPop(PltObject,PltObject*);
    size_t listSize(PltObject);
    void listResize(PltObject,size_t);
    PltObject* listAsArray(PltObject);
    //Module
    PltObject allocModule(const char*);
    void addModuleMember(PltObject,const char*,PltObject);
    //Klass
    PltObject allocKlass(const char*);
    void klassAddMember(const char*,PltObject);
    void klassAddPrivateMember(const char*,PltObject);
    //Klass Objects
    PltObject allocObj(PltObject);
    void objAddMember(const char*,PltObject);
    void objAddPrivateMember(const char*,PltObject);
    bool objGetMember(const char*,PltObject*);
    bool objSetMember(const char*,PltObject);
    //Native Function
    PltObject PObjFromNativeFun(const char*,NativeFunPtr);
    //Native Method
    PltObject PObjFromNativeMethod(const char*,NativeFunPtr,PltObject);
    //ErrObject
    PltObject Plt_Err(PltObject,const char*);
    //String
    PltObject allocStr(const char*);
    size_t strLength(PltObject);
    const char* strAsCstr(PltObject);
    const char* strResize(PltObject);
    //Conversions
    PltObject PObjFromInt(int32_t);
    PltObject PObjFromInt64(int64_t);
    PltObject PObjFromDouble(double);
    PltObject PObjFromByte(uint8_t);
    PltObject PObjFromBool(bool);
    void foo();
    #define AS_FILEOBJECT(x) *(FileObject*)x.ptr
    #define AS_PTR(x) x.ptr
    #define AS_BOOL(x) x.i
    #define AS_INT(x) x.i
    #define AS_INT64(x) x.l
    #define AS_DOUBLE(x) x.f
    #define AS_BYTE(x) x.i

    // Typedefs
    typedef bool(*fn11)(PltObject*,PltObject*,int,PltObject*);//callobject
    typedef void(*fn12)(void*);//markImportant
    typedef void(*fn13)(void*);//unmarkImpotant
    extern fn11 vm_callObject;
    extern fn12 vm_markImportant;
    extern fn13 vm_unmarkImportant;

    //
#ifdef __cplusplus
}
#endif
//
#endif