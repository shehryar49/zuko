#ifndef PLTOBJECT_H_
#define PLTOBJECT_H_
#include <string>
#include <vector>
#include <unordered_map>
using namespace std;

#define PltList vector<PltObject>
#define Dictionary std::unordered_map<PltObject,PltObject,PltObject::HashFunction>
struct PltObject;

//
//Types for different plutonium objects
#define PLT_LIST 'j'
#define PLT_DICT 'a'
#define PLT_INT 'i'
#define PLT_INT64 'l'
#define PLT_FLOAT 'f'
#define PLT_BYTE = 'm'
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
size_t hashList(void*);
size_t hashDict(void*);
//
enum ErrCode
{
  TYPE_ERROR = 1,
  VALUE_ERROR = 2,
  MATH_ERROR  = 3,
  NAME_ERROR = 4,
  INDEX_ERROR = 5,
  ARGUMENT_ERROR = 6,
  UNKNOWN_ERROR = 7,
  FILEIO_ERROR = 8,
  KEY_ERROR = 9,
  OVERFLOW_ERROR = 10,
  FILE_OPEN_ERROR = 11,
  FILE_SEEK_ERROR  = 12,
  IMPORT_ERROR = 13,
  THROW_ERROR = 14,
  MAX_RECURSION_ERROR=15,
  ACCESS_ERROR = 16
};

struct FileObject
{
  FILE* fp;
  bool open;
};
struct PltObject
{
    union
    {
        void* ptr;
        double f;
        long long int l;
        int i;
    };
    char type;
    size_t extra;
    PltObject()
    {
        type = 'n';
    }
    
    bool operator==(const PltObject& other)const
    {
        if(other.type!=type)
            return false;
        if(type=='n')
        {
          return true;
        }
        else if(type=='i')
        {
          return i==other.i;
        }
        else if(type=='l')
        {
          return l==other.l;
        }
        else if(type=='f')
        {
          return f==other.f;
        }
        else if(type=='b')
        {
          return i==other.i;
        }
        else if(type=='m')
        {
          return i==other.i;
        }
        else if(type=='u')
        {
          return ((FileObject*)ptr)==((FileObject*)other.ptr);
        }
        else if(type=='s')
        {
          return *(string*)other.ptr==*(string*)ptr;
        }
        else if(type=='j')
        {
            return *(PltList*)ptr==*(PltList*)other.ptr;
        }
        else if(other.type=='y' || other.type=='r')
        {
          return ptr==other.ptr;
        }
        else if(type=='q' || type=='z')
        {
          return ptr==other.ptr;;
        }
        else if(type=='a')
        {
            return *(Dictionary*)ptr==*(Dictionary*)other.ptr;
        }
        return false;
    }
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
};
size_t hashPltObject(const PltObject&);
size_t hashList(void* p)
{
  PltList& l = *(PltList*)p;
   size_t hash = l.size();
   for (auto& i : l)
      hash ^= hashPltObject(i) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    return hash;
}
size_t hashDict(void* p)
{
  Dictionary& d = *(Dictionary*)p;
   size_t hash = d.size();
    for (const auto& i : d)
      hash ^= hashPltObject(i.first)+hashPltObject(i.second) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    return hash;
}
size_t hashPltObject(const PltObject& a)
{
    char t = a.type;
    if(t=='s')
        return std::hash<std::string>()(*(string*)a.ptr);
    else if(t=='i')
        return std::hash<int>()(a.i);
    else if(t=='l')
        return std::hash<long long int>()(a.l);
    else if(t=='f')
        return std::hash<double>()(a.f);
    else if(t=='j')
        return hashList(a.ptr);
    else if(t=='a')
        return hashDict(a.ptr);
    else if(t=='u')
        return std::hash<FILE*>()(((FileObject*)a.ptr)->fp);
    else if(t=='m')
        return std::hash<unsigned char>()(a.i);
    else if(t=='b')
        return std::hash<bool>()(a.i);
    else if(t=='r' || t=='q' || t=='y')
      return std::hash<void*>()(a.ptr);
    return 0;
}
struct Klass
{
  string name;
  std::unordered_map<string,PltObject> members;
  std::unordered_map<string,PltObject> privateMembers;
};
struct KlassInstance
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
  string name;
  size_t i;
  size_t args;
  PltList opt; //default/optional parameters
};
struct ErrObject
{
  string des;
  int code;
};
typedef void(*NativeFunPtr)(PltObject*,int,PltObject*);
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

inline PltObject PltObjectFromStringPtr(string* s)
{
  PltObject ret;
  ret.type = 's';
  ret.ptr = (void*)s;
  return ret;
}

inline PltObject PltObjectFromInt(int x)
{
  PltObject ret;
  ret.type = 'i';
  ret.i = x;
  return ret;
}
inline PltObject PltObjectFromDouble(double f)
{
  PltObject ret;
  ret.type = 'f';
  ret.f = f;
  return ret;
}
inline PltObject PltObjectFromInt64(long long int x)
{
  PltObject ret;
  ret.type = 'l';
  ret.l = x;
  return ret;
}
inline PltObject PltObjectFromPointer(void* p)
{
  PltObject ret;
  ret.type = 'p';
  ret.ptr = p;
  return ret;
}
inline PltObject PltObjectFromByte(unsigned char x)
{
  PltObject ret;
  ret.type = 'm';
  ret.i = x;
  return ret;
}

inline PltObject PltObjectFromList(PltList* l)
{
  PltObject ret;
  ret.type = PLT_LIST;
  ret.ptr = (void*)l;
  return ret;
}
inline PltObject PltObjectFromDict(Dictionary* l)
{
  PltObject ret;
  ret.type = PLT_DICT;
  ret.ptr = (void*)l;
  return ret;
}
inline PltObject PltObjectFromModule(Module* k)
{
  PltObject ret;
  ret.type = PLT_MODULE;
  ret.ptr = (void*)k;
  return ret;
}
inline PltObject PltObjectFromKlass(Klass* k)
{
  PltObject ret;
  ret.type = PLT_CLASS;
  ret.ptr = (void*)k;
  return ret;
}
inline PltObject PltObjectFromKlassInstance(KlassInstance* ki)
{
  PltObject ret;
  ret.type = PLT_OBJ;
  ret.ptr = (void*)ki;
  return ret;
}

typedef PltList*(*alloc1)();
typedef Dictionary*(*alloc2)();
typedef string*(*alloc3)();
typedef ErrObject*(*alloc4)();
typedef FileObject*(*alloc5)();
typedef Klass*(*alloc6)();
typedef KlassInstance*(*alloc7)();
typedef NativeFunction*(*alloc8)();
typedef Module*(*alloc9)();
//
alloc1 vm_allocList;
alloc2 vm_allocDict;
alloc3 vm_allocString;
alloc4 vm_allocErrObject;
alloc5 vm_allocFileObject;
alloc6 vm_allocKlass;
alloc7 vm_allocKlassInstance;
alloc8 vm_allocNativeFunObj;
alloc9 vm_allocModule;
inline PltObject PltObjectFromString(string s)
{
  string* ptr = vm_allocString();
  *ptr = s;
  PltObject ret;
  ret.type = 's';
  ret.ptr = (void*)ptr;
  return ret;
}
PltObject Plt_Err(ErrCode e,string des)
{
  PltObject ret;
  ErrObject* p = vm_allocErrObject();
  p->des = des;
  p->code = (int)e;
  ret.type = 'e';
  ret.ptr = (void*) p;
  return ret;
}
inline PltObject PltObjectFromMethod(string name,NativeFunPtr r,Klass* k)
{
  PltObject ret;
  NativeFunction* fn = vm_allocNativeFunObj();
  fn->name = name;
  fn->klass = k;
  fn->addr = r;
  ret.type = 'y';
  ret.ptr = (void*)fn;
  return ret;
}
inline PltObject PltObjectFromFunction(string name,NativeFunPtr r,Klass* k=NULL)
{
  PltObject ret;
  NativeFunction* fn = vm_allocNativeFunObj();
  fn->name = name;
  fn->klass = k;
  fn->addr = r;
  ret.type = 'y';
  ret.ptr = (void*)fn;
  return ret;
}
//
struct allocFuncions
{
  alloc1 a1;
  alloc2 a2;
  alloc3 a3;
  alloc4 a4;
  alloc5 a5;
  alloc6 a6;
  alloc7 a7;
  alloc8 a8;
  alloc9 a9;
};
extern "C" void api_setup(allocFuncions* p)
{
  vm_allocList = p->a1;
  vm_allocDict = p->a2;
  vm_allocString = p->a3;
  vm_allocErrObject = p->a4;
  vm_allocFileObject = p->a5;
  vm_allocKlass = p->a6;
  vm_allocKlassInstance = p->a7;
  vm_allocNativeFunObj = p->a8;
  vm_allocModule = p->a9;
}
#endif
