#include "builtinfunc.h"
#include "vm.h"
#include "zuko.h"
#include "zobject.h"
#include "zlist.h"
#include <stdint.h>
#include <unordered_map>

using namespace std;

std::unordered_map<string,BuiltinFunc> funcs;

string fullform(char);
void PromoteType(ZObject&,char);

ZObject nil;
ZObject quickErr(Klass* k,string s)
{
  return Z_Err(k,s.c_str());
}
bool validateArgTypes(string name,string e,ZObject* args,int32_t argc,ZObject& x)
{
    if(e.length()!=(size_t)argc)
    {
      x = quickErr(ArgumentError,"Error "+name+"() takes "+str((int64_t)e.length())+" arguments!");
      return false;
    }
    for(int32_t f=1;f<=argc;f++)
    {
        ZObject k = args[f-1];
        if(k.type!=e[f-1])
            {
           x = quickErr(TypeError,"Error argument "+str(f)+" of "+name+"() should be of type "+fullform(e[f-1]));
            return false;
            }
        
    }
    return true;
}
ZObject Z_ISALPHA(ZObject* args,int32_t argc)
{
  if(argc!=1)
    return quickErr(ArgumentError,"Error isalpha() takes one argument!");
  if(args[0].type!=Z_STR)
    return quickErr(TypeError,"Error isalpha() takes a string argument!");
  ZStr* s = AS_STR(args[0]);
  ZObject ret = nil;
  ret.type = 'b';
  ret.i = 0;
  char* p = s->val;
  char ch;
  while((ch = *p++))
  {
    if(!isalpha(ch))
      return ret;
  }
  ret.i = 1;
  return ret;
}

ZObject ASCII(ZObject* args,int32_t argc)
{
  if(argc!=1)
    return quickErr(ArgumentError,"Error ascii() takes one argument!");
  if(args[0].type!='s')
    return quickErr(TypeError,"Error ascii() takes a string argument!");
  ZStr* s = AS_STR(args[0]);
  if(s->len != 1)
      return quickErr(ValueError,"Error ascii() takes a string argument of length 1!");
 
  ZObject ret = nil;
  ret.type = 'i';
  ret.i = s->val[0];
  return ret;
}
ZObject TOCHAR(ZObject* args,int32_t argc)
{
  if(argc!=1)
    return quickErr(ArgumentError,"Error char() takes one argument!");
  if(args[0].type!='i')
    return quickErr(TypeError,"Error char() takes an integer argument!");
  char ch = (char)args[0].i;
  ZStr* p = vm_allocString(1);
  p->val[0] = ch;
  return ZObjFromStrPtr(p);
}
string unescape(string);
void printZDict(ZDict* l,vector<void*> seen)
{
  if(std::find(seen.begin(),seen.end(),(void*)l)!=seen.end())
  {
    printf("{...}");
    return;
  }
  seen.push_back((void*)l);
  size_t k = l->size;
  size_t m = 0;
  printf("{");
  for(size_t i=0;i<l->capacity;i++)
  {
    if(l->table[i].stat != OCCUPIED)
      continue;
    ZObject key = l->table[i].key;
    ZObject val = l->table[i].val;
    void printList(ZList*,vector<void*> = {});

    if(key.type=='j')
      printList((ZList*)key.ptr,seen);
    else if(key.type=='a')
      printZDict((ZDict*)key.ptr,seen);
    else if(key.type=='s')
    {
      printf("\"%s\"",unescape(AS_STR(key)->val).c_str());
    }
    else
      printf("%s",ZObjectToStr(key).c_str());
    printf(" : ");
    //
    if(val.type=='j')
      printList((ZList*)val.ptr,seen);
    else if(val.type=='a')
      printZDict((ZDict*)val.ptr,seen);
    else if(val.type=='s')
    {
      printf("\"%s\"",unescape(AS_STR(val)->val).c_str());
    }
    else
      printf("%s",ZObjectToStr(val).c_str());
    if(m!=k-1)
      printf(",");
    m+=1;
  }
  printf("}");
}
void printList(ZList* l,vector<void*> seen)
{
  if(std::find(seen.begin(),seen.end(),(void*)l)!=seen.end())
  {
    printf("[...]");
    return;
  }
  seen.push_back((void*)l);
  size_t k = l->size;
  ZObject val;
  printf("[");
  for(size_t i=0;i<k;i+=1)
  {
    val = l->arr[i];
    if(val.type=='j')
      printList((ZList*)val.ptr,seen);
    else if(val.type=='a')
      printZDict((ZDict*)val.ptr,seen);
    else if(val.type=='s')
    {
      printf("\"%s\"",unescape(((ZStr*)val.ptr)->val).c_str());
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

void printByteArray(ZByteArr* arr)
{
  string IntToHex(int);
  size_t len = arr->size;
  printf("bytearr([");
  for(size_t i=0;i<len;i++)
  {
    printf("%s",IntToHex(arr->arr[i]).c_str());
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
           printZDict((ZDict*)args[k].ptr);
        else if(args[k].type==Z_BYTEARR)
          printByteArray((ZByteArr*)args[k].ptr);
        else if(args[k].type == Z_OBJ)
        {
          KlassObject* ki = (KlassObject*)args[k].ptr;
          ZObject r,ret;

          if(StrMap_get(&(ki->members),"__print__",&r))
          {
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
      return quickErr(ArgumentError,"At least one argument needed!");
    if(args[0].type!='s')
      return quickErr(TypeError,"Argument 1 must be a string!");
    ZStr* format = (ZStr*)args[0].ptr;
    int32_t k = 0;
    int32_t l = format->len;
    int32_t j = 1;
    while(k<l)
    {
        if(format->val[k] == '%')
        {
          if(k+1 < l && format->val[k+1] == '%')
          {
            printf("%%");
            k+=2;
            continue;
          }
          if(j>=argc)
            return quickErr(ArgumentError,"String format requires more arguments!");
          if(args[j].type=='j')
           printList((ZList*)args[j].ptr);
          else if(args[j].type=='a')
           printZDict((ZDict*)args[j].ptr);
          else if(args[j].type==Z_BYTEARR)
          printByteArray((ZByteArr*)args[j].ptr);
          else
           printf("%s",ZObjectToStr(args[j]).c_str());
          j+=1;
        }
        else
          printf("%c",format->val[k]);
        k+=1;
    }
    ZObject ret = nil;
    ret.type = 'n';
    return ret;
}
ZObject FORMAT(ZObject* args,int32_t argc)
{
    if(argc < 1)
      return quickErr(ArgumentError,"At least one argument needed!");
    if(args[0].type!=Z_STR)
      return quickErr(TypeError,"Argument 1 must be a string!");
    ZStr* format = (ZStr*)args[0].ptr;
    int32_t k = 0;
    int32_t l = format->len;
    int32_t j = 1;

    string p;
    while(k<l)
    {
        if(format->val[k] == '%')
        {
          if(k+1 < l && format->val[k+1] == '%')
          {
            p+="%%";
            k+=2;
            continue;
          }
          if(j>=argc)
            return quickErr(ArgumentError,"String format requires more arguments!");
          p += ZObjectToStr(args[j]);
          j+=1;
        }
        else
          p+=format->val[k];
        k+=1;
    }
    ZStr* s = vm_allocString(p.length());
    strcpy(s->val,p.c_str());
    return ZObjFromStrPtr(s);
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
          printZDict((ZDict*)args[k].ptr);
        else if(args[k].type==Z_BYTEARR)
          printByteArray((ZByteArr*)args[k].ptr);
        else if(args[k].type == Z_OBJ)
        {
          KlassObject* ki = (KlassObject*)args[k].ptr;
          ZObject r,ret;
          if(StrMap_get(&(ki->members),"__print__",&r))
            vm_callObject(&r,&r,1,&ret);
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
        return quickErr(ArgumentError,"Error input() takes 1 or 0 arguments!");
    }
    if(argc==1)
    {
      if(args[0].type!='s')
        return quickErr(TypeError,"Error input() takes a string argument!");
      char* prompt = ((ZStr*)args[0].ptr)->val;
      printf("%s",prompt);
    }
    string s;
    char ch;
    while(true)
    {
        ch = fgetc(stdin);
        if(ch=='\n')
          break;
        s+=ch;
    }
    ZStr* p = vm_allocString(s.length());
    strcpy(p->val,s.c_str());
    return ZObjFromStrPtr(p);
}
ZObject TYPEOF(ZObject* args,int32_t argc)
{
  if(argc!=1)
  {
      return quickErr(TypeError,"Error typeof() takes one argument only!");
  }
  string fullform(char);
  string s = fullform(args[0].type);
  ZStr* p = vm_allocString(s.length());
  strcpy(p->val,s.c_str());
  return ZObjFromStrPtr(p);
}
ZObject isInstanceOf(ZObject* args,int32_t argc)
{
  if(argc!=2)
    return quickErr(ArgumentError,"Error function isInstanceOf() takes 2 arguments!");
  if(args[1].type!='v')
    return quickErr(TypeError,"Error second argument to  isInstanceOf() should be a class!");
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
        return quickErr(ArgumentError,"Error len() takes one argument!");

    ZObject ret = nil;
    ret.type = Z_INT64;
    if(args[0].type=='s')
    {
      ZStr* s = (ZStr*)args[0].ptr;
      ret.l = s->len;
    }
    else if(args[0].type=='j')
        ret.l = ((ZList*)args[0].ptr)->size;
    else if(args[0].type=='a')
        ret.l = ((ZDict*)args[0].ptr)->size;
    else if(args[0].type == 'c')
        ret.l = ((ZByteArr*)args[0].ptr)->size;
    else
        return quickErr(TypeError,"Error len() unsupported for type "+fullform(args[0].type));
    return ret;
}
//////////
ZObject OPEN(ZObject* args,int32_t argc)
{
    string patt = "ss";
    ZObject ret = nil;
    if(!validateArgTypes("open",patt,args,argc,ret))
      return ret;
    string filename = AS_STR(args[0])->val;
    string mode = AS_STR(args[1])->val;
    if(mode!="r" && mode!="w" && mode!="a" && mode!="rw" && mode!="rw+" && mode!="rb" && mode!="wb")
        return quickErr(ValueError,"Error unknown mode: \""+mode+"\"");
    FILE* fp = fopen(filename.c_str(), mode.c_str());
    if(!fp)
        return quickErr(FileOpenError,strerror(errno));
    zfile* f = vm_alloczfile();
    f->fp = fp;
    f->open = true;
    ret.type = 'u';
    ret.ptr = (void*)f;
    return ret;
}
ZObject READ(ZObject* args,int32_t argc)
{
    if(argc!=2 && argc!=1)
        return quickErr(ArgumentError,"Error read() function takes one or two arguments!");
    if(args[0].type!='u')
        return quickErr(TypeError,"Error read() needs a file stream to read from!");
    char delim = EOF;
    if(argc==2)
    {
      if(args[1].type!='s')
        return quickErr(TypeError,"Error argument 2 to read() function should be of type string!");
      ZStr* l = AS_STR(args[1]);
      if(l->len!=1)
        return quickErr(ValueError,"Error optional delimeter argument to read() should be string of length 1");
      delim = l->val[0];  
    }
    zfile fobj = *(zfile*)args[0].ptr;
    if(!fobj.open)
      return quickErr(ValueError,"Error the file stream is closed!");
    FILE* fp = fobj.fp;
    if(!fp)
        return quickErr(FileIOError,"Error can't read from a closed file stream!");
    char ch;
    string p;
    while((ch = fgetc(fp))!=EOF)
    {
        if(ch==delim)
          break;
        p+=ch;
    }
    ZStr* s = vm_allocString(p.length());
    strcpy(s->val,p.c_str());
    return ZObjFromStrPtr(s);
}
ZObject CLOSE(ZObject* args,int32_t argc)
{
    if(argc!=1)
        return quickErr(ArgumentError,"Error close() takes 1 argument!");
  //  printf("args[0].type = %c\n",args[0].type);
    if(args[0].type!='u')
        return quickErr(TypeError,"Error close() takes a file stream as an argument!");
    zfile* fobj = (zfile*)args[0].ptr;
    if(!fobj->open)
    {
      return quickErr(ValueError,"Error file already closed!");
    }
    FILE* f = fobj->fp;
    if(f==stdin || f==stdout)
      return quickErr(ValueError,"Are you nuts?You should not close stdin or stdout!");
    fclose(f);
    ZObject ret = nil;
    ret.type='n';
    fobj->open = false;
    return ret;
}
ZObject RAND(ZObject* args,int32_t argc)
{
    if(argc!=0)
        return quickErr(ArgumentError,"Error rand() takes 0 arguments!");
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
    ZByteArr* arr = vm_allocByteArray();
    ret.type = Z_BYTEARR;
    ret.ptr = (void*)arr;
    return ret;
  }
  else if(argc == 1)
  {
    if(args[0].type != Z_LIST)
    {
      return quickErr(TypeError,"Error bytearray() takes a list argument!");
    }
    ZList* p = (ZList*)args[0].ptr;
    size_t len = p->size;
    ZByteArr* arr = vm_allocByteArray();
    ret.type = Z_BYTEARR;
    ret.ptr = (void*)arr;
    for(size_t i=0;i<len;i++)
    {
      if(p->arr[i].type!=Z_BYTE)
        return quickErr(TypeError,"Error argument list given to bytearray() must contain only bytes!");
      ZByteArr_push(arr,p->arr[i].i);
    }
    return ret;
  }
  return quickErr(ArgumentError,"Error bytearray() takes 0 or 1 arguments!");
}

ZObject WRITE(ZObject* args,int32_t argc)
{
  //printf("writing %s\n",args[0].s.c_str());
  if(argc!=2)
    return quickErr(ArgumentError,"Error write() takes two arguments!");
  string patt = "su";
  ZObject ret = nil;
  if(!validateArgTypes("write",patt,args,argc,ret))
    return ret;
  ZStr* data = AS_STR(args[0]);
  zfile* p = (zfile*)args[1].ptr;
  FILE* fp = p->fp;
  if(fputs(data->val,fp)==EOF)
    return quickErr(FileIOError,"Error while writing to file: "+(std::string)strerror(errno));
  ret.type = Z_NIL;
  return ret;
}
ZObject EXIT(ZObject* args,int32_t argc)
{
    int ret =  0;
    if(argc == 1)
    {
      if(args[0].type!=Z_INT)
        return quickErr(TypeError,"Integer argument required!");
      ret = args[0].i;
    }
    else if(argc == 0);
    else
        return quickErr(ArgumentError,"Error exit() takes either 0 or 1 argument!");
    exit(ret);
}
////////////////
ZObject REVERSE(ZObject* args,int32_t argc)
{
    if(argc!=1)
        return quickErr(ArgumentError,"Error reverse() takes 1 argument!");
    const ZObject& q  = args[0];
    if(q.type!='s' && q.type!='j')
        return quickErr(TypeError,"Error reverse() takes a string or list argument!");
    if(q.type=='s')
    {
        ZStr* data = AS_STR(q);
        ZStr* l = vm_allocString(data->len);
        size_t i = 0;
        for(int32_t k=data->len-1;k>=0;k--)
        {
            l->val[i++] = data->val[k];
        }
        return ZObjFromStrPtr(l);
    }
    ZList* l = vm_allocList();
    ZList* currList = (ZList*)q.ptr;
    for(int32_t k =currList->size-1;k>=0;k--)
    {
        ZList_push(l,currList->arr[k]);
    }
    ZObject ret = nil;// = l;
    ret.type = 'j';
    ret.ptr = (void*)l;
    return ret;
}///////
/////////
ZObject BYTES(ZObject* args,int32_t argc)
{
    if(argc!=1)
      return quickErr(ArgumentError,"Error bytes() takes one argument!");
    auto p = vm_allocByteArray();
    const ZObject& e = args[0];
    ZObject ret = nil;
    ret.type = Z_BYTEARR;
    ret.ptr = (void*)p;
    if(e.type=='i')
    {
      ZByteArr_resize(p,4);
      memcpy(p->arr,&e.i,4);
      return ret;
    }
    else if(e.type=='l')
    {
      ZByteArr_resize(p,8);
      memcpy(p->arr,&e.l,8);
      return ret;
    }
    else if(e.type=='f')
    {
      ZByteArr_resize(p,4);
      memcpy(p->arr,&e.f,4);
      return ret;
    }
    else if(e.type=='s')
    {
       ZStr* s = AS_STR(e);
       char* ptr = s->val;
       char ch;
       while((ch = *ptr++))
         ZByteArr_push(p,(uint8_t)ch);
       return ret;
    }
    else if(e.type=='b')
    {
      if(e.i)
        ZByteArr_push(p,1);
      else
        ZByteArr_push(p,0);
      return ret;
    }
    else if(e.type == Z_BYTE)
    {
      ZByteArr_push(p,e.i);
      return ret;
    }
    else
    {
        return quickErr(TypeError,"Error cannot convert "+fullform(e.type)+" type to bytes!");
    }
}
///////////////
ZObject OBJINFO(ZObject* args,int32_t argc)
{
    if(argc!=1)
      return quickErr(ArgumentError,"Error obj_info() takes 1 argument!");
    if( args[0].type=='o')
    {
      KlassObject* k = (KlassObject*)args[0].ptr;
      for(size_t idx = 0; idx < k->members.capacity;idx++)
      {
        if(k->members.table[idx].stat != SM_OCCUPIED)
          continue;
        auto& e = k->members.table[idx];
        printf("%s: %s\n",e.key,fullform(e.val.type).c_str());
          
      }
      for(size_t idx = 0; idx < k->privateMembers.capacity;idx++)
      {
        if(k->privateMembers.table[idx].stat != SM_OCCUPIED)
          continue;
        auto& e = k->privateMembers.table[idx];
        printf("%s: %s\n",e.key,fullform(e.val.type).c_str());
          
      }
      ZObject ret = nil;
      return ret;
    }
    else
    {
        return quickErr(TypeError,"Error argument is not an object of any class!");
    }
}
ZObject moduleInfo(ZObject* args,int32_t argc)
{
  if(argc!=1)
    return quickErr(ArgumentError,"Error moduleInfo takes 1 argument!");
  if(args[0].type!=Z_MODULE)
    return quickErr(TypeError,"Argument must be a module object!");
  Module* mod = (Module*)args[0].ptr;
  printf("Module %s\n",mod->name);
  printf("------------\n");
  for(size_t idx=0;idx<mod->members.capacity;idx++)
  {
    if(mod->members.table[idx].stat != SM_OCCUPIED)
      continue;
    auto& e = mod->members.table[idx];
    printf("%s  %s\n",e.key,fullform(e.val.type).c_str());

  }
  ZObject ret = nil;
  return ret;
}

/*
--Deprecated--

ZObject makeList(ZObject* args,int32_t argc)
{
        ZObject ret = nil;
        if(!validateArgTypes("makeList","js",args,argc,ret))
          return ret;
        string& pattern = *(ZStr*)args[1].ptr;
        size_t k = 0;
        ZList res;
        size_t i = 0;
        
        
        ZList currList = *(ZList*)args[0].ptr;
        while(k<currList.size)
        {
            if(i>pattern.length()-1)
            {
               return quickErr(ValueError,"Error no pattern specified for the remaining bytes!");
            }
            ZObject l = currList.arr[k];
            if(l.type!='m')
            {
                return quickErr(ValueError,"Error list should only contain bytes!");
            }
            int32_t b = l.i;
            if(pattern[i]=='i')
            {
               FOO.bytes[0] = b;
               if(k+3 >=currList.size)
               {
                   return quickErr(ValueError,"Error the list is missing some bytes!");
               }
               k+=1;
               l = currList.arr[k];
               FOO.bytes[1] = l.i;
               k+=1;
               l = currList.arr[k];
               FOO.bytes[1+1] = l.i;
               k+=1;
               l = currList.arr[k];
               FOO.bytes[3] = l.i;
               ZObject e;
               e.type = 'i';
               e.i =(int32_t) FOO.x;
               ZList_push(&res,e);
            }
            else if(pattern[i]=='b')
            {

               ZObject e;
               e.type = 'b';
               e.i = b;
              ZList_push(&res,e);
            }
            else if(pattern[i]=='l')
            {
               FOO1.bytes[0] = b;
               if(k+7 >=currList.size)
               {
                   return quickErr(ValueError,"Error the list is missing some bytes!");
               }
               k+=1;
               l = currList.arr[k];
               FOO1.bytes[1] = l.i;
               k+=1;
               l = currList.arr[k];
               FOO1.bytes[1+1] = l.i;
               k+=1;
               l = currList.arr[k];
               FOO1.bytes[3] = l.i;
               k+=1;
               l = currList.arr[k];
               FOO1.bytes[4] = l.i;
               k+=1;
               l = currList.arr[k];
               FOO1.bytes[5] = l.i;
               k+=1;
               l = currList.arr[k];
               FOO1.bytes[6] = l.i;
               k+=1;
               l = currList.arr[k];
               FOO1.bytes[7] = l.i;
               ZObject e;
               e.type = 'l';
               e.l = FOO1.l;

                             ZList_push(&res,e);
            }
            else if(pattern[i]=='s')
            {
                size_t j = k;
                ZObject e;
                ZStr* f = vm_allocString();
                bool terminated = false;
                while(true)
                {
                    if(j>=currList.size)
                    {
                        return quickErr(ValueError,"Ran out of bytes!");
                    }
                    e = currList.arr[j];
                    if(e.type!='m')
                        return quickErr(ValueError,"Error the list should only contain bytes!");
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
                    return quickErr(ValueError,"Error the bytes are invalid to be converted to string!");
                }

                ZList_push(&res,ZObjFromStrPtr(f));
                k  = j;
            }
            else if(pattern[i]=='f')
            {
               FOO2.bytes[0] = b;
               if(k+3 >=currList.size)
               {
                   return quickErr(ValueError,"Error the list is missing some bytes!");
               }
               k+=1;
               l = currList.arr[k];
               FOO2.bytes[1] = l.i;
               k+=1;
               l = currList.arr[k];
               FOO2.bytes[1+1] = l.i;
               k+=1;
               l = currList.arr[k];
               FOO2.bytes[3] = l.i;
               ZObject e;
               e.type = 'f';
               e.f = FOO2.f;
               ZList_push(&res,e);
            }
            else
            {
                return quickErr(TypeError,"Error unknown type used in pattern!");
            }
            i+=1;
            k+=1;
        }
        if(i!=pattern.length())
        {
            return quickErr(ValueError,"Error the list does not have enough bytes to follow the pattern!");
        }
        ZList* p = vm_allocList();
        ZList_assign(p,&res);
        ret.ptr = (void*)p;
        ret.type = 'j';
        return ret;
}*/
///////
/////////

ZObject SUBSTR(ZObject* args,int32_t argc)
{
    if(argc==3)
		{
			if(args[0].type!='i' && args[0].type!='l')
        return quickErr(TypeError,"Error first argument of substr() should be an integer");
      if(args[1].type!='i' && args[1].type!='l')
        return quickErr(TypeError,"Error second argument of substr() should be an integer");
			if(args[2].type=='s')
			{
        PromoteType(args[0],'l');
        PromoteType(args[1],'l');
        
        if(args[0].l<0 || args[1].l<0 )
        {
          ZStr* q = vm_allocString(0);
          return ZObjFromStrPtr(q);
        }
        ZStr* q = vm_allocString(args[1].l - args[0].l + 1);
        ZStr* data = (ZStr*)args[2].ptr;
        strcpy(q->val,substr((int32_t)args[0].l,(int32_t)args[1].l,data->val).c_str());
        return ZObjFromStrPtr(q);
			}
			else
        return quickErr(TypeError,"Error third argument of substr() should be a string!\n");
		}
		return quickErr(ArgumentError,"Error substr() takes three arguments!");
}
ZObject getFileSize(ZObject* args,int32_t argc)
{
  if(argc==1)
  {
    if(args[0].type!='u')
        return quickErr(TypeError,"Error getFileSize() takes an open file as argument!");
  
    zfile* p = (zfile*)args[0].ptr;
    if(!p->open)
      return quickErr(FileIOError,"Unable to get size of closed file!");
    FILE* currF = p->fp;
    fseek(currF,0,SEEK_END);
    int64_t n = ftell(currF);
    rewind(currF);
    return ZObjFromInt64(n);
  }
  return quickErr(ArgumentError,"Error getFileSize() takes 1 argument!");
}
ZObject FTELL(ZObject* args,int32_t argc)
{
  if(argc==1)
  {
    if(args[0].type!='u')
        return quickErr(TypeError,"Error ftell() takes an open file as argument!");
    zfile* p = (zfile*)args[0].ptr;
    if(!p->open)
      return quickErr(FileIOError,"Error file is closed!");
    FILE* currF = p->fp;
    int64_t n = ftell(currF);
    ZObject ret = nil;
    ret.type = 'l';
    ret.l = (int64_t)n;
    return ret;
  }
  return quickErr(ArgumentError,"Error ftell() takes 1 argument!");
}
ZObject REWIND(ZObject* args,int32_t argc)
{
  if(argc==1)
  {
    if(args[0].type!='u')
        return quickErr(TypeError,"Error rewind() takes an open file as argument!");
  
    zfile* p = (zfile*)args[0].ptr;
    if(!p->open)
      return quickErr(FileIOError,"Unable to rewind closed file!");
    FILE* currF = p->fp;
    rewind(currF);
    ZObject ret = nil;
    return ret;
  }
  return quickErr(ArgumentError,"Error rewind() takes 1 argument!");
}
ZObject SYSTEM(ZObject* args,int32_t argc)
{
  if(argc!=1)
      return quickErr(ArgumentError,"Error system() takes 1 argument!");
  if(args[0].type!='s')
      return quickErr(TypeError,"Error system() takes a string argument!");
  ZStr* command = (ZStr*)args[0].ptr;
  int32_t i = system(command->val);
  return ZObjFromInt(i);
}
ZObject SPLIT(ZObject* args,int32_t argc)
{
    if(argc==2)
		{
			if( (args[0].type=='s') && ( args[1].type=='s'))
			{
        ZStr* data = (ZStr*)args[0].ptr;
        ZStr* delim = (ZStr*)args[1].ptr;
				vector<string> list = split(data->val,delim->val);
				uint32_t  o = 0;
				ZList* l = vm_allocList();
				ZStr* value;
				while(o<list.size())
        {
          value = vm_allocString(list[o].length());
          strcpy(value->val,list[o].c_str());
          ZList_push(l,ZObjFromStrPtr(value));
          o+=1;
        }
        ZObject ret;
        ret.type = 'j';
        ret.ptr = l;
        return ret;
			}
			else
      {
        return quickErr(TypeError,"Error split() takes both string arguments!\n");
        exit(0);
      }
		}
		return quickErr(ArgumentError,"Error split() takes two arguments!");
}
ZObject GETENV(ZObject* args,int32_t argc)
{
  if(argc==1)
  {

      if(args[0].type=='s' )
      {
        ZStr* vname = (ZStr*)args[0].ptr;
        char* c = getenv(vname->val);
        if(!c)
          return nil;
        ZStr* s = vm_allocString(strlen(c));
        strcpy(s->val,c);
        return ZObjFromStrPtr(s);
      }
      else
      {
        return quickErr(TypeError,"Error getenv() takes a string argument!");
      }
  }
  return quickErr(ArgumentError,"Error getenv() takes one argument!");
}
ZObject SHUFFLE(ZObject* args,int32_t argc)
{
    if(argc==1)
		{
			if(args[0].type=='j')
			{
			  ZList* p = (ZList*)args[0].ptr;
        std::random_shuffle(p->arr,p->arr+p->size);
				ZObject ret = nil;
				return ret;
			}
			else
      {
        return quickErr(TypeError,"Error shuffle takes a list as an argument!");
      }
		}
		return quickErr(ArgumentError,"Error shuffle() takes exactly one argument!");
}
ZObject STR(ZObject* args,int32_t argc)
{
    if(argc==1)
	{
		if(args[0].type=='i')
		{
            string s = str(args[0].i);
            ZStr* p = vm_allocString(s.length());
            strcpy(p->val,s.c_str());
            return ZObjFromStrPtr(p);
		}
		else if(args[0].type=='f')
		{
            string s = str(args[0].f);
            ZStr* p = vm_allocString(s.length());
            strcpy(p->val,s.c_str());
            return ZObjFromStrPtr(p);
		}
        else if(args[0].type=='l')
        {
            string s = str(args[0].l);
            ZStr* p = vm_allocString(s.length());
            strcpy(p->val,s.c_str());
            return ZObjFromStrPtr(p);
        }
        else if(args[0].type == Z_BYTE)
        {
            string s = ZObjectToStr(args[0]);
            ZStr* p = vm_allocString(s.length());
            strcpy(p->val,s.c_str());
            return ZObjFromStrPtr(p);
        }
        else if(args[0].type == 'b')
        {
            ZStr* s = vm_allocString(5);
            if(args[0].i)
                strcpy(s->val,"true");
            else
                strcpy(s->val,"false");
            return ZObjFromStrPtr(s);
        }
        else if(args[0].type == 'j')
        {
          string s = ZObjectToStr(args[0]);
          ZStr* p = vm_allocString(s.length());
          strcpy(p->val,s.c_str());
          return ZObjFromStrPtr(p);
        }
        else if(args[0].type == 'a')
        {
          string s = ZObjectToStr(args[0]);
          ZStr* p = vm_allocString(s.length());
          strcpy(p->val,s.c_str());
          return ZObjFromStrPtr(p);
        }   
        else if(args[0].type == Z_BYTEARR)
        {
            string s;
            ZByteArr* bytes = (ZByteArr*)args[0].ptr;
            for(size_t i=0;i<bytes->size;i++)
            {
                uint8_t byte = bytes->arr[i];
                s.push_back((char)byte);
            }
            ZStr* p = vm_allocString(s.length());
            strcpy(p->val,s.c_str());
            return ZObjFromStrPtr(p);
        }
            return quickErr(TypeError,"Error str() unsupported for type "+fullform(args[0].type));
		}
    return quickErr(ArgumentError,"Error str() takes only one argument!");
}
ZObject FIND(ZObject* args,int32_t argc)
{
  ZObject ret = nil;
  if(argc != 2)
    return quickErr(ArgumentError,"Error find() takes 2 arguments!");
  if(args[0].type != Z_STR)
    return quickErr(TypeError,"Error first argument given to find() must be a stirng!");
  if(args[1].type != Z_STR)
    return quickErr(TypeError,"Error second argument given to find() must be a stirng!");
  
  ret.type = 'l';
  ZStr* a = (ZStr*)args[0].ptr;
  string b = ((ZStr*)args[1].ptr) -> val;
  auto y = b.find(a->val); 
  if(y==std::string::npos)
    return nil;
  else
    ret.l = static_cast<int64_t>(y);
  return ret;
}
ZObject TOINT(ZObject* args,int32_t argc)
{
    if(argc==1)
		{
			if(args[0].type=='s')
			{
			    ZStr* q = ((ZStr*)args[0].ptr);
			    ZObject ret = nil;
          if(isnum(q->val))
          {
              ret.i = Int(q->val);
              ret.type = 'i';
          }
          else if(isInt64(q->val))
          {
              ret.l = toInt64(q->val);
              ret.type = 'l';
          }
          else if(isaFloat(q->val))
          {
              string s = q->val;
              s = s.substr(0,s.find('.'));
              ret.type = Z_INT;
              if(isnum(s))
                ret.i = Int(s);
              else if(isInt64(s))
              {
                ret.type = Z_INT64;
                ret.l = toInt64(s);
              }
              else
              {
                ret.type = Z_INT64;
                ret.l =  LLONG_MAX;
              }
              return ret;
          }
          else
              return quickErr(ValueError,"Error the string "+(string)q->val+" cannot be converted to an integer!");
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
      return quickErr(TypeError,"Error int() unsupported for type "+fullform(args[0].type));
		}
		return quickErr(ArgumentError,"Error int() takes exactly one argument!");
		exit(0);
}
ZObject TOINT32(ZObject* args,int32_t argc)
{
    if(argc==1)
		{
			if(args[0].type=='s')
			{
			    ZStr* q = (ZStr*)args[0].ptr;
			    ZObject ret = nil;
          if(isnum(q->val))
          {
              ret.i = Int(q->val);
              ret.type = 'i';
          }
          else if(isInt64(q->val))
          {
              ret.i = INT_MAX;
              ret.type = Z_INT;
          }
          else if(isaFloat(q->val))
          {
              string s = q->val;
              s = s.substr(0,s.find('.'));
              ret.type = Z_INT;
              if(isnum(s))
                ret.i = Int(s);
              else if(isInt64(s))
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
              return quickErr(ValueError,"Error the string "+(string)q->val+" cannot be converted to an integer!");
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
      return quickErr(TypeError,"Error int32() unsupported for type "+fullform(args[0].type));
		}
		return quickErr(ArgumentError,"Error int32() takes exactly one argument!");
		exit(0);
}
ZObject TOINT64(ZObject* args,int32_t argc)
{
    if(argc==1)
		{
			if(args[0].type=='s')
			{
			    ZStr* q = (ZStr*)args[0].ptr;
			    ZObject ret = nil;
          if(isnum(q->val))
          {
              ret.l = Int(q->val);
              ret.type = Z_INT64;
          }
          else if(isInt64(q->val))
          {
              ret.l = toInt64(q->val);
              ret.type = 'l';
          }
          else if(isaFloat(q->val))
          {
              string s = q->val;
              s = s.substr(0,s.find('.'));
              if(isInt64(s))
              {
                ret.type = Z_INT64;
                ret.l = toInt64(s);
              }
              else
              {
                ret.type = Z_INT64;
                ret.l =  LLONG_MAX;
              }
              return ret;
          }
          else
              return quickErr(ValueError,"Error the string "+(string)q->val+" cannot be converted to an integer!");
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
      return quickErr(TypeError,"Error int64() unsupported for type "+fullform(args[0].type));
		}
		return quickErr(ArgumentError,"Error int64() takes exactly one argument!");
		exit(0);
}
ZObject TOFLOAT(ZObject* args,int32_t argc)
{
    if(argc==1)
		{
			if(args[0].type=='s')
			{
        ZStr* q = (ZStr*)args[0].ptr;
        if(isnum(q->val))
          return ZObjFromDouble((double)Int(q->val));
        else if(isInt64(q->val))
          return ZObjFromDouble((double)toInt64(q->val));
        else if(isaFloat(q->val))
          return ZObjFromDouble(Float(q->val));
        else
          return quickErr(ValueError,"The string cannot be converted to a float!\n");
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
      return quickErr(TypeError,"Error float() unsupported for type "+fullform(args[0].type));
		}
		return quickErr(ArgumentError,"Error float() takes exactly one argument!");
		exit(0);
}
ZObject tonumeric(ZObject* args,int32_t argc)
{
    if(argc==1)
    {
      if(args[0].type=='s')
      {
          ZStr* q = (ZStr*)args[0].ptr;
          if(isnum(q->val))
          {
              ZObject ret = nil;
              ret.type='i';
              ret.i = Int(q->val);
              return ret;
          }
          else if(isInt64(q->val))
          {
              ZObject ret = nil;
              ret.type = 'l';
              ret.l = toInt64(q->val);
              return ret;
          }
          else if(isaFloat(q->val))
          {
              ZObject ret = nil;
              ret.type = 'f';
              ret.f = Float(q->val);
              return ret;
          }
          else
          {
              return quickErr(ValueError,"Error cannot convert the string \""+(string)q->val+"\" to numeric type!");
          }
      }
      else
      {
          return quickErr(TypeError,"Error tonumeric() takes a string argument!");
      }
		}
    return quickErr(ArgumentError,"Error tonumeric() takes one argument!");
}
ZObject isnumeric(ZObject* args,int32_t argc)
{
  ZObject ret = nil;
  ret.type = 'b';
  ret.i = 1;
    if(argc==1)
    {
            if(args[0].type=='s')
            {
               ZStr* s = (ZStr*)args[0].ptr;
               if(isnum(s->val))
                   return ret;
               else if(isInt64(s->val))
                   return ret;
               else if(isaFloat(s->val))
                   return ret;
               else
               {
                 ret.i = 0;
                   return ret;
               }
            }
            else
            {
                return quickErr(TypeError,"Error isnumeric() takes a string argument!");
            }
    }
    return quickErr(ArgumentError,"Error isnumeric() takes 1 argument!");
}
ZObject REPLACE(ZObject* args,int32_t argc)
{
        if(argc==3)
        {
           if(args[0].type!='s')
               return quickErr(TypeError,"Error first argument given to replace() must be a string!");
           if(args[1].type!='s')
               return quickErr(TypeError,"Error second argument given to replace() must be a string!");
           if(args[2].type!='s')
               return quickErr(TypeError,"Error third argument given to replace() must be a string!");
           ZStr* a = (ZStr*)args[0].ptr;
           ZStr* b = (ZStr*)args[1].ptr;
           ZStr* c = (ZStr*)args[2].ptr;
           string s = replace_all(a->val,b->val,c->val);
           ZStr* z = vm_allocString(s.length());
           strcpy(z->val,s.c_str());
           return ZObjFromStrPtr(z);
        }
        else
        {
            return quickErr(ArgumentError,"Error replace() takes three arguments\n");
        }
}
ZObject REPLACE_ONCE(ZObject* args,int32_t argc)
{
        if(argc==3)
        {
           if(args[0].type!='s')
               return quickErr(TypeError,"Error first argument given to replace_once() must be a string!");
           if(args[1].type!='s')
               return quickErr(TypeError,"Error second argument given to replace_once() must be a string!");
           if(args[2].type!='s')
               return quickErr(TypeError,"Error third argument given to replace_once() must be a string!");
           ZStr* a = (ZStr*)args[0].ptr;
           ZStr* b = (ZStr*)args[1].ptr;
           ZStr* c = (ZStr*)args[2].ptr;
           string s = replace(a->val,b->val,c->val);
           ZStr* z = vm_allocString(s.length());
           strcpy(z->val,s.c_str());
           return ZObjFromStrPtr(z);
        }
        else
        {
            return quickErr(ArgumentError,"Error replace_once() takes three arguments\n");
        }
}
ZObject SLEEP(ZObject* args,int32_t argc)
{
    if(argc!=1)
        return quickErr(ArgumentError,"Error sleep() takes 1 argument!");
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
      return quickErr(TypeError,"Error sleep() takes an integer argument!");
    }
}


ZObject TOBYTE(ZObject* args,int32_t argc)
{
  if(argc!=1)
    return quickErr(ArgumentError,"Error byte() takes 1 argument!");
  if(args[0].type == Z_INT)
  {
    if(args[0].i>255 || args[0].i<0)
      return quickErr(ValueError,"The integer must be in range 0 to 255!");
  }
  else if(args[0].type == Z_INT64)
  {
    if(args[0].l>255 || args[0].l<0)
      return quickErr(ValueError,"The integer must be in range 0 to 255!");
    args[0].i = args[0].l;
  }
  else if(args[0].type == Z_BYTE || args[0].type == Z_BOOL)
  {
    args[0].type = Z_BYTE;
    return args[0];
  }
  else
    return quickErr(TypeError,"Cannot convert type "+fullform(args[0].type)+" to byte!");
  args[0].type = Z_BYTE;
  return args[0];
}
ZObject writelines(ZObject* args,int32_t argc)
{
     if(argc==2)
        {
            if(args[0].type!='j')
                return quickErr(TypeError,"Error first argument of writelines() should be a list!");
            if(args[1].type=='u')
            {
                ZList* lines = (ZList*)args[0].ptr;
                uint32_t f = 0;
                string data = "";
                ZObject m;
                zfile* p = (zfile*)args[1].ptr;
                FILE* currF = p->fp;
                while(f<lines->size)
                {
                    m = (lines->arr[f]);
                    if(m.type!='s')
                      return quickErr(ValueError,"List provided to writelines should consist of string elements only!");
                    ZStr* line = (ZStr*)m.ptr;
                    fputs(line->val,currF);
                    if(f!=lines->size-1)
                        fputc('\n',currF);
                    f+=1;
                }
                return nil;
            }
            return quickErr(ArgumentError,"Error writelines() needs a filestream to write!");
        }
        else
        {
            return quickErr(ValueError,"Error writelines() takes two arguments!");
            exit(0);
        }
}
ZObject readlines(ZObject* args,int32_t argc)
{
    if(argc==1)
    {
      if(args[0].type!='u')
          return quickErr(TypeError,"Argument not a filestream!");
      signed char ch;
      zfile fobj = *(zfile*)args[0].ptr;
      if(!fobj.open)
        return quickErr(ValueError,"Error the file stream is closed!");
      FILE* currF = fobj.fp;
      ZList* lines = vm_allocList();
      string reg; 
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
              ZStr* line = vm_allocString(reg.length());
              strcpy(line->val,reg.c_str());
              ZList_push(lines,ZObjFromStrPtr(line));
              reg = "";

          }
          else
          {
            reg+=ch;
          }
      }
      ZStr* line = vm_allocString(reg.length());
      strcpy(line->val,reg.c_str());
      ZList_push(lines,ZObjFromStrPtr(line));
      ZObject ret;
      ret.type = 'j';
      ret.ptr = (void*)lines;
      return ret;
    }
    else
    {
        return quickErr(ArgumentError,"Error readlines() takes one argument!");
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
    if(argc!=3)
        return quickErr(ArgumentError,"Error fread() takes 3 arguments!");
    if (args[0].type != Z_BYTEARR)
        return quickErr(TypeError, "Error first argument to fread must be a bytearray!");
    if(args[1].type!='i' && args[1].type!='l')
        return quickErr(TypeError,"Error second argument of fread() should be an integer!");
    if(args[2].type!='u')
      return quickErr(TypeError,"Error third argument of fread() should be a file stream!");
    ZObject a = args[1];
    PromoteType(a,'l');
    int64_t e = a.l;
    auto p = (ZByteArr*)args[0].ptr;
    ZByteArr_resize(p,e);
    zfile fobj = *(zfile*)args[2].ptr;
    if(!fobj.open)
      return quickErr(ValueError,"Error the file stream is closed!");
    FILE* currF = fobj.fp;
    unsigned long long int read = 0;
    if((read = fread(p->arr,1,e,currF))!=(size_t)e)
        return quickErr(FileIOError,"Error unable to read specified bytes from the file.");
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
     S = ((ZByteArr*)args[0].ptr)->size; 
    }
    else if(argc==3)
    {
       if(args[0].type!='j' || args[1].type!='u' || (args[2].type!='i' && args[2].type!='l'))
         return quickErr(TypeError,"Invalid Argument types");
       if(args[2].type=='i')
         S = (size_t)args[2].i;
        else
         S = (size_t)args[2].l;
    }
    else
    {
      return quickErr(ArgumentError,"Error fwrite takes either 2 or 3 arguments");
    }
    auto l = (ZByteArr*)args[0].ptr;
    if(S > l->size)
      return quickErr(ValueError,"Error the bytearray needs to have specified number of bytes!");
    if(l->size == 0)
      return ret;
    zfile fobj = *(zfile*)args[1].ptr;
    if(!fobj.open)
      return quickErr(ValueError,"Error the file stream is closed!");
    FILE* currF = fobj.fp;
    int64_t written = 0;
    if((written = fwrite(l->arr,1,S,currF))!=(int64_t)S)
    {
        string what = strerror(errno);
        return quickErr(FileIOError,"Error unable to write the bytes to file!");

    }
    return ZObjFromInt64(written);
}
ZObject FSEEK(ZObject* args,int32_t argc)
{
    if(argc!=3)
        return quickErr(ArgumentError,"Error fseek() takes 3 arguments!");
    ZObject ret = nil;
    if(!validateArgTypes("fseek","uii",args,argc,ret))
      return ret;
    int32_t w = 0;
    int32_t whence = args[2].i;
    zfile fobj = *(zfile*)args[0].ptr;
    if(!fobj.open)
      return quickErr(ValueError,"Error the file stream is closed!");
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
        return quickErr(ValueError,"Error invalid option "+to_string(whence));
    if(fseek(currF,where,w)!=0)
      {
        string what = strerror(errno);
        return quickErr(FileSeekError,what);
      }
    return ret;
}
//
ZList* makeListCopy(ZList);
ZDict* makeDictCopy(ZDict);
//
ZDict* makeDictCopy(ZDict v)
{
  ZDict* d = vm_allocDict();
  for(size_t i=0;i<d->capacity;i++)
  {
    if(d->table[i].stat != OCCUPIED)
      continue;
    ZObject key,val;
    key = d->table[i].key;
    val = d->table[i].val;
    ZDict_set(d,key,val);
  }
  return d;
}
ZList* makeListCopy(ZList v)
{
  ZList* p = vm_allocList();
  for(size_t i = 0;i<v.size;i++)
  {
    ZObject e = v.arr[i];
    ZObject elem;
    elem = e;
    ZList_push(p,elem);
  }
  return p;
}
ZObject COPY(ZObject* args,int32_t argc)
{
  if(argc!=1)
      return quickErr(ArgumentError,"Error clone() takes one argument!");
  if(args[0].type=='a')
  {
     ZDict v = *(ZDict*)args[0].ptr;
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
    return quickErr(TypeError,"Error clone() takes a list or dictionary as an argument!");
}
ZObject POW(ZObject* args,int32_t argc)
{
    if(argc!=2)
    {
        return quickErr(ArgumentError,"pow() takes 2 arguments!");
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
      return quickErr(TypeError,"Error pow() unsupported for "+fullform(a.type)+" and "+fullform(b.type));
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
          return quickErr(OverflowError,"Integer Overflow occurred in pow()");
        }
        c.type = 'l';
        c.l = pow((int64_t)(a.i) , (int64_t)(b.i));
        return c;
      }
      else if(t=='f')
      {
        if(exponen_overflows(a.f,b.f))
        {
              return quickErr(OverflowError,"Floating Point Overflow occurred in pow()");
        }
        c.type = 'f';
        c.f = pow(a.f,b.f);
        return c;
      }
      else if(t=='l')
      {
          if(exponen_overflows(a.l,b.l))
          {
              return quickErr(OverflowError,"Integer Overflow occurred in pow().");
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
    return quickErr(ArgumentError,"Error clock() takes 0 arguments!");
  ZObject ret = nil;
  ret.type = 'f';
  ret.f = static_cast<double> (clock());
  return ret;
}

////////////////////
//Builtin Methods
//Methods work exactly like functions but their first argument should always
//be an object
//these are just functions that work on multiple supported types
//for example POP functions works on both list,bytearrays and mutable strings
ZObject POP(ZObject* args,int32_t argc)
{
  if(args[0].type!=Z_LIST && args[0].type!=Z_BYTEARR )
         return quickErr(NameError,"Error type "+fullform(args[0].type)+" has no method named pop()");
  if(argc!=1)
    return quickErr(ArgumentError,"Error method pop() takes 0 arguments!");//args[0] is self
  if(args[0].type == 'c')
  {
    ZObject ret = nil;

    auto p = (ZByteArr*)args[0].ptr;
    uint8_t res;
    ZByteArr_pop(p,&res);
    ret.type = Z_BYTE;
    ret.i = res;
    return ret;
  }
  else
  {
    ZList* p = (ZList*)args[0].ptr;
    ZObject ret = nil;
    if(p->size!=0)
    {
      ZList_pop(p,&ret);
    }
    else
      return quickErr(ValueError,"List is empty.No value to pop!");
    return ret;
  }
  return nil;
}

ZObject CLEAR(ZObject* args,int32_t argc)
{
  if(args[0].type!='j' && args[0].type != Z_BYTEARR)
         return quickErr(NameError,"Error type "+fullform(args[0].type)+" has no method named clear()");
  if(argc!=1)
    return quickErr(ArgumentError,"Error method clear() takes 0 arguments!");
  if(args[0].type == Z_BYTEARR)
  {
    ZByteArr* arr = (ZByteArr*)args[0].ptr;
    arr->size = 0;
  }
  else
  {
    ZList* p = (ZList*)args[0].ptr;
    ZList_resize(p,0);
  }
  return nil;
}
ZObject PUSH(ZObject* args,int32_t argc)
{
  if(args[0].type!='j' && args[0].type!='c')
         return quickErr(NameError,"Error type "+fullform(args[0].type)+" has no method named push()");
  if(argc!=2)
    return quickErr(ArgumentError,"Error method push() takes 1 argument!");
  if(args[0].type == 'j')
  {
    ZList* p = (ZList*)args[0].ptr;

    ZList_push(p,args[1]);
    ZObject ret = nil;
    return ret;
  }
  else
  {
    ZByteArr* p = (ZByteArr*)args[0].ptr;
    if(args[1].type != Z_BYTE)
     return quickErr(TypeError,"Can only push a byte to a bytearray!");
    ZByteArr_push(p,args[1].i);
    ZObject ret = nil;
    return ret;
  }
}

ZObject FIND_METHOD(ZObject* args,int32_t argc)
{
  if(args[0].type!='j' && args[0].type!=Z_BYTEARR && args[0].type!=Z_STR)
    return quickErr(NameError,"Error type "+fullform(args[0].type)+" has no method named find()");
  if(argc!=2)
    return quickErr(ArgumentError,"Error method find() takes 1 arguments!");
  if(args[0].type == 'j')
  {
    ZList* p = (ZList*)args[0].ptr;
    ZObject ret = nil;
    for(size_t k=0;k<p->size;k+=1)
    {
      if(ZObject_equals(p->arr[k],args[1]))
      {
        ret.type = 'i';
        ret.i = k;
        return ret;
      }
    }
    return ret;
  }
  else if(args[0].type == Z_STR )
  {
    string p = ((ZStr*)args[0].ptr) -> val;
    if(args[1].type != Z_STR)
      return quickErr(TypeError,"Argument 1 of str.find() must be a string!");
    ZStr* tofind = (ZStr*)args[1].ptr;
    size_t idx = p.find(tofind->val);
    if(idx == std::string::npos)
      return nil;
    return ZObjFromInt64((int64_t)idx);
  }
  else
  {
    ZByteArr* p = (ZByteArr*)args[0].ptr;
    ZObject ret = nil;
    for(size_t k=0;k<p->size;k+=1)
    {
      if(p->arr[k]==(uint8_t)args[1].i)
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
    ZByteArr* p = (ZByteArr*)args[0].ptr;
    ZObject idx = args[1];
    ZObject val = args[2];
    if(idx.type!='i' && idx.type!='l')
      return quickErr(TypeError,"Error method insert() expects an integer argument for position!");
    PromoteType(idx,'l');
    if(idx.l < 0)
      return quickErr(ValueError,"Error insertion position is negative!");
    if((size_t)idx.l > p->size)
          return quickErr(ValueError,"Error insertion position out of range!");
    if(val.type=='c')
    {
      auto subarr = (ZByteArr*)val.ptr;
      ZByteArr_insertArr(p,idx.l,subarr);
    }
    else if(val.type == Z_BYTE)
    {
      ZByteArr_insert(p,idx.l,val.i);
    }
    else
      return quickErr(TypeError,"Error method insert() takes a byte or bytearray argument!");
    ZObject ret = nil;
    return ret;
  }
  else
    return quickErr(ArgumentError,"Error method insert() takes 2 arguments!");
}

ZObject INSERTSTR(ZObject* args,int32_t argc)
{
  if(argc==3)
  {
    string p = ((ZStr*)args[0].ptr) -> val;
    ZObject idx = args[1];
    ZObject val = args[2];
    if(idx.type!='i' && idx.type!='l')
      return quickErr(TypeError,"Error method insert() expects an integer argument for position!");
    PromoteType(idx,'l');
    if(idx.l < 0)
      return quickErr(ValueError,"Error insertion position is negative!");
    if((size_t)idx.l > p.size())
          return quickErr(ValueError,"Error insertion position out of range!");
    if(val.type==Z_STR)
    {
      ZStr* sub = (ZStr*)val.ptr;
      ZStr* result = vm_allocString(sub->len + p.length());
      string q = sub->val;
      p.insert(p.begin()+idx.l,q.begin(),q.end());
      strcpy(result->val,p.c_str());
      return ZObjFromStrPtr(result);
    }
    else
      return quickErr(TypeError,"Error method insert() takes a string argument!");
  }
  else
    return quickErr(ArgumentError,"Error method insert() takes 2 arguments!");
}

ZObject INSERT(ZObject* args,int32_t argc)
{
  if(args[0].type == 'c')
    return INSERTBYTEARRAY(args,argc);
  else if(args[0].type == Z_STR)
    return INSERTSTR(args,argc);
  if(args[0].type!='j')
         return quickErr(NameError,"Error type "+fullform(args[0].type)+" has no method named insert()");
  if(argc==3)
  {
    ZList* p = (ZList*)args[0].ptr;
    ZObject idx = args[1];
    ZObject val = args[2];
    if(idx.type!='i' && idx.type!='l')
      return quickErr(TypeError,"Error method insert() expects an integer argument for position!");
    PromoteType(idx,'l');
    if(idx.l < 0)
      return quickErr(ValueError,"Error insertion position is negative!");
    if((size_t)idx.l > p->size)
          return quickErr(ValueError,"Error insertion position out of range!");
    if(val.type=='j')
    {
      ZList* sublist = (ZList*)val.ptr;
      ZList_insertList(p,idx.l,sublist);
    }
    else
    {
      ZList_insert(p,idx.l,val);
    }
    ZObject ret = nil;
    return ret;
  }
  else
    return quickErr(ArgumentError,"Error method insert() takes 3 arguments!");
}
ZObject ERASE(ZObject* args,int32_t argc)
{
  if(args[0].type=='j')
  {
    if(argc!=2 && argc!=3)
      return quickErr(ArgumentError,"Error erase() takes 2 or 3 arguments!");
    ZList* p = (ZList*)args[0].ptr;
    ZObject idx1 = args[1];
    ZObject idx2;
    if(argc==3)
     idx2 = args[2];
    else
      idx2 = idx1;
    if(idx1.type!='i' && idx1.type!='l')
        return quickErr(TypeError,"Error method erase() expects an integer argument for start!");
    if(idx2.type!='i' && idx2.type!='l')
        return quickErr(TypeError,"Error method erase() expects an integer argument for end!");
    PromoteType(idx1,'l');
    PromoteType(idx2,'l');
    if(idx1.l < 0 || idx2.l < 0)
        return quickErr(ValueError,"Error index is negative!");
    if((size_t)idx1.l >= p->size || (size_t)idx2.l >= p->size)
        return quickErr(ValueError,"Error index out of range!");
    ZList_eraseRange(p,idx1.l,idx2.l);
    ZObject ret = nil;
    return ret;
  }
  else if(args[0].type=='a')
  {
     if(argc!=2)
       return quickErr(ArgumentError,"Error dictionary method erase() takes 2 arguments!");
    ZDict* d = (ZDict*)args[0].ptr;
    ZObject key = args[1];
    if(key.type!='i' && key.type!='l' && key.type!='f' && key.type!='s' && key.type!='m' && key.type!='b')
      return quickErr(TypeError,"Error key of type "+fullform(key.type)+" not allowed.");
    bool b = ZDict_erase(d,key);
    ZObject ret;
    ret.type = Z_BOOL;
    ret.i = b;
    return ret;
  }
  else if(args[0].type == 'c')
  {
    if(argc!=2 && argc!=3)
      return quickErr(ArgumentError,"Error erase() takes 2 or 3 arguments!");
    ZByteArr* p = (ZByteArr*)args[0].ptr;
    ZObject idx1 = args[1];
    ZObject idx2;
    if(argc==3)
     idx2 = args[2];
    else
      idx2 = idx1;
    if(idx1.type!='i' && idx1.type!='l')
        return quickErr(TypeError,"Error method erase() expects an integer argument for start!");
    if(idx2.type!='i' && idx2.type!='l')
        return quickErr(TypeError,"Error method erase() expects an integer argument for end!");
    PromoteType(idx1,'l');
    PromoteType(idx2,'l');
    if(idx1.l < 0 || idx2.l < 0)
        return quickErr(ValueError,"Error index is negative!");
    if((size_t)idx1.l >= p->size || (size_t)idx2.l >= p->size)
        return quickErr(ValueError,"Error index out of range!");
    ZByteArr_eraseRange(p,idx1.l,idx2.l);
    ZObject ret = nil;
    return ret;
  }
  else if(args[0].type == Z_STR)
  {
    if(argc!=2 && argc!=3)
      return quickErr(ArgumentError,"Error erase() takes 2 or 3 arguments!");
    ZStr* p = (ZStr*)args[0].ptr;
    ZObject idx1 = args[1];
    ZObject idx2;
    if(argc==3)
     idx2 = args[2];
    else
      idx2 = idx1;
    if(idx1.type!='i' && idx1.type!='l')
        return quickErr(TypeError,"Error method erase() expects an integer argument for start!");
    if(idx2.type!='i' && idx2.type!='l')
        return quickErr(TypeError,"Error method erase() expects an integer argument for end!");
    PromoteType(idx1,'l');
    PromoteType(idx2,'l');
    if(idx1.l < 0 || idx2.l < 0)
        return quickErr(ValueError,"Error index is negative!");
    if((size_t)idx1.l >= p->len || (size_t)idx2.l >= p->len)
        return quickErr(ValueError,"Error index out of range!");
    string s = p->val;
    s.erase(s.begin()+idx1.l,s.begin()+idx2.l+1);

    ZStr* res = vm_allocString(s.length());
    strcpy(res->val,s.c_str());
    return ZObjFromStrPtr(res);
  }
  else
  {
      return quickErr(NameError,"Error type "+fullform(args[0].type)+" has no member named erase.");
  }
}
ZObject asMap(ZObject* args,int32_t argc)
{
    if(args[0].type!='j')
      return quickErr(NameError,"Error type "+fullform(args[0].type)+" has no member named asMap()");
    if(argc!=1)
      return quickErr(ArgumentError,"Error list member asMap() takes 0 arguments!");
    ZList l = *(ZList*)args[0].ptr;
    ZObject x;
    x.type = 'l';
    size_t i = 0;
    ZDict* d = vm_allocDict();

    for(;i<l.size;i++)
    {
       x.l = i;
       ZDict_emplace(d,x,l.arr[i]);
    }
    ZObject ret = nil;
    ret.type = 'a';
    ret.ptr = (void*)d;
    return ret;
}
ZObject ASLIST(ZObject* args,int32_t argc)
{
    if(args[0].type!='a')
      return quickErr(NameError,"Error type "+fullform(args[0].type)+" has no member named asList()");
    if(argc!=1)
      return quickErr(ArgumentError,"Error dictionary member asList() takes 0 arguments!");
    ZDict d = *(ZDict*)args[0].ptr;
    ZList* list = vm_allocList();
    for(size_t i=0;i<d.capacity;i++)
    {
      if(d.table[i].stat != OCCUPIED)
        continue;
      ZList* sub = vm_allocList();
      ZList_push(sub,d.table[i].key);
      ZList_push(sub,d.table[i].val);
      
      ZObject x;
      x.type = 'j';
      x.ptr = (void*)sub;
      ZList_push(list,x);
    }
    ZObject ret = nil;
    ret.type = 'j';
    ret.ptr = (void*)list;
    return ret;
}
ZObject REVERSE_METHOD(ZObject* args,int32_t argc)
{
  if(args[0].type!='j' && args[0].type != Z_STR)
         return quickErr(NameError,"Error type "+fullform(args[0].type)+" has no method named reverse()");
  if(argc!=1)
    return quickErr(ArgumentError,"Error method reverse() takes 0 arguments!");
  if(args[0].type == Z_STR)
  {
    ZStr* str = (ZStr*)args[0].ptr;
    ZStr* p = vm_allocString(str->len);
    size_t i = 0;
    while(i < str->len)
    {
      p->val[i] = str->val[str->len - i - 1];
      i+=1;
    }
    return ZObjFromStrPtr(p);
  }
  else
  {
    ZList* p = (ZList*)args[0].ptr;
    if(p -> size == 0)
      return nil;
    size_t l = 0;
    size_t h = p->size - 1;

    while(l < h)
    {
      ZObject tmp = p->arr[l];
      p->arr[l] = p->arr[h];
      p->arr[h] = tmp;
      l++;
      h--;
    }
  }
  ZObject ret = nil;
  return ret;
}
ZObject EMPLACE(ZObject* args,int32_t argc)
{
  if(args[0].type!='a')
         return quickErr(NameError,"Error type "+fullform(args[0].type)+" has no method named emplace()");
  if(argc!=3)
    return quickErr(ArgumentError,"Error method emplace() takes 2 arguments!");
  ZDict* p = (ZDict*)args[0].ptr;
  ZObject& key = args[1];
  if(key.type!='i' && key.type!='l' && key.type!='f' && key.type!='s' && key.type!='m' && key.type!='b')
    return quickErr(TypeError,"Error key of type "+fullform(key.type)+" not allowed.");

  ZDict_emplace(p,key,args[2]);
  ZObject ret = nil;
  return ret;

}
ZObject HASKEY(ZObject* args,int32_t argc)
{
  if(args[0].type!='a')
         return quickErr(NameError,"Error type "+fullform(args[0].type)+" has no method named hasKey()");
  if(argc!=2)
    return quickErr(ArgumentError,"Error method hasKey() takes 1 argument!");
  ZDict* p = (ZDict*)args[0].ptr;

  ZObject ret = nil;
  ZObject tmp;
  ret.type= 'b';
  ret.i = ZDict_get(p,args[1],&tmp);

  return ret;

}
ZObject UNPACK(ZObject* args,int32_t argc)
{
  if(args[0].type!=Z_BYTEARR)
    return quickErr(NameError,"Error object has no member unpack()");
  if(argc!=2)
    return quickErr(ArgumentError,"Error unpack() takes 2 arguments!");
  if(args[1].type!=Z_STR)
    return quickErr(TypeError,"Error unpack() takes a string argument!");
  auto arr = (ZByteArr*)args[0].ptr;
  string pattern = ((ZStr*)args[1].ptr)->val;
  size_t k = 0;
  int32_t int32;
  int64_t int64;
  double d;
  bool b;
  string str;
  ZList* lp = vm_allocList();
  ZList* res = lp;
  for(size_t i=0;i<pattern.length();i++)
  {
    char ch = pattern[i];
    if(ch == 'i')
    {
      if(k+3 >= arr->size)
         return quickErr(ValueError,"Error making element "+to_string(res->size)+" from bytearray(not enough bytes)!");
      memcpy(&int32,arr->arr+k,4);
      ZList_push(res,ZObjFromInt(int32));
      k+=4;
    }
    else if(ch == 'l')
    {
      if(k+7 >= arr->size)
         return quickErr(ValueError,"Error making element "+to_string(res->size)+" from bytearray(not enough bytes)!");
      memcpy(&int64,arr->arr+k,8);
      ZList_push(res,ZObjFromInt(int64));      
      k+=8;
    }
    else if(ch == 'f')
    {
      if(k+7 >= arr->size)
         return quickErr(ValueError,"Error making element "+to_string(res->size)+" from bytearray(not enough bytes)!");
      memcpy(&d,arr->arr+k,8);
      ZList_push(res,ZObjFromDouble(d));
      k+=8;
    }
    else if(ch == 'b')
    {
      if(k >= arr->size)
         return quickErr(ValueError,"Error making element "+to_string(res->size)+" from bytearray(not enough bytes)!");
      memcpy(&b,arr->arr+k,1);
      ZList_push(res,ZObjFromBool(b));
      k+=1;
    }
    else if(ch == 's')
    {
      if(i+1>=pattern.length())
        return quickErr(ValueError,"Error in pattern,required length after 's' ");
      if(!isdigit(pattern[i+1]))
        return quickErr(ValueError,"Error in pattern,required length after 's' ");
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
        return quickErr(OverflowError,"Error given string length too large!");
      long long int len = atoll(l.c_str());
      if((long long int)k+len-1 >= (long long int)arr->size)
         return quickErr(ValueError,"Error making element "+to_string(res->size)+" from bytearray(not enough bytes)!");
      str = "";
      size_t f = k+(size_t)len;
      for(;k!=f;++k)
      {
        char c = arr->arr[k];
        str.push_back(c);
      }
      auto p = vm_allocString(str.length());
      strcpy(p->val,str.c_str());
      ZList_push(res,ZObjFromStrPtr(p));
    }
    else
      return quickErr(ValueError,"Error invalid char in pattern string!"); 
  }
  return ZObjFromList(lp);
}
//String methods
ZObject SUBSTR_METHOD(ZObject* args,int32_t argc)
{
  if(args[0].type != Z_STR)
    return quickErr(NameError,"Error type "+fullform(args[0].type)+" has no member named substr");
  if(argc!=3)
    return quickErr(ArgumentError,"Error str.substr() takes 2 arguments");
  if(args[1].type!='i' && args[1].type!='l')
    return quickErr(TypeError,"Error first argument of str.substr() should be an integer");
  if(args[2].type!='i' && args[2].type!='l')
    return quickErr(TypeError,"Error second argument of str.substr() should be an integer");
  
  PromoteType(args[1],'l');
  PromoteType(args[2],'l');
  ZStr* data = (ZStr*)args[0].ptr;
  if(args[1].l < 0 || args[2].l < 0)
  {
    static ZStr empty;
    empty.len = 0;
    static char arr[] = "";
    empty.val = arr;
    return ZObjFromStrPtr(&empty);
  }
  else
  {
    string s = substr((int32_t)args[1].l,(int32_t)args[2].l,data->val);
    ZStr* res = vm_allocString(s.length());
    strcpy(res->val,s.c_str());
    return ZObjFromStrPtr(res);
  }
  return nil; //stupid MSVC gives warning, fuck you microsoft 
}
ZObject REPLACE_METHOD(ZObject* args,int32_t argc)
{
  if(args[0].type != Z_STR)
    return quickErr(NameError,"Error type "+fullform(args[0].type)+" has no member named replace");
  if(argc==3)
    {
        if(args[1].type!='s')
            return quickErr(TypeError,"Error first argument given to replace() must be a string!");
        if(args[2].type!='s')
            return quickErr(TypeError,"Error second argument given to replace() must be a string!");
      
        string c = ((ZStr*)args[0].ptr)->val;
        string a = ((ZStr*)args[1].ptr) -> val;
        string b = ((ZStr*)args[2].ptr) -> val;
        string s = replace_all(a,b,c); // can be optimized but not today
        ZStr* z = vm_allocString(c.length());
        strcpy(z->val,s.c_str());
        return ZObjFromStrPtr(z);
    }
    else
    {
        return quickErr(ArgumentError,"Error method replace() takes two arguments\n");
    }
    
}
ZObject REPLACE_ONCE_METHOD(ZObject* args,int32_t argc)
{
  if(args[0].type != Z_STR)
    return quickErr(NameError,"Error type "+fullform(args[0].type)+" has no member named replace");
  if(argc==3)
    {
        if(args[1].type!='s')
            return quickErr(TypeError,"Error first argument given to replace() must be a string!");
        if(args[2].type!='s')
            return quickErr(TypeError,"Error second argument given to replace() must be a string!");
      
        string a = ((ZStr*)args[1].ptr) -> val;
        string b = ((ZStr*)args[2].ptr) -> val;        
        string c = ( ((ZStr*)args[0].ptr) )->val;
        string s = replace(a,b,c);
        ZStr* z = vm_allocString(c.length());
        strcpy(z->val,s.c_str());
        return ZObjFromStrPtr(z);
        
    }
    else
    {
        return quickErr(ArgumentError,"Error method replace() takes two arguments\n");
    }
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
 // funcs.emplace("ListFromBytes",&makeList);
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
  methods.emplace("erase",&ERASE);
  methods.emplace("reverse",&REVERSE_METHOD);
  methods.emplace("emplace",&EMPLACE);
  methods.emplace("hasKey",&HASKEY);
  methods.emplace("asList",&ASLIST);
  methods.emplace("unpack",&UNPACK);
  methods.emplace("substr",&SUBSTR_METHOD);
  methods.emplace("replace",REPLACE_METHOD);
  methods.emplace("replace_once",REPLACE_ONCE_METHOD);

}
ZObject callmethod(string name,ZObject* args,int32_t argc)
{
     if(methods.find(name)==methods.end())
       return quickErr(NameError,"Error "+fullform(args[0].type)+" type has no method named "+name+"()");
     return methods[name](args,argc);
}
bool function_exists(string name)
{
  if(funcs.find(name)!=funcs.end())
    return true;
  return false;
}
