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


//This header contains some builtin functions which are embed into the interpreter

#ifndef BUILTIN_FUNC_H_
#define BUILTIN_FUNC_H_
#include "vm.h"
#include "zuko.h"
using namespace std;

std::unordered_map<string,BuiltinFunc> funcs;

int32_t f;
string fullform(char);
void PromoteType(ZObject&,char);

ZObject nil;
bool validateArgTypes(string name,string e,ZObject* args,int32_t argc,ZObject& x)
{
    if(e.length()!=(size_t)argc)
    {
      x = Z_Err(ArgumentError,"Error "+name+"() takes "+str((int64_t)e.length())+" arguments!");
      return false;
    }
    for(int32_t f=1;f<=argc;f++)
    {
        ZObject k = args[f-1];
        if(k.type!=e[f-1])
            {
           x = Z_Err(TypeError,"Error argument "+str(f)+" of "+name+"() should be of type "+fullform(e[f-1]));
            return false;
            }
        
    }
    return true;
}
ZObject Z_ISALPHA(ZObject* args,int32_t argc)
{
  if(argc!=1)
    return Z_Err(ArgumentError,"Error isalpha() takes one argument!");
  if(args[0].type!=Z_STR && args[0].type!=Z_MSTR)
    return Z_Err(TypeError,"Error isalpha() takes a string argument!");
  string s = *(string*)args[0].ptr;
  ZObject ret = nil;
  ret.type = 'b';
  ret.i = 0;
  for(auto e: s)
  {
    if(!isalpha(e))
      return ret;
  }
  ret.i = 1;
  return ret;
}

ZObject ASCII(ZObject* args,int32_t argc)
{
  if(argc!=1)
    return Z_Err(ArgumentError,"Error ascii() takes one argument!");
  if(args[0].type!='s' && args[0].type!=Z_MSTR)
    return Z_Err(TypeError,"Error ascii() takes a string argument!");
  string s = *(string*)args[0].ptr;
  if(s.length()!=1)
      return Z_Err(ValueError,"Error ascii() takes a string argument of length 1!");
 
  ZObject ret = nil;
  ret.type = 'i';
  ret.i = s[0];
  return ret;
}
ZObject TOCHAR(ZObject* args,int32_t argc)
{
  if(argc!=1)
    return Z_Err(ArgumentError,"Error char() takes one argument!");
  if(args[0].type!='i')
    return Z_Err(TypeError,"Error char() takes an integer argument!");
  char ch = (char)args[0].i;
  string s;
  s+=ch;
  ZObject ret = nil;
  string* p = allocString();
  *p = s;
  return ZObjFromStrPtr(p);
}
string unescape(string);
void printDictionary(Dictionary* l,vector<void*> seen = {})
{
  if(std::find(seen.begin(),seen.end(),(void*)l)!=seen.end())
  {
    printf("{...}");
    return;
  }
  seen.push_back((void*)l);
  Dictionary d = *l;
  ZObject val;
  size_t k = l->size();
  size_t i = 0;
  printf("{");
  for(auto e: d)
  {
    void printList(ZList*,vector<void*> = {});

    if(e.first.type=='j')
      printList((ZList*)e.first.ptr,seen);
    else if(e.first.type=='a')
      printDictionary((Dictionary*)e.first.ptr,seen);
    else if(e.first.type=='s')
    {
      printf("\"%s\"",unescape(*(string*)e.first.ptr).c_str());
    }
    else
      printf("%s",ZObjectToStr(e.first).c_str());
    printf(" : ");
    //
    if(e.second.type=='j')
      printList((ZList*)e.second.ptr,seen);
    else if(e.second.type=='a')
      printDictionary((Dictionary*)e.second.ptr,seen);
    else if(e.second.type=='s')
    {
      printf("\"%s\"",unescape(*(string*)e.second.ptr).c_str());
    }
    else
      printf("%s",ZObjectToStr(e.second).c_str());
    if(i!=k-1)
      printf(",");
    i+=1;
  }
  printf("}");
}
void printList(ZList* l,vector<void*> seen = {})
{
  if(std::find(seen.begin(),seen.end(),(void*)l)!=seen.end())
  {
    printf("[...]");
    return;
  }
  seen.push_back((void*)l);
  size_t k = l->size();
  ZObject val;
  printf("[");
  for(size_t i=0;i<k;i+=1)
  {
    val = l->at(i);
    if(val.type=='j')
      printList((ZList*)val.ptr,seen);
    else if(val.type=='a')
      printDictionary((Dictionary*)val.ptr,seen);
    else if(val.type=='s')
    {
      printf("\"%s\"",unescape(*(string*)val.ptr).c_str());
    }
    else
    {
      printf("%s",ZObjectToStr(val).c_str());
    }
    if(i!=k-1)
      printf(",");
  }
  printf("]");
}

void printByteArray(vector<uint8_t>* arr)
{
  string IntToHex(int);
  size_t len = arr->size();
  printf("bytearr([");
  for(size_t i=0;i<len;i++)
  {
    printf("%s",IntToHex((*arr)[i]).c_str());
    if(i!=len-1)
      printf(",");
  }
  printf("])");
}
ZObject print(ZObject* args,int32_t argc)
{
    int32_t k = 0;
    while(k<argc)
    {
        if(args[k].type=='j')
           printList((ZList*)args[k].ptr);
        else if(args[k].type=='a')
           printDictionary((Dictionary*)args[k].ptr);
        else if(args[k].type==Z_BYTEARR)
          printByteArray((vector<uint8_t>*)args[k].ptr);
        else if(args[k].type == Z_OBJ)
        {
          KlassObject* ki = (KlassObject*)args[k].ptr;
          ZObject r,ret;
          std::unordered_map<string,ZObject>::iterator it;
          if((it = ki->members.find("__print__"))!=ki->members.end())
          {
            r = (*it).second;
            vm_callObject(&r,&r,1,&ret);
          }
          else
            printf("%s",ZObjectToStr(args[k]).c_str());
        }
        else
          printf("%s",ZObjectToStr(args[k]).c_str());
        k+=1;
    }
    ZObject ret = nil;
    ret.type = 'n';
    return ret;
}
ZObject PRINTF(ZObject* args,int32_t argc)
{
    if(argc < 1)
      return Z_Err(ArgumentError,"At least one argument needed!");
    if(args[0].type!='s' && args[0].type!=Z_MSTR)
      return Z_Err(TypeError,"Argument 1 must be a string!");
    string& format = *(string*)args[0].ptr;
    int32_t k = 0;
    int32_t l = format.length();
    int32_t j = 1;
    while(k<l)
    {
        if(format[k] == '%')
        {
          if(k+1 < l && format[k+1] == '%')
          {
            printf("%%");
            k+=2;
            continue;
          }
          if(j>=argc)
            return Z_Err(ArgumentError,"String format requires more arguments!");
          if(args[j].type=='j')
           printList((ZList*)args[j].ptr);
          else if(args[j].type=='a')
           printDictionary((Dictionary*)args[j].ptr);
          else if(args[j].type==Z_BYTEARR)
          printByteArray((vector<uint8_t>*)args[j].ptr);
          else
           printf("%s",ZObjectToStr(args[j]).c_str());
          j+=1;
        }
        else
          printf("%c",format[k]);
        k+=1;
    }
    ZObject ret = nil;
    ret.type = 'n';
    return ret;
}
ZObject FORMAT(ZObject* args,int32_t argc)
{
    if(argc < 1)
      return Z_Err(ArgumentError,"At least one argument needed!");
    if(args[0].type!=Z_STR && args[0].type!=Z_MSTR)
      return Z_Err(TypeError,"Argument 1 must be a string!");
    const string& format = *(string*)args[0].ptr;
    int32_t k = 0;
    int32_t l = format.length();
    int32_t j = 1;
    string* p = allocString();
    while(k<l)
    {
        if(format[k] == '%')
        {
          if(k+1 < l && format[k+1] == '%')
          {
            *p+="%%";
            k+=2;
            continue;
          }
          if(j>=argc)
            return Z_Err(ArgumentError,"String format requires more arguments!");
          *p += ZObjectToStr(args[j]);
          j+=1;
        }
        else
          *p+=format[k];
        k+=1;
    }

    return ZObjFromStrPtr(p);
}

//void printList(ZList);

ZObject println(ZObject* args,int32_t argc)
{
    int32_t k = 0;
    while(k<argc)
    {
        if(args[k].type=='j')
          printList((ZList*)args[k].ptr);
        else if(args[k].type=='a')
          printDictionary((Dictionary*)args[k].ptr);
        else if(args[k].type==Z_BYTEARR)
          printByteArray((vector<uint8_t>*)args[k].ptr);
        else if(args[k].type == Z_OBJ)
        {
          KlassObject* ki = (KlassObject*)args[k].ptr;
          ZObject r,ret;
          std::unordered_map<string,ZObject>::iterator it;
          if((it = ki->members.find("__print__"))!=ki->members.end())
          {
            r = (*it).second;
            vm_callObject(&r,&r,1,&ret);
          }
          else
            printf("%s",ZObjectToStr(args[k]).c_str());
        }
        else
          printf("%s",ZObjectToStr(args[k]).c_str());
        k+=1;
    }
    puts("");
    ZObject ret = nil;
    return ret;
}

ZObject input(ZObject* args,int32_t argc)
{
    if(argc!=0 && argc!=1)
    {
        return Z_Err(ArgumentError,"Error input() takes 1 or 0 arguments!");
    }
    if(argc==1)
    {
      if(args[0].type!='s' && args[0].type!=Z_MSTR)
        return Z_Err(TypeError,"Error input() takes a string argument!");
      string& prompt = *(string*)args[0].ptr;
      printf("%s",prompt.c_str());
    }
    ZObject ret = nil;
    string s;
    char ch;
    while(true)
    {
        ch = fgetc(stdin);
        if(ch=='\n')
        {
            break;
        }

          s+=ch;
    }
    string* p = allocString();
    *p = s;
    return ZObjFromStrPtr(p);
}
ZObject TYPEOF(ZObject* args,int32_t argc)
{
  if(argc!=1)
  {
      return Z_Err(TypeError,"Error typeof() takes one argument only!");
  }
  string fullform(char);
  string* p = allocString();
  *p = fullform(args[0].type);
  return ZObjFromStrPtr(p);
}
ZObject isInstanceOf(ZObject* args,int32_t argc)
{
  if(argc!=2)
    return Z_Err(ArgumentError,"Error function isInstanceOf() takes 2 arguments!");
  if(args[1].type!='v')
    return Z_Err(TypeError,"Error second argument to  isInstanceOf() should be a class!");
  ZObject ret = nil;
  ret.type = 'b';
  if(args[0].type!='o')
  {
    ret.i = 0;
    return ret;
  }
  KlassObject* obj = (KlassObject*)args[0].ptr;
  Klass* k = (Klass*)args[1].ptr;
  ret.i = obj->klass == k;
  return ret;
}
ZObject LEN(ZObject* args,int32_t argc)
{
    if(argc!=1)
        return Z_Err(ArgumentError,"Error len() takes one argument!");

    ZObject ret = nil;
    ret.type = Z_INT64;
    if(args[0].type=='s' || args[0].type == Z_MSTR)
    {
      string& s = *(string*)args[0].ptr;
      ret.l = s.length();
    }
    else if(args[0].type=='j')
        ret.l = ((ZList*)args[0].ptr)->size();
    else if(args[0].type=='a')
        ret.l = ((Dictionary*)args[0].ptr)->size();
    else if(args[0].type == 'c')
        ret.l = ((vector<uint8_t>*)args[0].ptr)->size();
    else
        return Z_Err(TypeError,"Error len() unsupported for type "+fullform(args[0].type));
    return ret;
}
//////////
ZObject OPEN(ZObject* args,int32_t argc)
{
    string patt = "ss";
    ZObject ret = nil;
    if(!validateArgTypes("open",patt,args,argc,ret))
      return ret;
    string& filename = *(string*)args[0].ptr;
    string& mode = *(string*)args[1].ptr;
    if(mode!="r" && mode!="w" && mode!="a" && mode!="rw" && mode!="rw+" && mode!="rb" && mode!="wb")
        return Z_Err(ValueError,"Error unknown mode: \""+mode+"\"");
    FILE* fp = fopen(filename.c_str(), mode.c_str());
    if(!fp)
    {

        return Z_Err(FileOpenError,strerror(errno));
    }
    FileObject* f = allocFileObject();
    f->fp = fp;
    f->open = true;
    ret.type = 'u';
    ret.ptr = (void*)f;
    return ret;
}
ZObject READ(ZObject* args,int32_t argc)
{
    if(argc!=2 && argc!=1)
        return Z_Err(ArgumentError,"Error read() function takes one or two arguments!");
    if(args[0].type!='u')
        return Z_Err(TypeError,"Error read() needs a file stream to read from!");
    char delim = EOF;
    if(argc==2)
    {
      if(args[1].type!='s' && args[1].type!=Z_MSTR)
        return Z_Err(TypeError,"Error argument 2 to read() function should be of type string!");
      string& l = *(string*)args[1].ptr;
      if(l.length()!=1)
        return Z_Err(ValueError,"Error optional delimeter argument to read() should be string of length 1");
      delim = l[0];  
    }
    FileObject fobj = *(FileObject*)args[0].ptr;
    if(!fobj.open)
      return Z_Err(ValueError,"Error the file stream is closed!");
    FILE* fp = fobj.fp;
    if(!fp)
        return Z_Err(FileIOError,"Error can't read from a closed file stream!");
    char ch;
    string* p = allocString();
    while((ch = fgetc(fp))!=EOF)
    {
        if(ch==delim)
          break;
        (*p)+=ch;
    }
    return ZObjFromStrPtr(p);
}
ZObject CLOSE(ZObject* args,int32_t argc)
{
    if(argc!=1)
        return Z_Err(ArgumentError,"Error close() takes 1 argument!");
  //  printf("args[0].type = %c\n",args[0].type);
    if(args[0].type!='u')
        return Z_Err(TypeError,"Error close() takes a file stream as an argument!");
    FileObject* fobj = (FileObject*)args[0].ptr;
    if(!fobj->open)
    {
      return Z_Err(ValueError,"Error file already closed!");
    }
    FILE* f = fobj->fp;
    if(f==stdin || f==stdout)
      return Z_Err(ValueError,"Are you nuts?You should not close stdin or stdout!");
    fclose(f);
    ZObject ret = nil;
    ret.type='n';
    fobj->open = false;
    return ret;
}
ZObject RAND(ZObject* args,int32_t argc)
{
    if(argc!=0)
        return Z_Err(ArgumentError,"Error rand() takes 0 arguments!");
    ZObject ret = nil;
    ret.i = rand();
    ret.type= 'i';
    return ret;
}
//////////////
ZObject BYTEARRAY(ZObject* args,int32_t argc)
{
  ZObject ret = nil;
  if(argc==0)
  {
    vector<uint8_t>* arr = allocByteArray();
    ret.type = Z_BYTEARR;
    ret.ptr = (void*)arr;
    return ret;
  }
  else if(argc == 1)
  {
    if(args[0].type != Z_LIST)
    {
      return Z_Err(TypeError,"Error bytearray() takes a list argument!");
    }
    ZList* p = (ZList*)args[0].ptr;
    size_t len = p->size();
    vector<uint8_t>* arr = allocByteArray();
    ret.type = Z_BYTEARR;
    ret.ptr = (void*)arr;
    for(size_t i=0;i<len;i++)
    {
      if((*p)[i].type!=Z_BYTE)
        return Z_Err(TypeError,"Error argument list given to bytearray() must contain only bytes!");
      arr->push_back((*p)[i].i);
    }
    return ret;
  }
  return Z_Err(ArgumentError,"Error bytearray() takes 0 or 1 arguments!");
}

ZObject WRITE(ZObject* args,int32_t argc)
{
  //printf("writing %s\n",args[0].s.c_str());
  if(argc!=2)
    return Z_Err(ArgumentError,"Error write() takes two arguments!");
  string patt = "su";
  ZObject ret = nil;
  if(!validateArgTypes("write",patt,args,argc,ret))
    return ret;
  string& data = *(string*)args[0].ptr;
  FileObject* p = (FileObject*)args[1].ptr;
  FILE* fp = p->fp;
  if(fputs(data.c_str(),fp)==EOF)
    return Z_Err(FileIOError,"Error while writing to file: "+(std::string)strerror(errno));
  ret.type = 'n';
  //printf("done\n");
  return ret;
}
ZObject EXIT(ZObject* args,int32_t argc)
{
    int ret =  0;
    if(argc == 1)
    {
      if(args[0].type!=Z_INT)
        return Z_Err(TypeError,"Integer argument required!");
      ret = args[0].i;
    }
    else if(argc == 0);
    else
        return Z_Err(ArgumentError,"Error exit() takes either 0 or 1 argument!");
    exit(ret);
}
////////////////
ZObject REVERSE(ZObject* args,int32_t argc)
{
    if(argc!=1)
        return Z_Err(ArgumentError,"Error reverse() takes 1 argument!");
    const ZObject& q  = args[0];
    if(q.type!='s' && q.type!='j')
        return Z_Err(TypeError,"Error reverse() takes a string or list argument!");
    if(q.type=='s')
    {
        string* l = allocString();
        string& data = *(string*)q.ptr;
        for(int32_t k=data.length()-1;k>=0;k--)
        {
            *l+=data[k];
        }
        return ZObjFromStrPtr(l);
    }
    ZList l;
    ZList currList = *(ZList*)q.ptr;
    for(int32_t k =currList.size()-1;k>=0;k--)
    {
        l.push_back(currList[k]);
    }
    ZObject ret = nil;// = l;
    ZList* p = allocList();
    *p = l;
    ret.type = 'j';
    ret.ptr = (void*)p;
    return ret;
}///////
/////////
ZObject BYTES(ZObject* args,int32_t argc)
{
    if(argc!=1)
      return Z_Err(ArgumentError,"Error bytes() takes one argument!");
    auto p = allocByteArray();
    const ZObject& e = args[0];
    ZObject ret = nil;
    ret.type = Z_BYTEARR;
    ret.ptr = (void*)p;
    if(e.type=='i')
    {
      p->resize(4);
      memcpy(&(*p)[0],&e.i,4);
      return ret;
    }
    else if(e.type=='l')
    {
      p->resize(8);
      memcpy(&(*p)[0],&e.l,8);
      return ret;
    }
    else if(e.type=='f')
    {
      p->resize(4);
      memcpy(&(*p)[0],&e.f,4);
      return ret;
    }
    else if(e.type=='s' || e.type == Z_MSTR)
    {
       string& s = *(string*)e.ptr;
       for(auto ch: s)
         p->push_back((uint8_t)ch);
       return ret;
    }
    else if(e.type=='b')
    {
      if(e.i)
        p->push_back(1);
      else
        p->push_back(0);
      return ret;
    }
    else if(e.type == Z_BYTE)
    {
      p->push_back(e.i);
      return ret;
    }
    else
    {
        return Z_Err(TypeError,"Error cannot convert "+fullform(e.type)+" type to bytes!");
    }
}
///////////////
ZObject OBJINFO(ZObject* args,int32_t argc)
{
    if(argc!=1)
      return Z_Err(ArgumentError,"Error obj_info() takes 1 argument!");
    if( args[0].type=='o')
    {
      KlassObject* k = (KlassObject*)args[0].ptr;
      for(auto e: k->members)
      {
            printf("%s: %s\n",e.first.c_str(),fullform(e.second.type).c_str());
          
      }
      for(auto e: k->privateMembers)
      {
          printf("%s: %s\n",e.first.c_str(),fullform(e.second.type).c_str());
      }
      ZObject ret = nil;
      return ret;
    }
    else
    {
        return Z_Err(TypeError,"Error argument is not an object of any class!");
    }
}
ZObject moduleInfo(ZObject* args,int32_t argc)
{
  if(argc!=1)
    return Z_Err(ArgumentError,"Error moduleInfo takes 1 argument!");
  if(args[0].type!=Z_MODULE)
    return Z_Err(TypeError,"Argument must be a module object!");
  Module* mod = (Module*)args[0].ptr;
  printf("Module %s\n",mod->name.c_str());
  printf("------------\n");
  for(auto e: mod->members)
  {
    printf("%s  %s\n",e.first.c_str(),fullform(e.second.type).c_str());

  }
  ZObject ret = nil;
  return ret;
}
union FOO
{
  int32_t x;
  unsigned char bytes[sizeof(x)];
}FOO;
union FOO1
{
  int64_t l;
  unsigned char bytes[sizeof(l)];
}FOO1;
union FOO2
{
  double f;
  unsigned char bytes[sizeof(f)];
}FOO2;
ZObject makeList(ZObject* args,int32_t argc)
{
        ZObject ret = nil;
        if(!validateArgTypes("makeList","js",args,argc,ret))
          return ret;
        string& pattern = *(string*)args[1].ptr;
        size_t k = 0;
        ZList res;
        size_t i = 0;
        
        
        ZList currList = *(ZList*)args[0].ptr;
        while(k<currList.size())
        {
            if(i>pattern.length()-1)
            {
               return Z_Err(ValueError,"Error no pattern specified for the remaining bytes!");
            }
            ZObject l = currList[k];
            if(l.type!='m')
            {
                return Z_Err(ValueError,"Error list should only contain bytes!");
            }
            int32_t b = l.i;
            if(pattern[i]=='i')
            {
               FOO.bytes[0] = b;
               if(k+3 >=currList.size())
               {
                   return Z_Err(ValueError,"Error the list is missing some bytes!");
               }
               k+=1;
               l = currList[k];
               FOO.bytes[1] = l.i;
               k+=1;
               l = currList[k];
               FOO.bytes[1+1] = l.i;
               k+=1;
               l = currList[k];
               FOO.bytes[3] = l.i;
               ZObject e;
               e.type = 'i';
               e.i =(int32_t) FOO.x;
               res.push_back(e);
            }
            else if(pattern[i]=='b')
            {

               ZObject e;
               e.type = 'b';
               e.i = b;
               res.push_back(e);
            }
            else if(pattern[i]=='l')
            {
               FOO1.bytes[0] = b;
               if(k+7 >=currList.size())
               {
                   return Z_Err(ValueError,"Error the list is missing some bytes!");
               }
               k+=1;
               l = currList[k];
               FOO1.bytes[1] = l.i;
               k+=1;
               l = currList[k];
               FOO1.bytes[1+1] = l.i;
               k+=1;
               l = currList[k];
               FOO1.bytes[3] = l.i;
               k+=1;
               l = currList[k];
               FOO1.bytes[4] = l.i;
               k+=1;
               l = currList[k];
               FOO1.bytes[5] = l.i;
               k+=1;
               l = currList[k];
               FOO1.bytes[6] = l.i;
               k+=1;
               l = currList[k];
               FOO1.bytes[7] = l.i;
               ZObject e;
               e.type = 'l';
               e.l = FOO1.l;
               res.push_back(e);
            }
            else if(pattern[i]=='s')
            {
                size_t j = k;
                ZObject e;
                string* f = allocString();
                bool terminated = false;
                while(true)
                {
                    if(j>=currList.size())
                    {
                        return Z_Err(ValueError,"Ran out of bytes!");
                    }
                    e = currList[j];
                    if(e.type!='m')
                        return Z_Err(ValueError,"Error the list should only contain bytes!");
                    if(e.i==0)
                    {
                        terminated = true;
                        break;
                    }
                    *f+=(char)e.i;
                    j+=1;
                }
                if(!terminated)
                {
                    return Z_Err(ValueError,"Error the bytes are invalid to be converted to string!");
                }
                res.push_back(ZObjFromStrPtr(f));
                k  = j;
            }
            else if(pattern[i]=='f')
            {
               FOO2.bytes[0] = b;
               if(k+3 >=currList.size())
               {
                   return Z_Err(ValueError,"Error the list is missing some bytes!");
               }
               k+=1;
               l = currList[k];
               FOO2.bytes[1] = l.i;
               k+=1;
               l = currList[k];
               FOO2.bytes[1+1] = l.i;
               k+=1;
               l = currList[k];
               FOO2.bytes[3] = l.i;
               ZObject e;
               e.type = 'f';
               e.f = FOO2.f;
               res.push_back(e);
            }
            else
            {
                return Z_Err(TypeError,"Error unknown type used in pattern!");
            }
            i+=1;
            k+=1;
        }
        if(i!=pattern.length())
        {
            return Z_Err(ValueError,"Error the list does not have enough bytes to follow the pattern!");
        }

        ZList* p = allocList();
        *p =  (res);
        ret.ptr = (void*)p;
        ret.type = 'j';

        return ret;
}
///////
/////////

ZObject SUBSTR(ZObject* args,int32_t argc)
{
    if(argc==3)
		{
			if(args[0].type!='i' && args[0].type!='l')
        return Z_Err(TypeError,"Error first argument of substr() should be an integer");
      if(args[1].type!='i' && args[1].type!='l')
        return Z_Err(TypeError,"Error second argument of substr() should be an integer");
			if(args[2].type=='s' || args[2].type==Z_MSTR)
			{
        string* q = allocString();
        PromoteType(args[0],'l');
        PromoteType(args[1],'l');
        
        if(args[0].l<0 || args[1].l<0 )
        {
           return ZObjFromStrPtr(q);
        }
        string& data = *(string*)args[2].ptr;
        *q  = substr((int32_t)args[0].l,(int32_t)args[1].l,data);
        return ZObjFromStrPtr(q);
			}
			else
      {
      return Z_Err(TypeError,"Error third argument of substr() should be a string!\n");
      }
		}
		return Z_Err(ArgumentError,"Error substr() takes three arguments!");
}
ZObject getFileSize(ZObject* args,int32_t argc)
{
  if(argc==1)
  {
    if(args[0].type!='u')
        return Z_Err(TypeError,"Error getFileSize() takes an open file as argument!");
  
    FileObject* p = (FileObject*)args[0].ptr;
    if(!p->open)
      return Z_Err(FileIOError,"Unable to get size of closed file!");
    FILE* currF = p->fp;
    fseek(currF,0,SEEK_END);
    int64_t n = ftell(currF);
    rewind(currF);
    return ZObjFromInt64(n);
  }
  return Z_Err(ArgumentError,"Error getFileSize() takes 1 argument!");
}
ZObject FTELL(ZObject* args,int32_t argc)
{
  if(argc==1)
  {
    if(args[0].type!='u')
        return Z_Err(TypeError,"Error ftell() takes an open file as argument!");
    FileObject* p = (FileObject*)args[0].ptr;
    if(!p->open)
      return Z_Err(FileIOError,"Error file is closed!");
    FILE* currF = p->fp;
    int64_t n = ftell(currF);
    ZObject ret = nil;
    ret.type = 'l';
    ret.l = (int64_t)n;
    return ret;
  }
  return Z_Err(ArgumentError,"Error ftell() takes 1 argument!");
}
ZObject REWIND(ZObject* args,int32_t argc)
{
  if(argc==1)
  {
    if(args[0].type!='u')
        return Z_Err(TypeError,"Error rewind() takes an open file as argument!");
  
    FileObject* p = (FileObject*)args[0].ptr;
    if(!p->open)
      return Z_Err(FileIOError,"Unable to rewind closed file!");
    FILE* currF = p->fp;
    rewind(currF);
    ZObject ret = nil;
    return ret;
  }
  return Z_Err(ArgumentError,"Error rewind() takes 1 argument!");
}
ZObject SYSTEM(ZObject* args,int32_t argc)
{
  if(argc!=1)
      return Z_Err(ArgumentError,"Error system() takes 1 argument!");
  if(args[0].type!='s' && args[0].type!=Z_MSTR)
      return Z_Err(TypeError,"Error system() takes a string argument!");
  string& command = *(string*)args[0].ptr;
  int32_t i = system(command.c_str());
  return ZObjFromInt(i);
}
ZObject SPLIT(ZObject* args,int32_t argc)
{
    if(argc==2)
		{
			if( (args[0].type=='s' || args[0].type==Z_MSTR) && (args[1].type == Z_MSTR || args[1].type=='s'))
			{
        string& data = *(string*)args[0].ptr;
        string& delim = *(string*)args[1].ptr;
				vector<string> list = split(data,delim);
				uint32_t  o = 0;
				ZList l;
				string* value;
				while(o<list.size())
        {
          value = allocString();
          *value = list[o];
          l.push_back(ZObjFromStrPtr(value));
          o+=1;
        }
        ZList* p = vm_allocList();
        *p = l;
        ZObject ret = nil;
        ret.type = 'j';
        ret.ptr = p;
        return ret;
			}
			else
      {
        return Z_Err(TypeError,"Error split() takes both string arguments!\n");
        exit(0);
      }
		}
		return Z_Err(ArgumentError,"Error split() takes two arguments!");
}
ZObject GETENV(ZObject* args,int32_t argc)
{
  if(argc==1)
  {

      if(args[0].type=='s' || args[0].type==Z_MSTR)
      {
        string& vname = *(string*)args[0].ptr;
        char* c = getenv(vname.c_str());
        if(!c)
        {
          //   return Z_Err(NameError,"Unknown environment variable!\n");
          ZObject ret = nil;
          return ret;
        }
        string* s = allocString();
        *s = c;

        return ZObjFromStrPtr(s);
      }
      else
      {
        return Z_Err(TypeError,"Error getenv() takes a string argument!");
      }
  }
  return Z_Err(ArgumentError,"Error getenv() takes one argument!");
}
ZObject SHUFFLE(ZObject* args,int32_t argc)
{
    if(argc==1)
		{
			if(args[0].type=='j')
			{
			  ZList* a = (ZList*)args[0].ptr;
				std::random_shuffle(a->begin(),a->end());
				ZObject ret = nil;
				return ret;
			}
			else
      {
        return Z_Err(TypeError,"Error shuffle takes a list as an argument!");
      }
		}
		return Z_Err(ArgumentError,"Error shuffle() takes exactly one argument!");
}
ZObject STR(ZObject* args,int32_t argc)
{
    if(argc==1)
		{
			if(args[0].type=='i')
			{
        string* s = allocString();
        *s = str(args[0].i);
        return ZObjFromStrPtr(s);
			}
			else if(args[0].type=='f')
			{
        string* s = allocString();
        *s = str(args[0].f);
        return ZObjFromStrPtr(s);
			}
      else if(args[0].type=='l')
      {
          string* s = allocString();
          *s = str(args[0].l);
          return ZObjFromStrPtr(s);
      }
      else if(args[0].type == Z_BYTE)
      {
        string* s = allocString();
        *s = ZObjectToStr(args[0]);
        return ZObjFromStrPtr(s);
      }
      else if(args[0].type == 'b')
      {
          string* s = allocString();
          *s = (args[0].i) ? "true" : "false";
          return ZObjFromStrPtr(s);
      }
      else if(args[0].type == 'j')
      {
          string* s = allocString();
          *s = ZObjectToStr(args[0]);
          return ZObjFromStrPtr(s);
      }
      else if(args[0].type == 'a')
      {
          string* s = allocString();
          *s = ZObjectToStr(args[0]);
          return ZObjFromStrPtr(s);
      }   
      else if(args[0].type == Z_BYTEARR)
      {
        string* s = allocString();
        vector<uint8_t>& bytes = *(vector<uint8_t>*)args[0].ptr;
        for(auto byte: bytes)
        {
          s->push_back((char)byte);
        }
        return ZObjFromStrPtr(s);
      }
      return Z_Err(TypeError,"Error str() unsupported for type "+fullform(args[0].type));
		}
    return Z_Err(ArgumentError,"Error str() takes only one argument!");
}
ZObject FIND(ZObject* args,int32_t argc)
{
  ZObject ret = nil;
  if(argc != 2)
    return Z_Err(ArgumentError,"Error find() takes 2 arguments!");
  if(args[0].type != Z_STR && args[0].type!=Z_MSTR)
    return Z_Err(TypeError,"Error first argument given to find() must be a stirng!");
  if(args[1].type != Z_STR && args[1].type!=Z_MSTR)
    return Z_Err(TypeError,"Error second argument given to find() must be a stirng!");
  
  ret.type = 'l';
  string& a = *(string*)args[0].ptr;
  string& b = *(string*)args[1].ptr;
  auto y = b.find(a); 
  if(y==std::string::npos)
  {
    ret.type = 'n';
    return ret;
  }
  else
    ret.l = static_cast<int64_t>(y);
  return ret;
}
ZObject TOINT(ZObject* args,int32_t argc)
{
    if(argc==1)
		{
			if(args[0].type=='s' || args[0].type == Z_MSTR)
			{
			    string& q = *(string*)args[0].ptr;
			    ZObject ret = nil;
          if(isnum(q))
          {
              ret.i = Int(q);
              ret.type = 'i';
          }
          else if(isInt64(q))
          {
              ret.l = toInt64(q);
              ret.type = 'l';
          }
          else if(isaFloat(q))
          {
              q = q.substr(0,q.find('.'));
              ret.type = Z_INT;
              if(isnum(q))
                ret.i = Int(q);
              else if(isInt64(q))
              {
                ret.type = Z_INT64;
                ret.l = toInt64(q);
              }
              else
              {
                ret.type = Z_INT64;
                ret.l =  LLONG_MAX;
              }
              return ret;
          }
          else
              return Z_Err(ValueError,"Error the string "+q+" cannot be converted to an integer!");
			    return ret;
      }
			else if(args[0].type=='f')
			{
               ZObject ret = nil;
               double d = args[0].f;
               if(d<(double)LLONG_MIN)
                 ret.l = LLONG_MIN;
               else if(d>(double)LLONG_MAX)
                 ret.l = LLONG_MAX;
               else 
                 ret.l = static_cast<int64_t>(args[0].f);
               ret.type = 'l';
               return ret;
			}
      else if(args[0].type == 'm' || args[0].type == Z_BOOL)
      {
        args[0].type = Z_INT;
        return args[0];
      }
      else if(args[0].type == Z_INT || args[0].type == Z_INT64)
        return args[0];
      return Z_Err(TypeError,"Error int() unsupported for type "+fullform(args[0].type));
		}
		return Z_Err(ArgumentError,"Error int() takes exactly one argument!");
		exit(0);
}
ZObject TOINT32(ZObject* args,int32_t argc)
{
    if(argc==1)
		{
			if(args[0].type=='s' || args[0].type == Z_MSTR)
			{
			    string& q = *(string*)args[0].ptr;
			    ZObject ret = nil;
          if(isnum(q))
          {
              ret.i = Int(q);
              ret.type = 'i';
          }
          else if(isInt64(q))
          {
              ret.i = INT_MAX;
              ret.type = Z_INT;
          }
          else if(isaFloat(q))
          {
              q = q.substr(0,q.find('.'));
              ret.type = Z_INT;
              if(isnum(q))
                ret.i = Int(q);
              else if(isInt64(q))
              {
                ret.type = Z_INT;
                ret.i = INT_MAX;
              }
              else
              {
                ret.type = Z_INT;
                ret.i =  INT_MAX;
              }
              return ret;
          }
          else
              return Z_Err(ValueError,"Error the string "+q+" cannot be converted to an integer!");
			    return ret;
      }
			else if(args[0].type=='f')
			{
        ZObject ret = nil;
        double d = args[0].f;
        if(d<(double)INT_MIN)
          ret.i = INT_MIN;
        else if(d>(double)INT_MAX)
          ret.l = INT_MAX;
        else 
          ret.l = static_cast<int32_t>(args[0].f);
        ret.type = 'i';
        return ret;
			}
      else if(args[0].type == 'm' || args[0].type == Z_BOOL)
      {
        args[0].type = Z_INT;
        return args[0];
      }
      else if(args[0].type == Z_INT)
        return args[0];
      else if(args[0].type == Z_INT64)
      {
        if(args[0].l > INT_MAX)
          return ZObjFromInt(INT_MAX);
        else if(args[0].l < INT_MIN)
          return ZObjFromInt(INT_MIN);
        return ZObjFromInt(static_cast<int32_t>(args[0].l)); 
      }
      return Z_Err(TypeError,"Error int32() unsupported for type "+fullform(args[0].type));
		}
		return Z_Err(ArgumentError,"Error int32() takes exactly one argument!");
		exit(0);
}
ZObject TOINT64(ZObject* args,int32_t argc)
{
    if(argc==1)
		{
			if(args[0].type=='s' || args[0].type==Z_MSTR)
			{
			    string& q = *(string*)args[0].ptr;
			    ZObject ret = nil;
          if(isnum(q))
          {
              ret.l = Int(q);
              ret.type = Z_INT64;
          }
          else if(isInt64(q))
          {
              ret.l = toInt64(q);
              ret.type = 'l';
          }
          else if(isaFloat(q))
          {
              q = q.substr(0,q.find('.'));
              ret.type = Z_INT;
              if(isInt64(q))
              {
                ret.type = Z_INT64;
                ret.l = toInt64(q);
              }
              else
              {
                ret.type = Z_INT64;
                ret.l =  LLONG_MAX;
              }
              return ret;
          }
          else
              return Z_Err(ValueError,"Error the string "+q+" cannot be converted to an integer!");
			    return ret;
      }
			else if(args[0].type=='f')
			{
               ZObject ret = nil;
               double d = args[0].f;
               if(d<(double)LLONG_MIN)
                 ret.l = LLONG_MIN;
               else if(d>(double)LLONG_MAX)
                 ret.l = LLONG_MAX;
               else 
                 ret.l = static_cast<int64_t>(args[0].f);
               ret.type = 'l';
               return ret;
			}
      else if(args[0].type == 'm' || args[0].type == Z_BOOL || args[0].type == Z_INT)
      {
        args[0].type = Z_INT64;
        args[0].l = args[0].i;
        return args[0];
      }
      else if(args[0].type == Z_INT64)
        return args[0];
      return Z_Err(TypeError,"Error int64() unsupported for type "+fullform(args[0].type));
		}
		return Z_Err(ArgumentError,"Error int64() takes exactly one argument!");
		exit(0);
}
ZObject TOFLOAT(ZObject* args,int32_t argc)
{
    if(argc==1)
		{
			if(args[0].type=='s' || args[0].type == Z_MSTR)
			{
        string& q = *(string*)args[0].ptr;
        if(isnum(q))
          return ZObjFromDouble((double)Int(q));
        else if(isInt64(q))
          return ZObjFromDouble((double)toInt64(q));
        else if(isaFloat(q))
          return ZObjFromDouble(Float(q));
        else
          return Z_Err(ValueError,"The string cannot be converted to a float!\n");
			}
      else if(args[0].type == 'i')
      {
        args[0].f = (double)args[0].i;
        args[0].type = Z_FLOAT;
        return args[0];
      }
      else if(args[0].type == 'l')
      {
        args[0].f = (double)args[0].l;
        args[0].type = Z_FLOAT;
        return args[0];
      }
      else if(args[0].type == Z_FLOAT)
        return args[0];
      return Z_Err(TypeError,"Error float() unsupported for type "+fullform(args[0].type));
		}
		return Z_Err(ArgumentError,"Error float() takes exactly one argument!");
		exit(0);
}
ZObject tonumeric(ZObject* args,int32_t argc)
{
    if(argc==1)
    {
      if(args[0].type=='s' || args[0].type==Z_MSTR)
      {
          string& q = *(string*)args[0].ptr;
          if(isnum(q))
          {
              ZObject ret = nil;
              ret.type='i';
              ret.i = Int(q);
              return ret;
          }
          else if(isInt64(q))
          {
              ZObject ret = nil;
              ret.type = 'l';
              ret.l = toInt64(q);
              return ret;
          }
          else if(isaFloat(q))
          {
              ZObject ret = nil;
              ret.type = 'f';
              ret.f = Float(q);
              return ret;
          }
          else
          {
              return Z_Err(ValueError,"Error cannot convert the string \""+q+"\" to numeric type!");
          }
      }
      else
      {
          return Z_Err(TypeError,"Error tonumeric() takes a string argument!");
      }
		}
    return Z_Err(ArgumentError,"Error tonumeric() takes one argument!");
}
ZObject isnumeric(ZObject* args,int32_t argc)
{
  ZObject ret = nil;
  ret.type = 'b';
  ret.i = 1;
    if(argc==1)
    {
            if(args[0].type=='s' || args[0].type == Z_MSTR)
            {
               string& s = *(string*)args[0].ptr;
               if(isnum(s))
                   return ret;
               else if(isInt64(s))
                   return ret;
               else if(isaFloat(s))
                   return ret;
               else
               {
                 ret.i = 0;
                   return ret;
               }
            }
            else
            {
                return Z_Err(TypeError,"Error isnumeric() takes a string argument!");
            }
    }
    return Z_Err(ArgumentError,"Error isnumeric() takes 1 argument!");
}
ZObject REPLACE(ZObject* args,int32_t argc)
{
        if(argc==3)
        {
           if(args[0].type!='s')
               return Z_Err(TypeError,"Error first argument given to replace() must be a string!");
           if(args[1].type!='s')
               return Z_Err(TypeError,"Error second argument given to replace() must be a string!");
           if(args[2].type!='s')
               return Z_Err(TypeError,"Error third argument given to replace() must be a string!");
           string& a = *(string*)args[0].ptr;
           string& b = *(string*)args[1].ptr;
           string& c = *(string*)args[2].ptr;
           
           string* z = allocString();
           *z = replace_all(a,b,c);
           return ZObjFromStrPtr(z);
        }
        else
        {
            return Z_Err(ArgumentError,"Error replace() takes three arguments\n");
        }
}
ZObject REPLACE_ONCE(ZObject* args,int32_t argc)
{
        if(argc==3)
        {
           if(args[0].type!='s')
               return Z_Err(TypeError,"Error first argument given to replace_once() must be a string!");
           if(args[1].type!='s')
               return Z_Err(TypeError,"Error second argument given to replace_once() must be a string!");
           if(args[2].type!='s')
               return Z_Err(TypeError,"Error third argument given to replace_once() must be a string!");
           string& a = *(string*)args[0].ptr;
           string& b = *(string*)args[1].ptr;
           string& c = *(string*)args[2].ptr;
           string* z = allocString();
           *z = replace(a,b,c);
           return ZObjFromStrPtr(z);
        }
        else
        {
            return Z_Err(ArgumentError,"Error replace_once() takes three arguments\n");
        }
}
ZObject SLEEP(ZObject* args,int32_t argc)
{
    if(argc!=1)
        return Z_Err(ArgumentError,"Error sleep() takes 1 argument!");
    if(args[0].type=='i' && args[0].type!='l')
    {
      ZObject r = args[0];
      PromoteType(r,'l');
      #ifdef _WIN32
      #include <windows.h>
         Sleep(r.l);
      #else
        usleep(r.l*1000);
      #endif
      ZObject ret = nil;
      return ret;
    }
    else
    {
      return Z_Err(TypeError,"Error sleep() takes an integer argument!");
    }
}


ZObject TOBYTE(ZObject* args,int32_t argc)
{
  if(argc!=1)
    return Z_Err(ArgumentError,"Error byte() takes 1 argument!");
  if(args[0].type == Z_INT)
  {
    if(args[0].i>255 || args[0].i<0)
      return Z_Err(ValueError,"The integer must be in range 0 to 255!");
  }
  else if(args[0].type == Z_INT64)
  {
    if(args[0].l>255 || args[0].l<0)
      return Z_Err(ValueError,"The integer must be in range 0 to 255!");
    args[0].i = args[0].l;
  }
  else if(args[0].type == Z_BYTE || args[0].type == Z_BOOL)
  {
    args[0].type = Z_BYTE;
    return args[0];
  }
  else
    return Z_Err(TypeError,"Cannot convert type "+fullform(args[0].type)+" to byte!");
  args[0].type = Z_BYTE;
  return args[0];
}
ZObject writelines(ZObject* args,int32_t argc)
{
     if(argc==2)
        {
            if(args[0].type!='j')
                return Z_Err(TypeError,"Error first argument of writelines() should be a list!");
            if(args[1].type=='u')
            {
                const ZList& lines = *(ZList*)args[0].ptr;
                uint32_t f = 0;
                string data = "";
                ZObject m;
                while(f<lines.size())
                {
                    m = (lines[f]);
                    if(m.type!='s' && m.type!=Z_MSTR)
                      return Z_Err(ValueError,"List provided to writelines should consist of string elements only!");
                    data+=*(string*)m.ptr;
                    if(f!=lines.size()-1)
                    {
                        data+="\n";
                    }
                    f+=1;
                }
                FileObject* p = (FileObject*)args[1].ptr;
                FILE* currF = p->fp;
                fputs(data.c_str(),currF);
                ZObject ret = nil;
                return ret;
            }
            return Z_Err(ArgumentError,"Error writelines() needs a filestream to write!");
        }
        else
        {
            return Z_Err(ValueError,"Error writelines() takes two arguments!");
            exit(0);
        }
}
ZObject readlines(ZObject* args,int32_t argc)
{
    if(argc==1)
    {
      if(args[0].type!='u')
          return Z_Err(TypeError,"Argument not a filestream!");
      signed char ch;
      FileObject fobj = *(FileObject*)args[0].ptr;
      if(!fobj.open)
        return Z_Err(ValueError,"Error the file stream is closed!");
      FILE* currF = fobj.fp;
      ZList lines;
      string* reg = allocString();
      lines.push_back(ZObjFromStrPtr(reg));
      int32_t k = 0;

      while(true)
      {
          ch = fgetc(currF);
          if(ch==EOF)
          {
              break;
          }
          else if(ch=='\n')
          {
              k+=1;
              reg = allocString();
              lines.push_back(ZObjFromStrPtr(reg));
          }
          else
          {
            *reg+=ch;
          }
      }
      ZObject ret = nil;
      ZList* p = allocList();
      *p = lines;
      ret.type = 'j';
      ret.ptr = (void*)p;
      return ret;
    }
    else
    {
        return Z_Err(ArgumentError,"Error readlines() takes one argument!");
        exit(0);
    }
}
void clean_stdin(void)
{
    int32_t c = 0;
    while(c!='\n' && c!=EOF) 
    {
        c = getchar();
    }
}
ZObject FREAD(ZObject* args,int32_t argc)
{
    ZObject ret = nil;
    if(argc!=3)
        return Z_Err(ArgumentError,"Error fread() takes 3 arguments!");
    if (args[0].type != Z_BYTEARR)
        return Z_Err(TypeError, "Error first argument to fread must be a bytearray!");
    if(args[1].type!='i' && args[1].type!='l')
        return Z_Err(TypeError,"Error second argument of fread() should be an integer!");
    if(args[2].type!='u')
      return Z_Err(TypeError,"Error third argument of fread() should be a file stream!");
    ZObject a = args[1];
    PromoteType(a,'l');
    int64_t e = a.l;
    auto p = (vector<uint8_t>*)args[0].ptr;
    p->resize(e);
    FileObject fobj = *(FileObject*)args[2].ptr;
    if(!fobj.open)
      return Z_Err(ValueError,"Error the file stream is closed!");
    FILE* currF = fobj.fp;
    unsigned long long int read = 0;
    if((read = fread(&(p->at(0)),1,e,currF))!=(size_t)e)
        return Z_Err(FileIOError,"Error unable to read specified bytes from the file.");
    if(currF == stdin)
      clean_stdin();//the newline character can cause problem with future inputs or REPL
    return ZObjFromInt64(read);;
}
ZObject FWRITE(ZObject* args,int32_t argc)
{
    ZObject ret = nil;
    size_t S = 0;
    if(argc==2)
    {
    if(!validateArgTypes("fwrite","cu",args,argc,ret))
      return ret;
     S = ((vector<uint8_t>*)args[0].ptr)->size(); 
    }
    else if(argc==3)
    {
       if(args[0].type!='j' || args[1].type!='u' || (args[2].type!='i' && args[2].type!='l'))
         return Z_Err(TypeError,"Invalid Argument types");
       if(args[2].type=='i')
         S = (size_t)args[2].i;
        else
         S = (size_t)args[2].l;
    }
    else
    {
      return Z_Err(ArgumentError,"Error fwrite takes either 2 or 3 arguments");
    }
    auto l = (vector<uint8_t>*)args[0].ptr;
    if(S > l->size())
      return Z_Err(ValueError,"Error the bytearray needs to have specified number of bytes!");
    if(l->size() == 0)
      return ret;
    FileObject fobj = *(FileObject*)args[1].ptr;
    if(!fobj.open)
      return Z_Err(ValueError,"Error the file stream is closed!");
    FILE* currF = fobj.fp;
    int64_t written = 0;
    if((written = fwrite(&(l->at(0)),1,S,currF))!=(int64_t)S)
    {
        string what = strerror(errno);
        return Z_Err(FileIOError,"Error unable to write the bytes to file!");

    }
    return ZObjFromInt64(written);
}
ZObject FSEEK(ZObject* args,int32_t argc)
{
    if(argc!=3)
        return Z_Err(ArgumentError,"Error fseek() takes 3 arguments!");
    ZObject ret = nil;
    if(!validateArgTypes("fseek","uii",args,argc,ret))
      return ret;
    int32_t w = 0;
    int32_t whence = args[2].i;
    FileObject fobj = *(FileObject*)args[0].ptr;
    if(!fobj.open)
      return Z_Err(ValueError,"Error the file stream is closed!");
    FILE* currF = fobj.fp;
    int32_t where = args[1].i;
    if(whence==SEEK_SET)
    {
        w = SEEK_SET;
    }
    else if(whence==SEEK_END)
        w = SEEK_END;
    else if(whence==SEEK_CUR)
        w = SEEK_CUR;
    else
        return Z_Err(ValueError,"Error invalid option "+to_string(whence));
    if(fseek(currF,where,w)!=0)
      {
        string what = strerror(errno);
        return Z_Err(FileSeekError,what);
      }
    return ret;
}
//
ZList* makeListCopy(ZList);
Dictionary* makeDictCopy(Dictionary);
//
Dictionary* makeDictCopy(Dictionary v)
{
  Dictionary* d = allocDict();
  for(auto e: v)
  {
    ZObject key,val;
    key = e.first;
    val = e.second;
    d->emplace(key,val);
  }
  return d;
}
ZList* makeListCopy(ZList v)
{
  ZList* p = allocList();
  for(auto e: v)
  {
    ZObject elem;
    elem = e;
    p->push_back(elem);
  }
  return p;
}
ZObject COPY(ZObject* args,int32_t argc)
{
  if(argc!=1)
      return Z_Err(ArgumentError,"Error clone() takes one argument!");
  if(args[0].type=='a')
  {
     Dictionary v = *(Dictionary*)args[0].ptr;
     ZObject ret = nil;
     ret.type = 'a';
     ret.ptr = (void*)makeDictCopy(v);
     return ret;

  }
  else if(args[0].type=='j')
  {
    ZList v = *(ZList*)args[0].ptr;
    ZObject ret = nil;
    ret.type = 'j';
    ret.ptr = (void*)makeListCopy(v);
    return ret;
  }
  else
    return Z_Err(TypeError,"Error clone() takes a list or dictionary as an argument!");
}
ZObject POW(ZObject* args,int32_t argc)
{
    if(argc!=2)
    {
        return Z_Err(ArgumentError,"pow() takes 2 arguments!");
    }
    ZObject a = args[0];
    ZObject b = args[1];
    ZObject c;
    c.type = Z_NIL;
    char t;
    if(isNumeric(a.type) && isNumeric(b.type))
    {
      if(a.type=='f' || b.type=='f')
          t = 'f';
      else if(a.type=='l' || b.type=='l')
          t = 'l';
      else if(a.type=='i' || b.type=='i')
          t = 'i';
      PromoteType(a,t);
      PromoteType(b,t);
    }
    else
    {
      return Z_Err(TypeError,"Error pow() unsupported for "+fullform(a.type)+" and "+fullform(b.type));
    }
      if(t=='i')
      {
        c.type = 'i';
        if(!exponen_overflows(a.i,b.i))
        {
          c.i = pow(a.i,b.i);
          return c;
        }

        if(exponen_overflows((int64_t)a.i,(int64_t)b.i))
        {
          return Z_Err(OverflowError,"Integer Overflow occurred in pow()");
        }
        c.type = 'l';
        c.l = pow((int64_t)(a.i) , (int64_t)(b.i));
        return c;
      }
      else if(t=='f')
      {
        if(exponen_overflows(a.f,b.f))
        {
              return Z_Err(OverflowError,"Floating Point Overflow occurred in pow()");
        }
        c.type = 'f';
        c.f = pow(a.f,b.f);
        return c;
      }
      else if(t=='l')
      {
          if(exponen_overflows(a.l,b.l))
          {
              return Z_Err(OverflowError,"Integer Overflow occurred in pow().");
          }
          c.type = 'l';
          c.l = pow(a.l,b.l);
          return c;
      }
        return c;//to avoid warning otherwise this stmt is never executed
}
ZObject CLOCK(ZObject* args,int32_t argc)
{
  if(argc!=0)
    return Z_Err(ArgumentError,"Error clock() takes 0 arguments!");
  ZObject ret = nil;
  ret.type = 'f';
  ret.f = static_cast<double> (clock());
  return ret;
}
ZObject MUTABLESTRING(ZObject* args,int32_t argc)
{
  if(argc!=1 || (args[0].type != Z_STR && args[0].type!=Z_MSTR))
    return Z_Err(ArgumentError,"Error mutableString() takes 1 string argument!");
  const string& val = *(string*)args[0].ptr;
  string* p = allocMutString();
  *p = val;
  return ZObjFromMStrPtr(p);
}
////////////////////
//Builtin Methods
//Methods work exactly like functions but their first argument should always
//be an object
//these are just functions that work on multiple supported types
//for example POP functions works on both list,bytearrays and mutable strings
ZObject POP(ZObject* args,int32_t argc)
{
  if(args[0].type!=Z_LIST && args[0].type!=Z_BYTEARR && args[0].type!=Z_MSTR)
         return Z_Err(NameError,"Error type "+fullform(args[0].type)+" has no method named pop()");
  if(argc!=1)
    return Z_Err(ArgumentError,"Error method pop() takes 0 arguments!");//args[0] is self
  if(args[0].type == 'c')
  {
    ZObject ret = nil;

    auto p = (vector<uint8_t>*)args[0].ptr;
    if(p->size() != 0)
    {
      ret.type = Z_BYTE;
      ret.i = p->back();
      p->pop_back();
      return ret;
    }
    return ret;
  }
  else if(args[0].type == Z_MSTR)
  {
    string* p = (string*)args[0].ptr;
    if(p->size() == 0)
      return nil;
    string* s = allocMutString();
    (*s) += p->back();
    p->pop_back();
    return ZObjFromMStrPtr(s);
  }
  else
  {
    ZList* p = (ZList*)args[0].ptr;
    ZObject ret = nil;
    if(p->size()!=0)
    {
      ret = p->back();
      p->pop_back();
    }
    else
      return Z_Err(ValueError,"List is empty.No value to pop!");
    return ret;
  }
  return nil;
}

ZObject CLEAR(ZObject* args,int32_t argc)
{
  if(args[0].type!='j' && args[0].type!=Z_MSTR)
         return Z_Err(NameError,"Error type "+fullform(args[0].type)+" has no method named clear()");
  if(argc!=1)
    return Z_Err(ArgumentError,"Error method clear() takes 0 arguments!");
  if(args[0].type == Z_MSTR)
  {
    string* p = (string*)args[0].ptr;
    p->clear();
  }
  else
  {
    ZList* p = (ZList*)args[0].ptr;
    p->clear();
  }
  ZObject ret = nil;
  return ret;
}
ZObject PUSH(ZObject* args,int32_t argc)
{
  if(args[0].type!='j' && args[0].type!='c')
         return Z_Err(NameError,"Error type "+fullform(args[0].type)+" has no method named push()");
  if(argc!=2)
    return Z_Err(ArgumentError,"Error method push() takes 1 argument!");
  if(args[0].type == 'j')
  {
    ZList* p = (ZList*)args[0].ptr;
  // p->push_back(args[1]);
    p->emplace_back(args[1]); //faster
    ZObject ret = nil;
    return ret;
  }
  else
  {
    vector<uint8_t>* p = (vector<uint8_t>*)args[0].ptr;
    if(args[1].type != Z_BYTE)
     return Z_Err(TypeError,"Can only push a byte to a bytearray!");
    p->emplace_back((uint8_t)args[1].i); //faster
    ZObject ret = nil;
    return ret;
  }
}
ZObject RESERVE(ZObject* args,int32_t argc)
{
  if(args[0].type!='j' && args[0].type!='c' && args[0].type!=Z_MSTR)
         return Z_Err(NameError,"Error type "+fullform(args[0].type)+" has no method named push()");
  if(argc!=2)
    return Z_Err(ArgumentError,"Error method reserve() takes 1 argument!");
  if(args[1].type!='i' && args[1].type!='l')
    return Z_Err(TypeError,"Error reserve() takes an integer argument!");
  ZObject x = args[1];
  PromoteType(x,'l');
  if(args[0].type == Z_LIST)
  {
    ZList* p = (ZList*)args[0].ptr;
    p->reserve(x.l);
  }
  else if(args[0].type == Z_MSTR)
  {
    string* p = (string*) args[0].ptr;
    p->reserve(x.l);
  }
  else
  {
    vector<uint8_t>* p = (vector<uint8_t>*)args[0].ptr;
    p->reserve(x.l);
  }
  ZObject ret = nil;
  return ret;

}
ZObject FIND_METHOD(ZObject* args,int32_t argc)
{
  if(args[0].type!='j' && args[0].type!=Z_BYTEARR && args[0].type!=Z_STR && args[0].type!=Z_MSTR)
    return Z_Err(NameError,"Error type "+fullform(args[0].type)+" has no method named find()");
  if(argc!=2)
    return Z_Err(ArgumentError,"Error method find() takes 1 arguments!");
  if(args[0].type == 'j')
  {
    ZList* p = (ZList*)args[0].ptr;
    ZObject ret = nil;
    for(size_t k=0;k<p->size();k+=1)
    {
      if((*p)[k]==args[1])
      {
        ret.type = 'i';
        ret.i = k;
        return ret;
      }
    }
    return ret;
  }
  else if(args[0].type == Z_STR || args[0].type == Z_MSTR)
  {
    string* p = (string*)args[0].ptr;
    if(args[1].type != Z_STR && args[1].type!=Z_MSTR)
      return Z_Err(TypeError,"Argument 1 of str.find() must be a string!");
    const string& tofind = *(string*)args[1].ptr;
    size_t idx = p->find(tofind);
    if(idx == std::string::npos)
      return nil;
    return ZObjFromInt64((int64_t)idx);
  }
  else
  {
    vector<uint8_t>* p = (vector<uint8_t>*)args[0].ptr;
    ZObject ret = nil;
    for(size_t k=0;k<p->size();k+=1)
    {
      if((*p)[k]==(uint8_t)args[1].i)
      {
        ret.type = 'i';
        ret.i = k;
        return ret;
      }
    }
    return ret;
  }
}
ZObject INSERTBYTEARRAY(ZObject* args,int32_t argc)
{
  if(argc==3)
  {
    vector<uint8_t>* p = (vector<uint8_t>*)args[0].ptr;
    ZObject idx = args[1];
    ZObject val = args[2];
    if(idx.type!='i' && idx.type!='l')
      return Z_Err(TypeError,"Error method insert() expects an integer argument for position!");
    PromoteType(idx,'l');
    if(idx.l < 0)
      return Z_Err(ValueError,"Error insertion position is negative!");
    if((size_t)idx.l > p->size())
          return Z_Err(ValueError,"Error insertion position out of range!");
    if(val.type=='c')
    {
      auto subarr = *(vector<uint8_t>*)val.ptr;
      p->insert(p->begin()+idx.l,subarr.begin(),subarr.end());
    }
    else if(val.type == Z_BYTE)
    {
      p->insert(p->begin()+idx.l,(uint8_t)val.i);
    }
    else
      return Z_Err(TypeError,"Error method insert() takes a byte or bytearray argument!");
    ZObject ret = nil;
    return ret;
  }
  else
    return Z_Err(ArgumentError,"Error method insert() takes 2 arguments!");
}
ZObject INSERTMSTR(ZObject* args,int32_t argc)
{
  if(argc==3)
  {
    string* p = (string*)args[0].ptr;
    ZObject idx = args[1];
    ZObject val = args[2];
    if(idx.type!='i' && idx.type!='l')
      return Z_Err(TypeError,"Error method insert() expects an integer argument for position!");
    PromoteType(idx,'l');
    if(idx.l < 0)
      return Z_Err(ValueError,"Error insertion position is negative!");
    if((size_t)idx.l > p->size())
          return Z_Err(ValueError,"Error insertion position out of range!");
    if(val.type==Z_STR || val.type == Z_MSTR)
    {
      const string& sub = *(string*)val.ptr;
      p->insert(p->begin()+idx.l,sub.begin(),sub.end());
    }
    else
      return Z_Err(TypeError,"Error method insert() takes a string argument!");
    ZObject ret = nil;
    return ret;
  }
  else
    return Z_Err(ArgumentError,"Error method insert() takes 2 arguments!");
}
ZObject INSERTSTR(ZObject* args,int32_t argc)
{
  if(argc==3)
  {
    string* p = allocString();
    *p = *(string*)args[0].ptr;
    ZObject idx = args[1];
    ZObject val = args[2];
    if(idx.type!='i' && idx.type!='l')
      return Z_Err(TypeError,"Error method insert() expects an integer argument for position!");
    PromoteType(idx,'l');
    if(idx.l < 0)
      return Z_Err(ValueError,"Error insertion position is negative!");
    if((size_t)idx.l > p->size())
          return Z_Err(ValueError,"Error insertion position out of range!");
    if(val.type==Z_STR || val.type == Z_MSTR)
    {
      const string& sub = *(string*)val.ptr;
      p->insert(p->begin()+idx.l,sub.begin(),sub.end());
    }
    else
      return Z_Err(TypeError,"Error method insert() takes a string argument!");
    return ZObjFromStrPtr(p);
  }
  else
    return Z_Err(ArgumentError,"Error method insert() takes 2 arguments!");
}

ZObject INSERT(ZObject* args,int32_t argc)
{
  if(args[0].type == 'c')
    return INSERTBYTEARRAY(args,argc);
  else if(args[0].type == Z_MSTR)
    return INSERTMSTR(args,argc);
  else if(args[0].type == Z_STR)
    return INSERTSTR(args,argc);
  if(args[0].type!='j')
         return Z_Err(NameError,"Error type "+fullform(args[0].type)+" has no method named insert()");
  if(argc==3)
  {
    ZList* p = (ZList*)args[0].ptr;
    ZObject idx = args[1];
    ZObject val = args[2];
    if(idx.type!='i' && idx.type!='l')
      return Z_Err(TypeError,"Error method insert() expects an integer argument for position!");
    PromoteType(idx,'l');
    if(idx.l < 0)
      return Z_Err(ValueError,"Error insertion position is negative!");
    if((size_t)idx.l > p->size())
          return Z_Err(ValueError,"Error insertion position out of range!");
    if(val.type=='j')
    {
      ZList sublist = *(ZList*)val.ptr;
      p->insert(p->begin()+idx.l,sublist.begin(),sublist.end());
    }
    else
    {
      p->insert(p->begin()+idx.l,val);
    }
    ZObject ret = nil;
    return ret;
  }
  else
    return Z_Err(ArgumentError,"Error method insert() takes 3 arguments!");
}
ZObject ERASE(ZObject* args,int32_t argc)
{
  if(args[0].type=='j')
  {
    if(argc!=2 && argc!=3)
      return Z_Err(ArgumentError,"Error erase() takes 2 or 3 arguments!");
    ZList* p = (ZList*)args[0].ptr;
    ZObject idx1 = args[1];
    ZObject idx2;
    if(argc==3)
     idx2 = args[2];
    else
      idx2 = idx1;
    if(idx1.type!='i' && idx1.type!='l')
        return Z_Err(TypeError,"Error method erase() expects an integer argument for start!");
    if(idx2.type!='i' && idx2.type!='l')
        return Z_Err(TypeError,"Error method erase() expects an integer argument for end!");
    PromoteType(idx1,'l');
    PromoteType(idx2,'l');
    if(idx1.l < 0 || idx2.l < 0)
        return Z_Err(ValueError,"Error index is negative!");
    if((size_t)idx1.l >= p->size() || (size_t)idx2.l >= p->size())
        return Z_Err(ValueError,"Error index out of range!");
    p->erase(p->begin()+idx1.l,p->begin()+idx2.l+1);
    ZObject ret = nil;
    return ret;
  }
  else if(args[0].type=='a')
  {
     if(argc!=2)
       return Z_Err(ArgumentError,"Error dictionary method erase() takes 2 arguments!");
    Dictionary* d = (Dictionary*)args[0].ptr;
    ZObject key = args[1];
    if(key.type!='i' && key.type!='l' && key.type!='f' && key.type!='s' && key.type!='m' && key.type!='b')
      return Z_Err(TypeError,"Error key of type "+fullform(key.type)+" not allowed.");
    if(d->find(key)==d->end())
      return Z_Err(KeyError,"Error cannot erase value,key not found in the dictionary!");
    d->erase(key);
    ZObject ret = nil;
    return ret;
  }
  else if(args[0].type == 'c')
  {
    if(argc!=2 && argc!=3)
      return Z_Err(ArgumentError,"Error erase() takes 2 or 3 arguments!");
    vector<uint8_t>* p = (vector<uint8_t>*)args[0].ptr;
    ZObject idx1 = args[1];
    ZObject idx2;
    if(argc==3)
     idx2 = args[2];
    else
      idx2 = idx1;
    if(idx1.type!='i' && idx1.type!='l')
        return Z_Err(TypeError,"Error method erase() expects an integer argument for start!");
    if(idx2.type!='i' && idx2.type!='l')
        return Z_Err(TypeError,"Error method erase() expects an integer argument for end!");
    PromoteType(idx1,'l');
    PromoteType(idx2,'l');
    if(idx1.l < 0 || idx2.l < 0)
        return Z_Err(ValueError,"Error index is negative!");
    if((size_t)idx1.l >= p->size() || (size_t)idx2.l >= p->size())
        return Z_Err(ValueError,"Error index out of range!");
    p->erase(p->begin()+idx1.l,p->begin()+idx2.l+1);
    ZObject ret = nil;
    return ret;
  }
  else if(args[0].type == Z_MSTR)
  {
    if(argc!=2 && argc!=3)
      return Z_Err(ArgumentError,"Error erase() takes 2 or 3 arguments!");
    string* p = (string*)args[0].ptr;
    ZObject idx1 = args[1];
    ZObject idx2;
    if(argc==3)
     idx2 = args[2];
    else
      idx2 = idx1;
    if(idx1.type!='i' && idx1.type!='l')
        return Z_Err(TypeError,"Error method erase() expects an integer argument for start!");
    if(idx2.type!='i' && idx2.type!='l')
        return Z_Err(TypeError,"Error method erase() expects an integer argument for end!");
    PromoteType(idx1,'l');
    PromoteType(idx2,'l');
    if(idx1.l < 0 || idx2.l < 0)
        return Z_Err(ValueError,"Error index is negative!");
    if((size_t)idx1.l >= p->size() || (size_t)idx2.l >= p->size())
        return Z_Err(ValueError,"Error index out of range!");
    p->erase(p->begin()+idx1.l,p->begin()+idx2.l+1);
    ZObject ret = nil;
    return ret;
  }
  else if(args[0].type == Z_STR)
  {
    if(argc!=2 && argc!=3)
      return Z_Err(ArgumentError,"Error erase() takes 2 or 3 arguments!");
    string* p = (string*)args[0].ptr;
    ZObject idx1 = args[1];
    ZObject idx2;
    if(argc==3)
     idx2 = args[2];
    else
      idx2 = idx1;
    if(idx1.type!='i' && idx1.type!='l')
        return Z_Err(TypeError,"Error method erase() expects an integer argument for start!");
    if(idx2.type!='i' && idx2.type!='l')
        return Z_Err(TypeError,"Error method erase() expects an integer argument for end!");
    PromoteType(idx1,'l');
    PromoteType(idx2,'l');
    if(idx1.l < 0 || idx2.l < 0)
        return Z_Err(ValueError,"Error index is negative!");
    if((size_t)idx1.l >= p->size() || (size_t)idx2.l >= p->size())
        return Z_Err(ValueError,"Error index out of range!");
   
    string* res = allocString();
    *res = *p;
    res->erase(res->begin()+idx1.l,res->begin()+idx2.l+1);
    return ZObjFromStrPtr(res);
  }
  else
  {
      return Z_Err(NameError,"Error type "+fullform(args[0].type)+" has no member named erase.");
  }
}
ZObject asMap(ZObject* args,int32_t argc)
{
    if(args[0].type!='j')
      return Z_Err(NameError,"Error type "+fullform(args[0].type)+" has no member named asMap()");
    if(argc!=1)
      return Z_Err(ArgumentError,"Error list member asMap() takes 0 arguments!");
    ZList l = *(ZList*)args[0].ptr;
    ZObject x;
    x.type = 'l';
    size_t i = 0;
    Dictionary* d = allocDict();

    for(;i<l.size();i++)
    {
       x.l = i;
       d->emplace(x,l[i]);
    }
    ZObject ret = nil;
    ret.type = 'a';
    ret.ptr = (void*)d;
    return ret;
}
ZObject ASLIST(ZObject* args,int32_t argc)
{
    if(args[0].type!='a')
      return Z_Err(NameError,"Error type "+fullform(args[0].type)+" has no member named asList()");
    if(argc!=1)
      return Z_Err(ArgumentError,"Error dictionary member asList() takes 0 arguments!");
    Dictionary d = *(Dictionary*)args[0].ptr;
    ZList* list = allocList();
    for(auto e: d)
    {
      ZList* sub = allocList();
      sub->push_back(e.first);
      sub->push_back(e.second);
      ZObject x;
      x.type = 'j';
      x.ptr = (void*)sub;
      list->push_back(x);
    }
    ZObject ret = nil;
    ret.type = 'j';
    ret.ptr = (void*)list;
    return ret;
}
ZObject REVERSE_METHOD(ZObject* args,int32_t argc)
{
  if(args[0].type!='j' && args[0].type != Z_STR && args[0].type!=Z_MSTR)
         return Z_Err(NameError,"Error type "+fullform(args[0].type)+" has no method named reverse()");
  if(argc!=1)
    return Z_Err(ArgumentError,"Error method reverse() takes 0 arguments!");
  if(args[0].type == Z_STR)
  {
    string* p = allocString();
    string* str = (string*)args[0].ptr;
    *p = *str;
    std::reverse(p->begin(),p->end());
    return ZObjFromStrPtr(p);
  }
  else if(args[0].type == Z_MSTR)
  {
    string* str = (string*)args[0].ptr;
    std::reverse(str->begin(),str->end());
  }
  else
  {
    ZList* p = (ZList*)args[0].ptr;
    ZList l = *p;
    std::reverse(l.begin(),l.end());
    *p = l;
  }
  ZObject ret = nil;
  return ret;
}
ZObject EMPLACE(ZObject* args,int32_t argc)
{
  if(args[0].type!='a')
         return Z_Err(NameError,"Error type "+fullform(args[0].type)+" has no method named emplace()");
  if(argc!=3)
    return Z_Err(ArgumentError,"Error method emplace() takes 2 arguments!");
  Dictionary* p = (Dictionary*)args[0].ptr;
  ZObject& key = args[1];
  if(key.type!='i' && key.type!='l' && key.type!='f' && key.type!='s' && key.type!='m' && key.type!='b')
    return Z_Err(TypeError,"Error key of type "+fullform(key.type)+" not allowed.");
  p->emplace(key,args[2]);
  ZObject ret = nil;
  return ret;

}
ZObject HASKEY(ZObject* args,int32_t argc)
{
  if(args[0].type!='a')
         return Z_Err(NameError,"Error type "+fullform(args[0].type)+" has no method named hasKey()");
  if(argc!=2)
    return Z_Err(ArgumentError,"Error method hasKey() takes 1 argument!");
  Dictionary* p = (Dictionary*)args[0].ptr;

  ZObject ret = nil;
  ret.type= 'b';
  ret.i = (p->find(args[1])!=p->end());
  return ret;

}
ZObject UNPACK(ZObject* args,int32_t argc)
{
  if(args[0].type!=Z_BYTEARR)
    return Z_Err(NameError,"Error object has no member unpack()");
  if(argc!=2)
    return Z_Err(ArgumentError,"Error unpack() takes 2 arguments!");
  if(args[1].type!=Z_STR)
    return Z_Err(TypeError,"Error unpack() takes a string argument!");
  auto arr = (vector<uint8_t>*)args[0].ptr;
  string& pattern = *(string*)args[1].ptr;
  size_t k = 0;
  int32_t int32;
  int64_t int64;
  double d;
  bool b;
  string str;
  vector<ZObject>* lp = allocList();
  vector<ZObject>& res = *lp;
  for(size_t i=0;i<pattern.length();i++)
  {
    char ch = pattern[i];
    if(ch == 'i')
    {
      if(k+3 >= arr->size())
         return Z_Err(ValueError,"Error making element "+to_string(res.size())+" from bytearray(not enough bytes)!");
      memcpy(&int32,&arr->at(k),4);
      res.push_back(ZObjFromInt(int32));
      k+=4;
    }
    else if(ch == 'l')
    {
      if(k+7 >= arr->size())
         return Z_Err(ValueError,"Error making element "+to_string(res.size())+" from bytearray(not enough bytes)!");
      memcpy(&int64,&arr->at(k),8);
      res.push_back(ZObjFromInt64(int64));      
      k+=8;
    }
    else if(ch == 'f')
    {
      if(k+7 >= arr->size())
         return Z_Err(ValueError,"Error making element "+to_string(res.size())+" from bytearray(not enough bytes)!");
      memcpy(&d,&arr->at(k),8);
      res.push_back(ZObjFromDouble(d));
      k+=8;
    }
    else if(ch == 'b')
    {
      if(k >= arr->size())
         return Z_Err(ValueError,"Error making element "+to_string(res.size())+" from bytearray(not enough bytes)!");
      memcpy(&b,&arr->at(k),1);
      res.push_back(ZObjFromBool(b));
      k+=1;
    }
    else if(ch == 's')
    {
      if(i+1>=pattern.length())
        return Z_Err(ValueError,"Error in pattern,required length after 's' ");
      if(!isdigit(pattern[i+1]))
        return Z_Err(ValueError,"Error in pattern,required length after 's' ");
      string l;
      i+=1;
      l+=pattern[i];
      i+=1;
      while(i<pattern.length() && isdigit(pattern[i]))
      {
        l+=pattern[i];
        i++;
      }
      if(!isnum(l) && !isInt64(l))
        return Z_Err(OverflowError,"Error given string length too large!");
      long long int len = atoll(l.c_str());
      if((long long int)k+len-1 >= (long long int)arr->size())
         return Z_Err(ValueError,"Error making element "+to_string(res.size())+" from bytearray(not enough bytes)!");
      str = "";
      size_t f = k+(size_t)len;
      for(;k!=f;++k)
      {
        char c = (*arr)[k];
        str.push_back(c);
      }
      auto p = allocString();
      *p = str;
      res.push_back(ZObjFromStrPtr(p));
    }
    else
      return Z_Err(ValueError,"Error invalid char in pattern string!"); 
  }
  return ZObjFromList(lp);
}
//String methods
ZObject SUBSTR_METHOD(ZObject* args,int32_t argc)
{
  if(args[0].type != Z_STR && args[0].type!=Z_MSTR)
    return Z_Err(NameError,"Error type "+fullform(args[0].type)+" has no member named substr");
  if(argc!=3)
    return Z_Err(ArgumentError,"Error str.substr() takes 2 arguments");
  if(args[1].type!='i' && args[1].type!='l')
    return Z_Err(TypeError,"Error first argument of str.substr() should be an integer");
  if(args[2].type!='i' && args[2].type!='l')
    return Z_Err(TypeError,"Error second argument of str.substr() should be an integer");
  bool a = false;
  string* q = ((a = args[0].type == Z_STR)) ? allocString() : allocMutString();

  PromoteType(args[1],'l');
  PromoteType(args[2],'l');
  const string& data = *(string*)args[0].ptr;
  if(args[1].l < 0 || args[2].l < 0)
   ;
  else
    *q = substr((int32_t)args[1].l,(int32_t)args[2].l,data);
  
  return (a) ? ZObjFromStrPtr(q) : ZObjFromMStrPtr(q);
  
  
}
ZObject REPLACE_METHOD(ZObject* args,int32_t argc)
{
   if(args[0].type != Z_STR && args[0].type!=Z_MSTR)
    return Z_Err(NameError,"Error type "+fullform(args[0].type)+" has no member named replace");
    if(argc==3)
    {
        if(args[1].type!='s' && args[1].type!=Z_MSTR)
            return Z_Err(TypeError,"Error first argument given to replace() must be a string!");
        if(args[2].type!='s' && args[2].type!=Z_MSTR)
            return Z_Err(TypeError,"Error second argument given to replace() must be a string!");
      
        string& c = *(string*)args[0].ptr;
        string& a = *(string*)args[1].ptr;
        string& b = *(string*)args[2].ptr;
        if(args[0].type == Z_MSTR)
        {
          c = replace_all(a,b,c);
          return nil;
        }
        else
        {
          string* z = allocString();
          *z = replace_all(a,b,c);
          return ZObjFromStrPtr(z);
        }
    }
    else
    {
        return Z_Err(ArgumentError,"Error method replace() takes two arguments\n");
    }
    
}
ZObject REPLACE_ONCE_METHOD(ZObject* args,int32_t argc)
{
   if(args[0].type != Z_STR && args[0].type!=Z_MSTR)
    return Z_Err(NameError,"Error type "+fullform(args[0].type)+" has no member named replace");
    if(argc==3)
    {
        if(args[1].type!='s')
            return Z_Err(TypeError,"Error first argument given to replace() must be a string!");
        if(args[2].type!='s')
            return Z_Err(TypeError,"Error second argument given to replace() must be a string!");
      
        string& c = *(string*)args[0].ptr;
        string& a = *(string*)args[1].ptr;
        string& b = *(string*)args[2].ptr;
        if(args[0].type == Z_MSTR)
        {
          c = replace(a,b,c);
          return nil;
        }
        else
        {
          string* z = allocString();
          *z = replace(a,b,c);
          return ZObjFromStrPtr(z);
        }
    }
    else
    {
        return Z_Err(ArgumentError,"Error method replace() takes two arguments\n");
    }
}
//Mutable String
ZObject APPEND_METHOD(ZObject* args,int32_t argc)
{
  if(args[0].type!=Z_MSTR)
    return Z_Err(NameError,"Error type "+fullform(args[0].type)+" has no method named append()");
  if(argc!=2)
    return Z_Err(ArgumentError,"Error method append() takes 1 arguments!");
  
  string* p = (string*)args[0].ptr;
  if(args[1].type != Z_STR && args[1].type!=Z_MSTR)
    return Z_Err(TypeError,"Argument 1 of append() must be a string!");
  const string& toappend = *(string*)args[1].ptr;
  p->insert(p->length(),toappend);
  return nil;
}
////////////////////
///////////////
void initFunctions()
{
  nil.type = Z_NIL;
  funcs.emplace("print",&print);
  funcs.emplace("println",&println);
  funcs.emplace("printf",&PRINTF);
  funcs.emplace("input",&input);
  funcs.emplace("typeof",&TYPEOF);
  funcs.emplace("len",&LEN);
  funcs.emplace("open",&OPEN);
  funcs.emplace("read",&READ);
  funcs.emplace("close",&CLOSE);
  funcs.emplace("rand",&RAND);
  funcs.emplace("shuffle",&SHUFFLE);
  funcs.emplace("reverse",&REVERSE);
  funcs.emplace("getenv",&GETENV);
  funcs.emplace("system",&SYSTEM);
  funcs.emplace("readlines",&readlines);
  funcs.emplace("writelines",&writelines);
  funcs.emplace("write",&WRITE);
  funcs.emplace("getFileSize",getFileSize);
  funcs.emplace("fread",&FREAD);
  funcs.emplace("fwrite",&FWRITE);
  funcs.emplace("substr",&SUBSTR);
  funcs.emplace("find",&FIND);
  funcs.emplace("replace_once",&REPLACE_ONCE);
  funcs.emplace("replace",&REPLACE);
  funcs.emplace("sleep",&SLEEP);
  funcs.emplace("exit",&EXIT);
  funcs.emplace("split",&SPLIT);
  funcs.emplace("int",&TOINT);
  funcs.emplace("int32",&TOINT32);
  funcs.emplace("int64",&TOINT64);
  funcs.emplace("float",&TOFLOAT);
  funcs.emplace("tonumeric",&tonumeric);
  funcs.emplace("isnumeric",&isnumeric);
  funcs.emplace("fseek",&FSEEK);
  funcs.emplace("bytes",&BYTES);
  funcs.emplace("ListFromBytes",&makeList);
  funcs.emplace("str",&STR);
  funcs.emplace("byte",& TOBYTE);
  funcs.emplace("clone",&COPY);
  funcs.emplace("pow",&POW);
  funcs.emplace("obj_info",&OBJINFO);
  funcs.emplace("isalpha",&Z_ISALPHA);
  funcs.emplace("ftell",&FTELL);
  funcs.emplace("rewind",&REWIND);
  funcs.emplace("clock",&CLOCK);
  funcs.emplace("isInstanceOf",&isInstanceOf);
  funcs.emplace("ascii",&ASCII);
  funcs.emplace("char",&TOCHAR);
  funcs.emplace("bytearray",&BYTEARRAY);
  funcs.emplace("moduleInfo",&moduleInfo);
  funcs.emplace("format",&FORMAT);
  funcs.emplace("mutableString",&MUTABLESTRING);

}
std::unordered_map<string,BuiltinFunc> methods;
void initMethods()
{
  nil.type = Z_NIL;
  methods.emplace("push",&PUSH);
  methods.emplace("pop",&POP);
  methods.emplace("clear",&CLEAR);
  methods.emplace("insert",&INSERT);
  methods.emplace("find",&FIND_METHOD);
  methods.emplace("asMap",&asMap);
  methods.emplace("reserve",&RESERVE);
  methods.emplace("erase",&ERASE);
  methods.emplace("reverse",&REVERSE_METHOD);
  methods.emplace("emplace",&EMPLACE);
  methods.emplace("hasKey",&HASKEY);
  methods.emplace("asList",&ASLIST);
  methods.emplace("unpack",&UNPACK);
  methods.emplace("substr",&SUBSTR_METHOD);
  methods.emplace("replace",REPLACE_METHOD);
  methods.emplace("replace_once",REPLACE_ONCE_METHOD);
  methods.emplace("append",&APPEND_METHOD);

}
ZObject callmethod(string name,ZObject* args,int32_t argc)
{
     if(methods.find(name)==methods.end())
       return Z_Err(NameError,"Error "+fullform(args[0].type)+" type has no method named "+name+"()");
     return methods[name](args,argc);
}
bool function_exists(string name)
{
  if(funcs.find(name)!=funcs.end())
    return true;
  return false;
}
#endif
