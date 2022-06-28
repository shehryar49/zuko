#ifndef PLTOBJECT_H_
#define PLTOBJECT_H_
#include <string>
#include <vector>
#include <unordered_map>
using namespace std;

#define PltList vector<PltObject>
#define Dictionary std::unordered_map<PltObject,PltObject,PltObject::HashFunction>
struct PltObject;
typedef void(*Method)(PltObject*,PltObject*,int,PltObject*);
typedef void(*Function)(PltObject*,int,PltObject*);

//
//Types for different plutonium objects
#define PLT_LIST 'j'
#define PLT_DICT 'a'
#define PLT_INT 'i'
#define PLT_INT64 'l'
#define PLT_FLOAT 'f'
#define PLT_BYTE = 'm'
#define PLT_NATIVE_METHOD 'r'  //native c++ method(see prototype above)
#define PLT_NATIVE_FUNC 'y'//native c++ function
#define PLT_MODULE 'q'
#define PLT_NATIVE_OBJ 'c'
#define PLT_STR 's'
#define PLT_FILESTREAM 'u'
#define PLT_NIL 'n'
#define PLT_OBJ 'o' //objects created using plutonium code
#define PLT_CLASS 'v' //Plutonium code class
#define PLT_BOOL 'b'
#define PLT_POINTER 'p' //just a pointer that means something only to c++ code,the interpreter just stores it
#define PLT_FUNC 'w' //plutonium code function
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
        float f;
        long long int l;
        int i;
    };
    char type;
    char extra;
    string s;
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
          return other.s==s;
        }
        else if(type=='j')
        {
            return *(PltList*)ptr==*(PltList*)other.ptr;
        }
        else if(other.type=='y' || other.type=='r')
        {
          return ptr==other.ptr;
        }
        else if(type=='q')
        {
          return false;
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
  PltList l = *(PltList*)p;
   size_t hash = l.size();
   for (auto i : l)
      hash ^= hashPltObject(i) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    return hash;
}
size_t hashDict(void* p)
{
  Dictionary d = *(Dictionary*)p;
   size_t hash = d.size();
    for (auto i : d)
      hash ^= hashPltObject(i.first)+hashPltObject(i.second) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    return hash;
}
size_t hashPltObject(const PltObject& a)
{
    char t = a.type;
    if(t=='s')
    {
        return std::hash<std::string>()(a.s);
    }
    else if(t=='i')
    {
        return std::hash<int>()(a.i);
    }
    else if(t=='l')
    {
        return std::hash<long long int>()(a.l);
    }
    else if(t=='f')
    {
        return std::hash<float>()(a.f);
    }
    else if(t=='j')
    {
        return hashList(a.ptr);
    }
    else if(t=='a')
    {
        return hashDict(a.ptr);
    }
    else if(t=='u')
    {
        return std::hash<FILE*>()(((FileObject*)a.ptr)->fp);
    }
    else if(t=='m')
    {
        return std::hash<unsigned char>()(a.i);
    }
    else if(t=='b')
    {
        return std::hash<bool>()(a.i);
    }
    else if(t=='r' || t=='q' || t=='y')
    {
      return std::hash<void*>()(a.ptr);
    }
    return 0;
}
PltObject Plt_Err(ErrCode e,string des)
{
  PltObject ret;
  ret.type = 'e';
  ret.s = des;
  ret.i = e;
  return ret;
}
inline PltObject PltObjectFromString(string s)
{
  PltObject ret;
  ret.type = 's';
  ret.s = s;
  return ret;
}
inline PltObject PltObjectFromInt(int x)
{
  PltObject ret;
  ret.type = 'i';
  ret.i = x;
  return ret;
}
inline PltObject PltObjectFromFloat(float f)
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
inline PltObject PltObjectFromMethod(Method r)
{
  PltObject ret;
  ret.type = 'r';
  ret.ptr = (void*)r;
  return ret;
}
inline PltObject PltObjectFromFunction(Function r)
{
  PltObject ret;
  ret.type = 'y';
  ret.ptr = (void*)r;
  return ret;
}
#endif
