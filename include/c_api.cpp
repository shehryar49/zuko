#include "c_api.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

#define PltList vector<PltObject>
#define Dictionary std::unordered_map<PltObject,PltObject,HashFunction>

using namespace std;


struct HashFunction
    {
      size_t operator()(const PltObject& obj) const
      {
        size_t a = std::hash<char>()(obj.type);
        size_t b;
        size_t hashPltObject(const PltObject&);
        b = hashPltObject(obj) << 1;
        return a ^ b;
      }
  };
bool operator==(const PltObject& lhs,const PltObject& other)
    {
        if(other.type!=lhs.type)
            return false;
        if(lhs.type=='n')
          return true;
        else if(lhs.type==PLT_INT)
          return lhs.i==other.i;
        else if(lhs.type==PLT_INT64)
          return lhs.l==other.l;
        else if(lhs.type==PLT_FLOAT)
          return lhs.f==other.f;
        else if(lhs.type==PLT_BYTE)
          return lhs.i==other.i;
        else if(lhs.type==PLT_BOOL)
          return lhs.i==other.i;
        else if(lhs.type==PLT_FILESTREAM)
          return ((FileObject*)lhs.ptr)==((FileObject*)other.ptr);
        else if(lhs.type==PLT_STR)
          return *(string*)other.ptr==*(string*)lhs.ptr;
        else if(lhs.type==PLT_LIST)
            return *(PltList*)lhs.ptr==*(PltList*)other.ptr;
        else if(other.type=='y' || other.type=='r' || other.type == PLT_CLASS)
          return lhs.ptr==other.ptr;
        else if(lhs.type=='q' || lhs.type=='z')
          return lhs.ptr==other.ptr;
        else if(lhs.type=='a')
            return *(Dictionary*)lhs.ptr==*(Dictionary*)other.ptr;
        return false;
    }
size_t hashPltObject(const PltObject& a)
{
    char t = a.type;
    if(t==PLT_STR)
        return std::hash<std::string>()(*(string*)a.ptr);
    else if(t==PLT_INT)
        return std::hash<int>()(a.i);
    else if(t==PLT_INT64)
        return std::hash<long long int>()(a.l);
    else if(t==PLT_FLOAT)
        return std::hash<double>()(a.f);
    else if(t==PLT_BYTE)
        return std::hash<unsigned char>()(a.i);
    else if(t==PLT_BOOL)
        return std::hash<bool>()(a.i);
    //other types not supported as keys in dictionaries
    return 0;
}
struct Klass
{
  string name;
  std::unordered_map<string,PltObject> members;
  std::unordered_map<string,PltObject> privateMembers;
};
struct KlassObject
{
  Klass* klass;
  std::unordered_map<string,PltObject> members;
  std::unordered_map<string,PltObject> privateMembers;
};
struct Module
{
  std::string name;
  std::unordered_map<string,PltObject> members;
};
struct FunObject
{
  Klass* klass;//functions can be binded to classes as methods in which case they will have access to private members of that class
  //and also keep the class alive with them
  string name;
  size_t i;
  size_t args;
  PltList opt; //default/optional parameters
};


struct NativeFunction
{
  Klass* klass;//address of class the function is member of (if any NULL otherwise)
  NativeFunPtr addr;
  string name;//name of function (used when printing errors)
};
enum CoState
{
  SUSPENDED,
  RUNNING,
  STOPPED,
};
struct Coroutine
{
  int curr;//index in bytecode from where to resume the coroutine
  PltList locals;
  CoState state;
  string name;
  FunObject* fun;//function from which this coroutine object was made
  bool giveValOnResume;
};


//Helper and extension API Functions
inline PltObject PObjFromStrPtr(string* s)
{
  PltObject ret;
  ret.type = 's';
  ret.ptr = (void*)s;
  return ret;
}
inline PltObject PObjFromInt(int32_t x)
{
  PltObject ret;
  ret.type = 'i';
  ret.i = x;
  return ret;
}
inline PltObject PObjFromDouble(double f)
{
  PltObject ret;
  ret.type = 'f';
  ret.f = f;
  return ret;
}
inline PltObject PObjFromInt64(int64_t x)
{
  PltObject ret;
  ret.type = 'l';
  ret.l = x;
  return ret;
}
inline PltObject PObjFromPtr(void* p)
{
  PltObject ret;
  ret.type = 'p';
  ret.ptr = p;
  return ret;
}
inline PltObject PObjFromByte(uint8_t x)
{
  PltObject ret;
  ret.type = 'm';
  ret.i = x;
  return ret;
}
inline PltObject PObjFromBool(bool b)
{
  PltObject ret;
  ret.type = PLT_BOOL;
  ret.i = b;
  return ret;
}

inline PltObject PObjFromList(PltList* l)
{
  PltObject ret;
  ret.type = PLT_LIST;
  ret.ptr = (void*)l;
  return ret;
}
inline PltObject PObjFromDict(Dictionary* l)
{
  PltObject ret;
  ret.type = PLT_DICT;
  ret.ptr = (void*)l;
  return ret;
}
inline PltObject PObjFromModule(Module* k)
{
  PltObject ret;
  ret.type = PLT_MODULE;
  ret.ptr = (void*)k;
  return ret;
}
inline PltObject PObjFromKlass(Klass* k)
{
  PltObject ret;
  ret.type = PLT_CLASS;
  ret.ptr = (void*)k;
  return ret;
}
inline PltObject PObjFromKlassObj(KlassObject* ki)
{
  PltObject ret;
  ret.type = PLT_OBJ;
  ret.ptr = (void*)ki;
  return ret;
}
inline PltObject PObjFromByteArr(vector<uint8_t>* k)
{
  PltObject ret;
  ret.type = PLT_BYTEARR;
  ret.ptr = (void*)k;
  return ret;
}
inline PltObject PObjFromFile(FileObject* file)
{
  PltObject ret;
  ret.type = PLT_FILESTREAM;
  ret.ptr = (void*)file;
  return ret;
}


void foo()
{
    printf("hello world!\n");
}

typedef PltList*(*fn1)();//allocList
typedef Dictionary*(*fn2)();//allocDictionary
typedef string*(*fn3)();//allocString
typedef void*(*fn4)();//unused for now
typedef FileObject*(*fn5)();//allocFileObject
typedef Klass*(*fn6)();//allocKlass
typedef KlassObject*(*fn7)();//allocKlassObject
typedef NativeFunction*(*fn8)();//allocNativeFunObj
typedef Module*(*fn9)();//allocModule
typedef vector<uint8_t>*(*fn10)();//allocBytearray

//
fn1 vm_allocList;
fn2 vm_allocDict;
fn3 vm_allocString;
//fn4 vm_allocErrObject;
fn5 vm_allocFileObject;
fn6 vm_allocKlass;
fn7 vm_allocKlassObject;
fn8 vm_allocNativeFunObj;
fn9 vm_allocModule;
fn10 vm_allocByteArray;
fn11 vm_callObject;
fn12 vm_markImportant;
fn13 vm_unmarkImportant;
//The below helper functions make use of above function pointers
inline PltObject PObjFromStr(const string& s)
{
  string* ptr = vm_allocString();
  *ptr = s;
  PltObject ret;
  ret.type = 's';
  ret.ptr = (void*)ptr;
  return ret;
}

//
extern "C"
{
  PltObject Error;
  PltObject TypeError;
  PltObject ValueError;
  PltObject MathError; 
  PltObject NameError;
  PltObject IndexError;
  PltObject ArgumentError;
  PltObject FileIOError;
  PltObject KeyError;
  PltObject OverflowError;
  PltObject FileOpenError;
  PltObject FileSeekError; 
  PltObject ImportError;
  PltObject ThrowError;
  PltObject MaxRecursionError;
  PltObject AccessError;
  struct apiFuncions
  {
    fn1 a1;//api function 1
    fn2 a2;//and so on
    fn3 a3;
    fn4 a4;
    fn5 a5;
    fn6 a6;
    fn7 a7;
    fn8 a8;
    fn9 a9;
    fn10 a10;
    fn11 a11;
    fn12 a12;
    fn13 a13;
    Klass* k1;
    Klass* k2;
    Klass* k3;
    Klass* k4;
    Klass* k5;
    Klass* k6;
    Klass* k7;
    Klass* k8;
    Klass* k9;
    Klass* k10;
    Klass* k11;
    Klass* k12;
    Klass* k13;
    Klass* k14;
    Klass* k15;
    Klass* k16;

  };
  PltObject nil;
  #ifdef _WIN32
  #ifndef PLUTONIUM_INTERPRETER // make sure this header is included in a shared library
  //and not the plutonium interpreter
  __declspec(dllexport)
  #endif
  #endif
  void api_setup(apiFuncions* p)
  {
    nil.type = PLT_NIL;
    vm_allocList = p->a1;
    vm_allocDict = p->a2;
    vm_allocString = p->a3;
  //  vm_allocErrObject = p->a4;
    vm_allocFileObject = p->a5;
    vm_allocKlass = p->a6;
    vm_allocKlassObject = p->a7;
    vm_allocNativeFunObj = p->a8;
    vm_allocModule = p->a9;
    vm_allocByteArray = p->a10;
    vm_callObject = p->a11;
    vm_markImportant = p->a12;
    vm_unmarkImportant = p->a13;
    Error = PObjFromKlass(p->k1);
    TypeError = PObjFromKlass(p->k2);
    ValueError =PObjFromKlass( p->k3);
    MathError = PObjFromKlass(p->k4); 
    NameError = PObjFromKlass(p->k5);
    IndexError = PObjFromKlass(p->k6);
    ArgumentError = PObjFromKlass(p->k7);
    FileIOError = PObjFromKlass(p->k8);
    KeyError = PObjFromKlass(p->k9);
    OverflowError = PObjFromKlass(p->k10);
    FileOpenError = PObjFromKlass(p->k11);
    FileSeekError = PObjFromKlass(p->k12); 
    ImportError = PObjFromKlass(p->k13);
    ThrowError = PObjFromKlass(p->k14);
    MaxRecursionError = PObjFromKlass(p->k15);
    AccessError = PObjFromKlass(p->k16);
  }
}

//C INTERFACE Implementation
//for manipulating STL containers or non C-like structs used by plutonium

//The below functions won't do any typechecking
//make sure do before calling them
//For example.
//check that a PltObject refers to a dictionary before calling dictGet
PltObject allocDict()
{
  Dictionary* d = vm_allocDict();
  return PObjFromDict(d);
}
PltObject dictGet(PltObject map,PltObject key,bool* ret)
{
  Dictionary* d = (Dictionary*)map.ptr;
  auto it = d->find(key);
  if(it == d->end())
  {
    *ret = false;
    return nil;
  }
  return (*it).second;
}
bool dictSet(PltObject map,PltObject key,PltObject val)
{
  Dictionary* d = (Dictionary*)map.ptr;
  auto it = d->find(key);
  if(it == d->end())
  {
    return false;
  }
  (*it).second = val;
  return true;
}
size_t dictSize(PltObject map)
{
  Dictionary* d = (Dictionary*)map.ptr;
  return d->size();
}
void* newDictIter(PltObject map)
{
  Dictionary* d = (Dictionary*)map.ptr;
  if(d->begin() == d->end())
    return NULL;
  Dictionary::iterator* ptr = new Dictionary::iterator(d->begin());
  return (void*)ptr;
}
void freeDictIter(void* ptr)
{
    Dictionary::iterator* it = (Dictionary::iterator*)ptr;
    delete it;
}
void advanceDictIter(PltObject map,void** ptr)
{
    Dictionary* d = (Dictionary*)map.ptr;
    Dictionary::iterator* it = (Dictionary::iterator*)(*ptr);
    (*it)++;
    if((*it) == d->end())
      *ptr = NULL;

}
void setDictIterValue(void* ptr,PltObject val)
{
    Dictionary::iterator* it = (Dictionary::iterator*)ptr;
    Dictionary::iterator i = *it;
    
    (*(*it)).second = val;
    
}
PltObject getDictIterValue(void* ptr)
{
    Dictionary::iterator* it = (Dictionary::iterator*)ptr;
    return (**it).second;
}
PltObject getDictIterKey(void* ptr)
{
    Dictionary::iterator* it = (Dictionary::iterator*)ptr;
    return (**it).first;
}
///////////////////
PltObject allocBytearray()
{
  vector<uint8_t>* bt = vm_allocByteArray();
  return PObjFromByteArr(bt);
}
void btPush(PltObject obj,uint8_t val)
{
    vector<uint8_t>* bt = (vector<uint8_t>*)obj.ptr;
    bt->push_back(val);
}
bool btPop(PltObject obj,PltObject* ret)
{
    vector<uint8_t>* bt = (vector<uint8_t>*)obj.ptr;
    if(bt->size() > 0)
    {
      *ret = PObjFromByte(bt->back());
      bt->pop_back();
      return true;
    }
    *ret = nil;
    return false;
}
size_t btSize(PltObject obj)
{
    vector<uint8_t>* bt = (vector<uint8_t>*)obj.ptr;
    return bt->size();
}
void btResize(PltObject obj,size_t newsize)
{
    vector<uint8_t>* bt = (vector<uint8_t>*)obj.ptr;
    bt->resize(newsize);
}
uint8_t* btAsArray(PltObject obj)
{
  vector<uint8_t>* bt = (vector<uint8_t>*)obj.ptr;
  return &bt->at(0);
}
////////////
PltObject allocList()
{
  PltList* p = vm_allocList();
  return PObjFromList(p);
}
void listPush(PltObject obj,PltObject val)
{
    PltList* LIST = (PltList*)obj.ptr;
    LIST->push_back(val);
}
bool listPop(PltObject obj,PltObject* ret)
{
    PltList* LIST = (PltList*)obj.ptr;
    if(LIST->size() > 0)
    {
        *ret = LIST->back();
        LIST->pop_back();
        return true;
    }
    *ret = nil;
    return false;
}
size_t listSize(PltObject obj)
{
    PltList* LIST = (PltList*)obj.ptr;
    return LIST->size();
}
void listResize(PltObject obj,size_t newsize)
{
    PltList* LIST = (PltList*)obj.ptr;
    LIST->resize(newsize);
}
PltObject* listAsArray(PltObject obj)
{
    PltList* LIST = (PltList*)obj.ptr;
    return &LIST->at(0);
}
//////////////////////////
PltObject allocModule(const char* name)
{
    Module* m = vm_allocModule();
    m->name = name;
    return PObjFromModule(m);
}
void addModuleMember(PltObject m,const char* name,PltObject val)
{
  Module* mptr = (Module*)m.ptr;
  mptr->members.emplace(name,val);
}
///////////////////////////////////////
PltObject allocKlass(const char* name)
{
  Klass* k = vm_allocKlass();
  k->name = name;
  return PObjFromKlass(k);
}
void klassAddMember(PltObject obj,const char* name,PltObject val)
{
  Klass* k = (Klass*)obj.ptr;
  k->members.emplace(name,val);
}
void klassAddPrivateMember(PltObject obj,const char* name,PltObject val)
{
  Klass* k = (Klass*)obj.ptr;
  k->privateMembers.emplace(name,val);
}
//////////////////
PltObject allocObj(PltObject klass)
{
 Klass* k = (Klass*)klass.ptr;
 KlassObject* ko = vm_allocKlassObject();
 ko->klass = (Klass*)klass.ptr;
 ko->members = k->members;
 ko->privateMembers = k->privateMembers;
 return PObjFromKlassObj(ko);
}
void objAddMember(PltObject obj,const char* name,PltObject val)
{
  KlassObject* ko = (KlassObject*)obj.ptr;
  ko->members.emplace(name,val);
}
void objAddPrivateMember(PltObject obj,const char* name,PltObject val)
{
  KlassObject* ko = (KlassObject*)obj.ptr;
  ko->privateMembers.emplace(name,val);
}
bool objGetMember(PltObject obj,const char* name,PltObject* ret)
{
    KlassObject* ko = (KlassObject*)obj.ptr;
    auto it = ko->members.find(name);
    if(it == ko->members.end())
      return false; 
    *ret = (*it).second;
    return true;
}
bool objSetMember(PltObject obj,const char* name,PltObject val)
{
    KlassObject* ko = (KlassObject*)obj.ptr;
    auto it = ko->members.find(name);
    if(it == ko->members.end())
      return false; 
    (*it).second = val;
    return true;
}
///////////////
PltObject PObjFromNativeFun(const char* name,NativeFunPtr ptr)
{
  NativeFunction* nf = vm_allocNativeFunObj();
  nf->name = name;
  nf->klass = NULL;
  nf->addr = ptr;
  PltObject ret;
  ret.type = PLT_NATIVE_FUNC;
  ret.ptr = (void*)nf;
  return ret;
}
PltObject PObjFromNativeMethod(const char* name,NativeFunPtr ptr,PltObject klass)
{
  Klass* k = (Klass*)klass.ptr;
  NativeFunction* nf = vm_allocNativeFunObj();
  nf->name = name;
  nf->klass = k;
  nf->addr = ptr;
  PltObject ret;
  ret.type = PLT_NATIVE_FUNC;
  ret.ptr = (void*)nf;
  return ret;
}
///////////////
PltObject Plt_Err(PltObject klass,const char* msg)
{
  Klass* k = (Klass*)klass.ptr;
  KlassObject* ko = vm_allocKlassObject();
  ko->klass = k;
  ko->members = k->members;
  ko->privateMembers = k->privateMembers;
  ko->members["msg"] = PObjFromStr(msg);
  PltObject ret;
  ret.type = PLT_ERROBJ;
  ret.ptr = (void*)ko;
  return ret;
}
////////////////
PltObject allocStr(const char* val)
{
  string* p  = vm_allocString();
  *p = val;
  return PObjFromStrPtr(p);
}
size_t strLength(PltObject str)
{
  string* p = (string*)str.ptr;
  return p->length();
} 
const char* strAsCstr(PltObject str)
{
    string* p = (string*)str.ptr;
    return &p->at(0);
}
void strResize(PltObject str,size_t newsize)
{
    string* p = (string*)str.ptr;
    p->resize(newsize);
}