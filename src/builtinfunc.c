#include "builtinfunc.h"
#include "builtin-map.h"
#include "strmap.h"
#include "vm.h"
#include "zbytearray.h"
#include "zobject.h"
#include "coroutineobj.h"
#include "klassobject.h"
#include "zstr.h"
#include "zdict.h"
#include "zfileobj.h"
#include "dyn-str.h"
#include "module.h"
#include "overflow.h"
#include "misc.h"
#include "convo.h"
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <unistd.h>
#include <time.h>

zobject zobj_from_str(const char* str);// makes deep copy of str
zobject z_err(zclass* errKlass,const char* des);
zlist* vm_alloc_zlist();
zbytearr* vm_alloc_zbytearr();
uint8_t* vm_alloc_raw(size_t);
zstr* vm_alloc_zstr(size_t);
zclass* vm_alloc_zclass();
zmodule* vm_alloc_zmodule();
zclass_object* vm_alloc_zclassobj(zclass*);
Coroutine* vm_alloc_coro_obj();//allocates coroutine object
zfun* vm_alloc_zfun();
zfun* vm_alloc_coro(); //coroutine can be represented by fun_object
zfile * vm_alloc_zfile();
zdict* vm_alloc_zdict();
znativefun* vm_alloc_znativefun();
//callObject also behaves as a kind of try/catch since v0.3.1
bool vm_call_object(zobject*,zobject*,int,zobject*);
void vm_mark_important(void* mem);
void vm_unmark_important(void* mem);
void PromoteType(zobject*,char);
//
//Error classes (defined in vm.cpp)
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

bmap funcs;

const char* fullform(char);

static zobject nil;
//zobject z_err(zclass* k,string s)
//{
//  return z_err(k,s.c_str());
//}
char error_buffer[100];
bool validateArgTypes(const char* name,const char* e,zobject* args,int32_t argc,zobject* x)
{
    size_t len = strlen(e);
    if(strlen(e)!=(size_t)argc)
    {
      snprintf(error_buffer,100,"Error %s() takes %zu arguments!",name,strlen(e));
      *x = z_err(ArgumentError,error_buffer);
      return false;
    }
    for(int32_t f=1;f<=argc;f++)
    {
        zobject k = args[f-1];
        if(k.type!=e[f-1])
        {
          snprintf(error_buffer,100, "Error argument %d of %s() should be of type %s",f,name,fullform(e[f-1]));
          *x = z_err(TypeError,error_buffer);
          return false;
        }
        
    }
    return true;
}
zobject Z_ISALPHA(zobject* args,int32_t argc)
{
  if(argc!=1)
    return z_err(ArgumentError,"Error isalpha() takes one argument!");
  if(args[0].type!=Z_STR)
    return z_err(TypeError,"Error isalpha() takes a string argument!");
  zstr* s = AS_STR(args[0]);
  zobject ret = nil;
  ret.type = 'b';
  ret.i = 0;
  const char* p = s->val;
  char ch;
  while((ch = *p++))
  {
    if(!isalpha(ch))
      return ret;
  }
  ret.i = 1;
  return ret;
}

zobject ASCII(zobject* args,int32_t argc)
{
  if(argc!=1)
    return z_err(ArgumentError,"Error ascii() takes one argument!");
  if(args[0].type!='s')
    return z_err(TypeError,"Error ascii() takes a string argument!");
  zstr* s = AS_STR(args[0]);
  if(s->len != 1)
      return z_err(ValueError,"Error ascii() takes a string argument of length 1!");
 
  zobject ret = nil;
  ret.type = 'i';
  ret.i = s->val[0];
  return ret;
}
zobject TOCHAR(zobject* args,int32_t argc)
{
  if(argc!=1)
    return z_err(ArgumentError,"Error char() takes one argument!");
  if(args[0].type!='i')
    return z_err(TypeError,"Error char() takes an integer argument!");
  char ch = (char)args[0].i;
  zstr* p = vm_alloc_zstr(1);
  p->val[0] = ch;
  return zobj_from_str_ptr(p);
}
//string unescape(string);
void print_zdict(zdict* l,ptr_vector* seen)
{
    if(ptr_vector_search(seen,l) != -1)
    {
        printf("{...}");
        return;
    }
    ptr_vector_push(seen,l);
    size_t k = l->size;
    size_t m = 0;
    printf("{");
    for(size_t i=0;i<l->capacity;i++)
    {
        if(l->table[i].stat != OCCUPIED)
            continue;
        zobject key = l->table[i].key;
        zobject val = l->table[i].val;
        if(key.type=='j')
            print_zlist((zlist*)key.ptr,seen);
        else if(key.type=='a')
            print_zdict((zdict*)key.ptr,seen);
        else if(key.type=='s')
            printf("\"%s\"",(AS_STR(key)->val)); // DO UNESCAPE
        else
            printf("%s",zobject_to_str(key));
        printf(" : ");
    //
        if(val.type=='j')
            print_zlist((zlist*)val.ptr,seen);
        else if(val.type=='a')
            print_zdict((zdict*)val.ptr,seen);
        else if(val.type=='s')
            printf("\"%s\"",(AS_STR(val)->val)); // DO UNESCAPE
        else
            printf("%s",zobject_to_str(val));
        if(m!=k-1)
            printf(",");
        m+=1;
 
    }
  printf("}");
}

void print_zlist(zlist* l,ptr_vector* seen)
{
  if(ptr_vector_search(seen,l) != -1)
  {
    printf("[...]");
    return;
  }
  ptr_vector_push(seen,l);
  size_t k = l->size;
  zobject val;
  printf("[");
  for(size_t i=0;i<k;i+=1)
  {
    val = l->arr[i];
    if(val.type=='j')
      print_zlist((zlist*)val.ptr,seen);
    else if(val.type=='a')
      print_zdict((zdict*)val.ptr,seen);
    else if(val.type=='s')
    {
      printf("\"%s\"",(((zstr*)val.ptr)->val)); // DO UNESCAPE
    }
    else
    {
      printf("%s",zobject_to_str(val));
    }
    if(i!=k-1)
      printf(",");
  }
  printf("]");
}

void print_zbytearray(zbytearr* arr)
{
    size_t len = arr->size;
    printf("bytearr([");
    char buffer[5];
    for(size_t i=0;i<len;i++)
    {
        printf("0x%x",arr->arr[i]);
        if(i!=len-1)
            printf(",");
    }
    printf("])");
}
zobject print(zobject* args,int32_t argc)
{
    int32_t k = 0;
    while(k<argc)
    {
        /*if(args[k].type=='j')
           printList((zlist*)args[k].ptr);
        else if(args[k].type=='a')
           printzdict((zdict*)args[k].ptr);
        else if(args[k].type==Z_BYTEARR)
          printByteArray((zbytearr*)args[k].ptr);
        else */if(args[k].type == Z_OBJ)
        {
          zclass_object* ki = (zclass_object*)args[k].ptr;
          zobject r,ret;

          if(StrMap_get(&(ki->members),"__print__",&r))
          {
            zobject args[] = {zobj_from_classobj(ki)};
            vm_call_object(&r, args, 1,&ret);
          }
          else
            ;//printf("%s",zobjectToStr(args[k]).c_str());
        }
        else
          printf("%s",zobject_to_str(args[k]));
        k+=1;
    }
    zobject ret = nil;
    ret.type = 'n';
    return ret;
}
zobject PRINTF(zobject* args,int32_t argc)
{
    if(argc < 1)
      return z_err(ArgumentError,"At least one argument needed!");
    if(args[0].type!='s')
      return z_err(TypeError,"Argument 1 must be a string!");
    zstr* format = (zstr*)args[0].ptr;
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
            return z_err(ArgumentError,"String format requires more arguments!");
          /*if(args[j].type=='j')
           printList((zlist*)args[j].ptr);
          else if(args[j].type=='a')
           printzdict((zdict*)args[j].ptr);
          else if(args[j].type==Z_BYTEARR)
          printByteArray((zbytearr*)args[j].ptr);
          else
           printf("%s",zobjectToStr(args[j]).c_str());*/
          j+=1;
        }
        else
          printf("%c",format->val[k]);
        k+=1;
    }
    zobject ret = nil;
    ret.type = 'n';
    return ret;
}
zobject FORMAT(zobject* args,int32_t argc)
{
    if(argc < 1)
      return z_err(ArgumentError,"At least one argument needed!");
    if(args[0].type!=Z_STR)
      return z_err(TypeError,"Argument 1 must be a string!");
    zstr* format = (zstr*)args[0].ptr;
    int32_t k = 0;
    int32_t l = format->len;
    int32_t j = 1;

 //   dyn_str p;
  //  dyn_str_init(&p);

    while(k<l)
    {
        if(format->val[k] == '%')
        {
          if(k+1 < l && format->val[k+1] == '%')
          {
            //p+="%%";
            k+=2;
            continue;
          }
          if(j>=argc)
            return z_err(ArgumentError,"String format requires more arguments!");
          //p += zobjectToStr(args[j]);
          j+=1;
        }
        else
          ;//p+=format->val[k];
        k+=1;
    }
    zstr* s ;//= vm_alloc_zstr(p.length());
   // strcpy(s->val,p.c_str());
    return zobj_from_str_ptr(s);
}

zobject println(zobject* args,int32_t argc)
{
    int32_t k = 0;
    ptr_vector seen;
    ptr_vector_init(&seen);
    while(k<argc)
    {
        seen.size = 0;
        if(args[k].type=='j')
          print_zlist((zlist*)args[k].ptr,&seen);
        else if(args[k].type=='a')
          print_zdict((zdict*)args[k].ptr,&seen);
        else if(args[k].type==Z_BYTEARR)
          print_zbytearray((zbytearr*)args[k].ptr);
        else
        if(args[k].type == Z_OBJ)
        {
          zclass_object* ki = (zclass_object*)args[k].ptr;
          zobject r,ret;
          if(StrMap_get(&(ki->members),"__print__",&r))
          {
            zobject args[] = {zobj_from_classobj(ki)};
            vm_call_object(&r, args, 1,&ret);
          }
          else
            printf("%s",zobject_to_str(args[k]));
        }
        else
            printf("%s",zobject_to_str(args[k]));
        k+=1;
    }
    puts("");
    zobject ret = nil;
    return ret;
}

zobject input(zobject* args,int32_t argc)
{
    if(argc!=0 && argc!=1)
    {
        return z_err(ArgumentError,"Error input() takes 1 or 0 arguments!");
    }
    if(argc==1)
    {
      if(args[0].type!='s')
        return z_err(TypeError,"Error input() takes a string argument!");
      char* prompt = ((zstr*)args[0].ptr)->val;
      printf("%s",prompt);
    }
    //string s;
    char ch;
    while(true)
    {
        ch = fgetc(stdin);
        if(ch=='\n')
          break;
        //s+=ch;
    }
    zstr* p ;//= vm_alloc_zstr(s.length());
   // strcpy(p->val,s.c_str());
    return zobj_from_str_ptr(p);
}
zobject TYPEOF(zobject* args,int32_t argc)
{
  if(argc!=1)
  {
      return z_err(TypeError,"Error typeof() takes one argument only!");
  }
  const char* fullform(char);
  const char* s = fullform(args[0].type);
  zstr* p = vm_alloc_zstr(strlen(s));
  strcpy(p->val,s);
  return zobj_from_str_ptr(p);
}
zobject isInstanceOf(zobject* args,int32_t argc)
{
  if(argc!=2)
    return z_err(ArgumentError,"Error function isInstanceOf() takes 2 arguments!");
  if(args[1].type!='v')
    return z_err(TypeError,"Error second argument to  isInstanceOf() should be a class!");
  zobject ret = nil;
  ret.type = 'b';
  if(args[0].type!='o')
  {
    ret.i = 0;
    return ret;
  }
  zclass_object* obj = (zclass_object*)args[0].ptr;
  zclass* k = (zclass*)args[1].ptr;
  ret.i = obj->_klass == k;
  return ret;
}
zobject LEN(zobject* args,int32_t argc)
{
    if(argc!=1)
        return z_err(ArgumentError,"Error len() takes one argument!");

    zobject ret = nil;
    ret.type = Z_INT64;
    if(args[0].type=='s')
    {
      zstr* s = (zstr*)args[0].ptr;
      ret.l = s->len;
    }
    else if(args[0].type=='j')
        ret.l = ((zlist*)args[0].ptr)->size;
    else if(args[0].type=='a')
        ret.l = ((zdict*)args[0].ptr)->size;
    else if(args[0].type == 'c')
        ret.l = ((zbytearr*)args[0].ptr)->size;
    else
    {
      snprintf(error_buffer,100,"Error len() unsupported for type %s",fullform(args[0].type));
        return z_err(TypeError,error_buffer);
    }
    return ret;
}
//////////
zobject OPEN(zobject* args,int32_t argc)
{
    const char* patt = "ss";
    zobject ret = nil;
    if(!validateArgTypes("open",patt,args,argc,&ret))
      return ret;
    const char* filename = AS_STR(args[0])->val;
    const char* mode = AS_STR(args[1])->val;
    if(strcmp(mode,"r") && strcmp(mode,"w") && strcmp(mode,"a") && strcmp(mode,"rw") && strcmp(mode,"rw+") && strcmp(mode,"rb") && strcmp(mode,"wb"))
        return z_err(ValueError,"Error unknown fileopen mode");
    FILE* fp = fopen(filename, mode);
    if(!fp)
        return z_err(FileOpenError,strerror(errno));
    zfile* f = vm_alloc_zfile();
    f->fp = fp;
    f->open = true;
    ret.type = 'u';
    ret.ptr = (void*)f;
    return ret;
}
zobject READ(zobject* args,int32_t argc)
{
    if(argc!=2 && argc!=1)
        return z_err(ArgumentError,"Error read() function takes one or two arguments!");
    if(args[0].type!='u')
        return z_err(TypeError,"Error read() needs a file stream to read from!");
    char delim = EOF;
    if(argc==2)
    {
      if(args[1].type!='s')
        return z_err(TypeError,"Error argument 2 to read() function should be of type string!");
      zstr* l = AS_STR(args[1]);
      if(l->len!=1)
        return z_err(ValueError,"Error optional delimeter argument to read() should be string of length 1");
      delim = l->val[0];  
    }
    zfile fobj = *(zfile*)args[0].ptr;
    if(!fobj.open)
      return z_err(ValueError,"Error the file stream is closed!");
    FILE* fp = fobj.fp;
    if(!fp)
        return z_err(FileIOError,"Error can't read from a closed file stream!");
    char ch;
    dyn_str p;
    dyn_str_init(&p);
    while((ch = fgetc(fp))!=EOF)
    {
        if(ch==delim)
          break;
        dyn_str_push(&p,ch);
    }
    zstr* s = vm_alloc_zstr(p.length);
    strcpy(s->val,p.arr);
    return zobj_from_str_ptr(s);
}
zobject CLOSE(zobject* args,int32_t argc)
{
    if(argc!=1)
        return z_err(ArgumentError,"Error close() takes 1 argument!");
  //  printf("args[0].type = %c\n",args[0].type);
    if(args[0].type!='u')
        return z_err(TypeError,"Error close() takes a file stream as an argument!");
    zfile* fobj = (zfile*)args[0].ptr;
    if(!fobj->open)
    {
      return z_err(ValueError,"Error file already closed!");
    }
    FILE* f = fobj->fp;
    if(f==stdin || f==stdout)
      return z_err(ValueError,"Are you nuts?You should not close stdin or stdout!");
    fclose(f);
    zobject ret = nil;
    ret.type='n';
    fobj->open = false;
    return ret;
}
zobject FFLUSH(zobject* args,int32_t argc)
{
    if(argc!=1)
        return z_err(ArgumentError,"Error fflush() takes 1 argument!");
    if(args[0].type!='u')
        return z_err(TypeError,"Error fflush() takes a file stream as an argument!");
    zfile* fobj = (zfile*)args[0].ptr;
    if(!fobj->open)
    {
     // return z_err(ValueError,"Error file is closed!");
    }
    FILE* f = fobj->fp;
    fflush(f);
    zobject ret = nil;
    ret.type='n';
    fobj->open = false;
    return ret;
}
zobject RAND(zobject* args,int32_t argc)
{
    if(argc!=0)
        return z_err(ArgumentError,"Error rand() takes 0 arguments!");
    zobject ret = nil;
    ret.i = rand();
    ret.type= 'i';
    return ret;
}
//////////////
zobject BYTEARRAY(zobject* args,int32_t argc)
{
  zobject ret = nil;
  if(argc==0)
  {
    zbytearr* arr = vm_alloc_zbytearr();
    ret.type = Z_BYTEARR;
    ret.ptr = (void*)arr;
    return ret;
  }
  else if(argc == 1)
  {
    if(args[0].type != Z_LIST)
    {
      return z_err(TypeError,"Error bytearray() takes a list argument!");
    }
    zlist* p = (zlist*)args[0].ptr;
    size_t len = p->size;
    zbytearr* arr = vm_alloc_zbytearr();
    ret.type = Z_BYTEARR;
    ret.ptr = (void*)arr;
    for(size_t i=0;i<len;i++)
    {
      if(p->arr[i].type!=Z_BYTE)
        return z_err(TypeError,"Error argument list given to bytearray() must contain only bytes!");
      zbytearr_push(arr,p->arr[i].i);
    }
    return ret;
  }
  return z_err(ArgumentError,"Error bytearray() takes 0 or 1 arguments!");
}

zobject WRITE(zobject* args,int32_t argc)
{
  //printf("writing %s\n",args[0].s.c_str());
  if(argc!=2)
    return z_err(ArgumentError,"Error write() takes two arguments!");
  const char* patt = "su";
  zobject ret = nil;
  if(!validateArgTypes("write",patt,args,argc,&ret))
    return ret;
  zstr* data = AS_STR(args[0]);
  zfile* p = (zfile*)args[1].ptr;
  FILE* fp = p->fp;
  if(fputs(data->val,fp)==EOF)
    return z_err(FileIOError,strerror(errno));
  ret.type = Z_NIL;
  return ret;
}
zobject EXIT(zobject* args,int32_t argc)
{
    int ret =  0;
    if(argc == 1)
    {
      if(args[0].type!=Z_INT)
        return z_err(TypeError,"Integer argument required!");
      ret = args[0].i;
    }
    else if(argc == 0);
    else
        return z_err(ArgumentError,"Error exit() takes either 0 or 1 argument!");
    exit(ret);
}
////////////////
zobject REVERSE(zobject* args,int32_t argc)
{
    if(argc!=1)
        return z_err(ArgumentError,"Error reverse() takes 1 argument!");
    zobject q  = args[0];
    if(q.type!='s' && q.type!='j')
        return z_err(TypeError,"Error reverse() takes a string or list argument!");
    if(q.type=='s')
    {
        zstr* data = AS_STR(q);
        zstr* l = vm_alloc_zstr(data->len);
        size_t i = 0;
        for(int32_t k=data->len-1;k>=0;k--)
        {
            l->val[i++] = data->val[k];
        }
        return zobj_from_str_ptr(l);
    }
    zlist* l = vm_alloc_zlist();
    zlist* currList = (zlist*)q.ptr;
    for(int32_t k =currList->size-1;k>=0;k--)
    {
        zlist_push(l,currList->arr[k]);
    }
    zobject ret = nil;// = l;
    ret.type = 'j';
    ret.ptr = (void*)l;
    return ret;
}///////
/////////
zobject BYTES(zobject* args,int32_t argc)
{
    if(argc!=1)
      return z_err(ArgumentError,"Error bytes() takes one argument!");
    zbytearr* p = vm_alloc_zbytearr();
    zobject e = args[0];
    zobject ret = nil;
    ret.type = Z_BYTEARR;
    ret.ptr = (void*)p;
    if(e.type=='i')
    {
      zbytearr_resize(p,4);
      memcpy(p->arr,&e.i,4);
      return ret;
    }
    else if(e.type=='l')
    {
      zbytearr_resize(p,8);
      memcpy(p->arr,&e.l,8);
      return ret;
    }
    else if(e.type=='f')
    {
      zbytearr_resize(p,4);
      memcpy(p->arr,&e.f,4);
      return ret;
    }
    else if(e.type=='s')
    {
       zstr* s = AS_STR(e);
       char* ptr = s->val;
       char ch;
       while((ch = *ptr++))
         zbytearr_push(p,(uint8_t)ch);
       return ret;
    }
    else if(e.type=='b')
    {
      if(e.i)
        zbytearr_push(p,1);
      else
        zbytearr_push(p,0);
      return ret;
    }
    else if(e.type == Z_BYTE)
    {
      zbytearr_push(p,e.i);
      return ret;
    }
    else
    {
        snprintf(error_buffer,100,"Cannot convert type %s to bytes",fullform(e.type));
        return z_err(TypeError,error_buffer);
    }
}
///////////////
zobject OBJINFO(zobject* args,int32_t argc)
{
    if(argc!=1)
      return z_err(ArgumentError,"Error obj_info() takes 1 argument!");
    if( args[0].type=='o')
    {
      zclass_object* k = (zclass_object*)args[0].ptr;
      for(size_t idx = 0; idx < k->members.capacity;idx++)
      {
        if(k->members.table[idx].stat != SM_OCCUPIED)
          continue;
        SM_Slot e = k->members.table[idx];
        if(e.val.type == Z_POINTER)
          printf("%s: %p\n",e.key,e.val.ptr);
        else
          printf("%s: %s\n",e.key,fullform(e.val.type));
          
      }
      for(size_t idx = 0; idx < k->privateMembers.capacity;idx++)
      {
        if(k->privateMembers.table[idx].stat != SM_OCCUPIED)
          continue;
        SM_Slot e = k->privateMembers.table[idx];
        printf("%s: %s\n",e.key,fullform(e.val.type));
          
      }
      zobject ret = nil;
      return ret;
    }
    else
    {
        return z_err(TypeError,"Error argument is not an object of any class!");
    }
}
zobject moduleInfo(zobject* args,int32_t argc)
{
  if(argc!=1)
    return z_err(ArgumentError,"Error moduleInfo takes 1 argument!");
  if(args[0].type!=Z_MODULE)
    return z_err(TypeError,"Argument must be a module object!");
  zmodule* mod = (zmodule*)args[0].ptr;
  printf("Module %s\n",mod->name);
  printf("------------\n");
  for(size_t idx=0;idx<mod->members.capacity;idx++)
  {
    if(mod->members.table[idx].stat != SM_OCCUPIED)
      continue;
    SM_Slot e = mod->members.table[idx];
    printf("%s  %s\n",e.key,fullform(e.val.type));

  }
  zobject ret = nil;
  return ret;
}
zobject TOLOWER(zobject* args,int32_t n)
{
  if(n!=1)
    return z_err(ArgumentError,"1 argument required!");
  if(args[0].type != Z_STR)
    return z_err(TypeError,"Argument 1 must be a string!");
  zstr* str = AS_STR(args[0]);
  zstr* lower = vm_alloc_zstr(str->len);
  char* p = str->val;
  size_t k =0;
  while(k < str->len)
  {
    lower->val[k] = tolower(str->val[k]);
    k++;
  }
  return zobj_from_str_ptr(lower);
}
zobject TOUPPER(zobject* args,int32_t n)
{
  if(n!=1)
    return z_err(ArgumentError,"1 argument required!");
  if(args[0].type != Z_STR)
    return z_err(TypeError,"Argument 1 must be a string!");
  zstr* str = AS_STR(args[0]);
  zstr* upper = vm_alloc_zstr(str->len);
  char* p = str->val;
  size_t k =0;
  while(k < str->len)
  {
    upper->val[k] = toupper(str->val[k]);
    k++;
  }
  return zobj_from_str_ptr(upper);
}

/*
--Deprecated--

zobject makeList(zobject* args,int32_t argc)
{
        zobject ret = nil;
        if(!validateArgTypes("makeList","js",args,argc,ret))
          return ret;
        string& pattern = *(zstr*)args[1].ptr;
        size_t k = 0;
        zlist res;
        size_t i = 0;
        
        
        zlist currList = *(zlist*)args[0].ptr;
        while(k<currList.size)
        {
            if(i>pattern.length()-1)
            {
               return z_err(ValueError,"Error no pattern specified for the remaining bytes!");
            }
            zobject l = currList.arr[k];
            if(l.type!='m')
            {
                return z_err(ValueError,"Error list should only contain bytes!");
            }
            int32_t b = l.i;
            if(pattern[i]=='i')
            {
               FOO.bytes[0] = b;
               if(k+3 >=currList.size)
               {
                   return z_err(ValueError,"Error the list is missing some bytes!");
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
               zobject e;
               e.type = 'i';
               e.i =(int32_t) FOO.x;
               zlist_push(&res,e);
            }
            else if(pattern[i]=='b')
            {

               zobject e;
               e.type = 'b';
               e.i = b;
              zlist_push(&res,e);
            }
            else if(pattern[i]=='l')
            {
               FOO1.bytes[0] = b;
               if(k+7 >=currList.size)
               {
                   return z_err(ValueError,"Error the list is missing some bytes!");
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
               zobject e;
               e.type = 'l';
               e.l = FOO1.l;

                             zlist_push(&res,e);
            }
            else if(pattern[i]=='s')
            {
                size_t j = k;
                zobject e;
                zstr* f = vm_alloc_zstr();
                bool terminated = false;
                while(true)
                {
                    if(j>=currList.size)
                    {
                        return z_err(ValueError,"Ran out of bytes!");
                    }
                    e = currList.arr[j];
                    if(e.type!='m')
                        return z_err(ValueError,"Error the list should only contain bytes!");
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
                    return z_err(ValueError,"Error the bytes are invalid to be converted to string!");
                }

                zlist_push(&res,zobj_from_str_ptr(f));
                k  = j;
            }
            else if(pattern[i]=='f')
            {
               FOO2.bytes[0] = b;
               if(k+3 >=currList.size)
               {
                   return z_err(ValueError,"Error the list is missing some bytes!");
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
               zobject e;
               e.type = 'f';
               e.f = FOO2.f;
               zlist_push(&res,e);
            }
            else
            {
                return z_err(TypeError,"Error unknown type used in pattern!");
            }
            i+=1;
            k+=1;
        }
        if(i!=pattern.length())
        {
            return z_err(ValueError,"Error the list does not have enough bytes to follow the pattern!");
        }
        zlist* p = vm_alloc_zlist();
        zlist_assign(p,&res);
        ret.ptr = (void*)p;
        ret.type = 'j';
        return ret;
}*/
///////
/////////

zobject SUBSTR(zobject* args,int32_t argc)
{
    if(argc==3)
	{
	    if(args[0].type!='i' && args[0].type!='l')
            return z_err(TypeError,"Error first argument of substr() should be an integer");
        if(args[1].type!='i' && args[1].type!='l')
            return z_err(TypeError,"Error second argument of substr() should be an integer");
		if(args[2].type=='s')
		{
            PromoteType(&args[0],'l');
            PromoteType(&args[1],'l');
            zstr* str = (zstr*)args[2].ptr;
            if(args[1].l == 0 || args[0].l >= str->len)
                return zobj_from_str("");
            if(args[1].l < 0)
                return z_err(ValueError,"substr(): count must be positive and nonzero");
            
            size_t start_idx = args[0].l;
            size_t end_idx = start_idx + args[1].l - 1;
            if(end_idx >= str->len)
                end_idx = str->len - 1;
            zstr* result = vm_alloc_zstr(end_idx - start_idx + 1);
            memcpy(result->val,str->val+start_idx,end_idx - start_idx + 1);
            return zobj_from_str_ptr(result);
		}
		else
            return z_err(TypeError,"Error third argument of substr() should be a string!\n");
	}
	return z_err(ArgumentError,"Error substr() takes three arguments!");
}
zobject getFileSize(zobject* args,int32_t argc)
{
  if(argc==1)
  {
    if(args[0].type!='u')
        return z_err(TypeError,"Error getFileSize() takes an open file as argument!");
  
    zfile* p = (zfile*)args[0].ptr;
    if(!p->open)
      return z_err(FileIOError,"Unable to get size of closed file!");
    FILE* currF = p->fp;
    fseek(currF,0,SEEK_END);
    int64_t n = ftell(currF);
    rewind(currF);
    return zobj_from_int64(n);
  }
  return z_err(ArgumentError,"Error getFileSize() takes 1 argument!");
}
zobject FTELL(zobject* args,int32_t argc)
{
  if(argc==1)
  {
    if(args[0].type!='u')
        return z_err(TypeError,"Error ftell() takes an open file as argument!");
    zfile* p = (zfile*)args[0].ptr;
    if(!p->open)
      return z_err(FileIOError,"Error file is closed!");
    FILE* currF = p->fp;
    int64_t n = ftell(currF);
    zobject ret = nil;
    ret.type = 'l';
    ret.l = (int64_t)n;
    return ret;
  }
  return z_err(ArgumentError,"Error ftell() takes 1 argument!");
}
zobject REWIND(zobject* args,int32_t argc)
{
  if(argc==1)
  {
    if(args[0].type!='u')
        return z_err(TypeError,"Error rewind() takes an open file as argument!");
  
    zfile* p = (zfile*)args[0].ptr;
    if(!p->open)
      return z_err(FileIOError,"Unable to rewind closed file!");
    FILE* currF = p->fp;
    rewind(currF);
    zobject ret = nil;
    return ret;
  }
  return z_err(ArgumentError,"Error rewind() takes 1 argument!");
}
zobject SYSTEM(zobject* args,int32_t argc)
{
  if(argc!=1)
      return z_err(ArgumentError,"Error system() takes 1 argument!");
  if(args[0].type!='s')
      return z_err(TypeError,"Error system() takes a string argument!");
  zstr* command = (zstr*)args[0].ptr;
  int32_t i = system(command->val);
  return zobj_from_int(i);
}
zobject SPLIT(zobject* args,int32_t argc)
{
    if(argc==2)
	{
		if( (args[0].type=='s') && ( args[1].type=='s'))
		{
            zstr* data = (zstr*)args[0].ptr;
            zstr* delim = (zstr*)args[1].ptr;
            if(delim->len == 0)
                return z_err(ValueError,"delimiter length must not be zero!");
			size_t k = 0;
            const char* str = data->val;
            const char* d = delim->val;

            char ch;
            zlist* parts = vm_alloc_zlist();
            size_t start = 0;
            while((ch = str[k]))
            {
                if(ch == d[0] && (k + delim->len - 1 < data->len) && strncmp(str + k,d, delim->len) == 0)
                {
                    size_t end = k + delim->len -1;
                    size_t chunk_size = end - start;
                    zstr* chunk = vm_alloc_zstr(chunk_size);
                    if(chunk_size != 0)
                        memcpy(chunk->val,str+start,chunk_size);
                    zlist_push(parts,zobj_from_str_ptr(chunk));
                    k += delim->len;
                    start = k;
                    continue;
                }
                k++;
            }
            size_t end = k;
            size_t chunk_size = end - start;
            zstr* chunk = vm_alloc_zstr(chunk_size);
            if(chunk_size!=0)
                memcpy(chunk->val,str+start,chunk_size);
            zlist_push(parts,zobj_from_str_ptr(chunk));
            return zobj_from_list(parts);
        }
        else
            return z_err(TypeError,"Error split() takes both string arguments!\n");
	}
	return z_err(ArgumentError,"Error split() takes two arguments!");
}
zobject GETENV(zobject* args,int32_t argc)
{
  if(argc==1)
  {

      if(args[0].type=='s' )
      {
        zstr* vname = (zstr*)args[0].ptr;
        char* c = getenv(vname->val);
        if(!c)
          return nil;
        zstr* s = vm_alloc_zstr(strlen(c));
        strcpy(s->val,c);
        return zobj_from_str_ptr(s);
      }
      else
      {
        return z_err(TypeError,"Error getenv() takes a string argument!");
      }
  }
  return z_err(ArgumentError,"Error getenv() takes one argument!");
}
zobject SHUFFLE(zobject* args,int32_t argc)
{
    if(argc==1)
		{
			if(args[0].type=='j')
			{
			  zlist* p = (zlist*)args[0].ptr;
        //std::random_shuffle(p->arr,p->arr+p->size);
				zobject ret = nil;
				return ret;
			}
			else
      {
        return z_err(TypeError,"Error shuffle takes a list as an argument!");
      }
		}
		return z_err(ArgumentError,"Error shuffle() takes exactly one argument!");
}
zobject STR(zobject* args,int32_t argc)
{
  if(argc==1)
	{
		if(args[0].type=='i')
		{
            //string s = str(args[0].i);
            char buffer[10];
            snprintf(buffer,10,"%d",args[0].i);
            return zobj_from_str(buffer);
		}
		else if(args[0].type=='f')
		{
            char* s = double_to_str(args[0].i);
            zobject ret;
            ret = zobj_from_str(s);
            free(s);
            return ret;
		}
        else if(args[0].type=='l')
        {
            char buffer[20];
            snprintf(buffer,20,"%" PRId64 ,args[0].l);
            return zobj_from_str(buffer);
        }
        else if(args[0].type == Z_BYTE)
        {
            char buffer[8];
            snprintf(buffer,8,"0x%02x",args[0].i);
            return zobj_from_str(buffer);
        }
        else if(args[0].type == 'b')
        {
            zstr* s = vm_alloc_zstr(5);
            if(args[0].i)
                strcpy(s->val,"true");
            else
                strcpy(s->val,"false");
            return zobj_from_str_ptr(s);
        }
        else if(args[0].type == 'j' || args[0].type == 'a')
        {
          char* s = zobject_to_str(args[0]);
          zobject ret = zobj_from_str(s);
          free(s);
          return ret;
        }
        else if(args[0].type == Z_BYTEARR)
        {
            //string s;
            dyn_str result;
            dyn_str_init(&result);
            zbytearr* bytes = (zbytearr*)args[0].ptr;
            for(size_t i=0;i<bytes->size;i++)
            {
                uint8_t byte = bytes->arr[i];
                dyn_str_push(&result,(char)byte);
            }
            zobject ret = zobj_from_str(result.arr);
            free(result.arr);
            return ret;
        }
        char error_buffer[50];
        snprintf(error_buffer,50,"str() unsupported for type %s",fullform(args[0].type));
	}
    return z_err(ArgumentError,"Error str() takes only one argument!");
}
zobject FIND(zobject* args,int32_t argc)
{
  zobject ret = nil;
  if(argc != 2)
    return z_err(ArgumentError,"Error find() takes 2 arguments!");
  if(args[0].type != Z_STR)
    return z_err(TypeError,"Error first argument given to find() must be a stirng!");
  if(args[1].type != Z_STR)
    return z_err(TypeError,"Error second argument given to find() must be a stirng!");
  
  ret.type = 'l';
  zstr* x = (zstr*)args[0].ptr;
  zstr* str = (zstr*)args[1].ptr;
  if(x->len == 0)
      return zobj_from_int(0);
  for(size_t i = 0;i< str->len; i++)
  {
      if(str->val[i] == x->val[0] && (i + x->len - 1 < str->len) && strncmp(str->val+i,x->val,x->len) == 0)
        return zobj_from_int64(i);
  }
  return nil;
}
zobject TOINT(zobject* args,int32_t argc)
{
    if(argc==1)
	{
	    if(args[0].type==Z_STR)
		{
		    zstr* q = ((zstr*)args[0].ptr);
			zobject ret = nil;
            if(isnum(q->val))
            {
                ret.i = str_to_int32(q->val);
                ret.type = 'i';
            }
            else if(is_int64(q->val))
            {
                ret.l = str_to_int64(q->val);
                ret.type = 'l';
            }
            else if(isdouble(q->val))
            {
                ret.type = Z_INT64;
                int64_t tmp = atoll(q->val);
                if(tmp >= INT_MIN && tmp <= INT_MAX)
                {
                    ret.type = Z_INT;
                    ret.i = (int32_t)tmp;
                    return ret;
                }
                ret.l = tmp;
                return ret;
            }
            else
                return z_err(ValueError,"Error the string  cannot be converted to an integer!");
            return ret;
        }
		else if(args[0].type=='f')
		{
            zobject ret;
            double d = args[0].f;
            if(d<(double)LLONG_MIN)
                ret.l = LLONG_MIN;
            else if(d>(double)LLONG_MAX)
                ret.l = LLONG_MAX;
            else 
                ret.l = (int64_t)(args[0].f);
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
        char buffer[50];
        snprintf(buffer,50,"Error int() unsupported for type %s",fullform(args[0].type));
        return z_err(TypeError,buffer);
	}
	return z_err(ArgumentError,"Error int() takes exactly one argument!");
}
zobject TOINT32(zobject* args,int32_t argc)
{
    if(argc==1)
	{
	    if(args[0].type=='s')
		{
		    zstr* q = (zstr*)args[0].ptr;
			zobject ret = nil;
            if(isnum(q->val))
                return zobj_from_int(str_to_int32(q->val));
            else if(is_int64(q->val))
                return zobj_from_int(INT_MAX);
            else if(isdouble(q->val))
            {
                double d = str_to_double(q->val);
                if(d < INT_MIN)
                    return zobj_from_int(INT_MIN);
                if(d > INT_MAX)
                    return zobj_from_int(INT_MAX);
                return zobj_from_int((int32_t)d);
            }
            else
                return z_err(ValueError,"Error the string cannot be converted to an integer!");
			return ret;
        }
	    else if(args[0].type=='f')
		{
            zobject ret = nil;
            double d = args[0].f;
            if(d<(double)INT_MIN)
                ret.i = INT_MIN;
            else if(d>(double)INT_MAX)
                ret.i = INT_MAX;
            else 
                ret.i = (int32_t)args[0].f;
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
                return zobj_from_int(INT_MAX);
            else if(args[0].l < INT_MIN)
                return zobj_from_int(INT_MIN);
            return zobj_from_int((int32_t)args[0].l); 
        }
        char buffer[50];
        snprintf(buffer,50,"int32() unsupported for type %s",fullform(args[0].type));
        return z_err(TypeError,buffer);
	}
	return z_err(ArgumentError,"Error int32() takes exactly one argument!");
}
zobject TOINT64(zobject* args,int32_t argc)
{
    if(argc==1)
	{
	    if(args[0].type=='s')
		{
		    zstr* q = (zstr*)args[0].ptr;
			zobject ret = nil;
            if(isnum(q->val))
                return zobj_from_int64(str_to_int32(q->val));
            else if(is_int64(q->val))
                return zobj_from_int64(str_to_int64(q->val));
            else if(isdouble(q->val))
            {
                double d = str_to_double(q->val);
                if(d < (double)LLONG_MIN)
                    return zobj_from_int64(LLONG_MIN);
                if(d > (double)LLONG_MAX)
                    return zobj_from_int64(LLONG_MAX);
                return zobj_from_int64((int64_t)d);
            }
            return z_err(ValueError,"string cannot be converted to int64!");
        }
		else if(args[0].type=='f')
		{
            double d = args[0].f;
            if(d<(double)LLONG_MIN)
                return zobj_from_int64(LLONG_MIN);
            if(d > (double)LLONG_MAX)
                return zobj_from_int64(LLONG_MAX);
            return zobj_from_int64(d);
		}
        else if(args[0].type == 'm' || args[0].type == Z_BOOL || args[0].type == Z_INT)
        {
            args[0].type = Z_INT64;
            args[0].l = args[0].i;
            return args[0];
        }
        else if(args[0].type == Z_INT64)
            return args[0];
        char buffer[50];
        snprintf(buffer,50,"int64() unsupported for type %s",fullform(args[0].type));
        return z_err(TypeError,buffer);
	}
	return z_err(ArgumentError,"Error int64() takes exactly one argument!");
}
zobject TOFLOAT(zobject* args,int32_t argc)
{
    if(argc==1)
	{
		if(args[0].type=='s')
		{
            zstr* q = (zstr*)args[0].ptr;
            if(isnum(q->val))
                return zobj_from_double((double)str_to_int32(q->val));
            else if(is_int64(q->val))
                return zobj_from_double((double)str_to_int64(q->val));
            else if(isdouble(q->val))
                return zobj_from_double(str_to_double(q->val));
            else
                return z_err(ValueError,"The string cannot be converted to a float!\n");
		}
        else if(args[0].type == 'i')
            return zobj_from_double(args[0].i);
        else if(args[0].type == 'l')
            return zobj_from_double(args[0].l);
        else if(args[0].type == Z_FLOAT)
            return args[0];
        char buffer[50];
        snprintf(buffer,50,"float() unsupported for type %s",fullform(args[0].type));
        return z_err(TypeError,buffer);
	}
	return z_err(ArgumentError,"Error float() takes exactly one argument!");
}
zobject tonumeric(zobject* args,int32_t argc)
{
    if(argc==1)
    {
        if(args[0].type=='s')
        {
            zstr* q = (zstr*)args[0].ptr;
            if(isnum(q->val))
                return zobj_from_int(str_to_int32(q->val));
            else if(is_int64(q->val))
                return zobj_from_int64(str_to_int64(q->val));
            else if(isdouble(q->val))
                return zobj_from_double(str_to_double(q->val));
            else
                return z_err(ValueError,"cannot convert the string to numeric type!");
        }
        else
          return z_err(TypeError,"Error tonumeric() takes a string argument!");
	}
    return z_err(ArgumentError,"Error tonumeric() takes one argument!");
}
zobject isnumeric(zobject* args,int32_t argc)
{
    if(argc==1)
    {
        if(args[0].type=='s')
        {
            zstr* s = (zstr*)args[0].ptr;
            if(isnum(s->val))
                return zobj_from_bool(true);
            else if(is_int64(s->val))
                return zobj_from_bool(true);
            else if(isdouble(s->val))
                return zobj_from_bool(true);
            else
                return zobj_from_bool(false);
        }
        else
            return z_err(TypeError,"isnumeric() takes a string argument!");
    }
    return z_err(ArgumentError,"Error isnumeric() takes 1 argument!");
}
zobject REPLACE(zobject* args,int32_t argc)
{
        if(argc==3)
        {
           if(args[0].type!='s')
               return z_err(TypeError,"Error first argument given to replace() must be a string!");
           if(args[1].type!='s')
               return z_err(TypeError,"Error second argument given to replace() must be a string!");
           if(args[2].type!='s')
               return z_err(TypeError,"Error third argument given to replace() must be a string!");
           zstr* a = (zstr*)args[0].ptr;
           zstr* b = (zstr*)args[1].ptr;
           zstr* c = (zstr*)args[2].ptr;
           dyn_str result;
           dyn_str_init(&result);
           replace_all(a,b,c,&result);
           zstr* str = vm_alloc_zstr(result.length);
           strcpy(str->val,result.arr);
           free(result.arr);
           return zobj_from_str_ptr(str);
        }
        else
        {
            return z_err(ArgumentError,"Error replace() takes three arguments\n");
        }
}
zobject REPLACE_ONCE(zobject* args,int32_t argc)
{
        if(argc==3)
        {
           if(args[0].type!='s')
               return z_err(TypeError,"Error first argument given to replace_once() must be a string!");
           if(args[1].type!='s')
               return z_err(TypeError,"Error second argument given to replace_once() must be a string!");
           if(args[2].type!='s')
               return z_err(TypeError,"Error third argument given to replace_once() must be a string!");
           zstr* a = (zstr*)args[0].ptr;
           zstr* b = (zstr*)args[1].ptr;
           zstr* c = (zstr*)args[2].ptr;
           dyn_str result;
           dyn_str_init(&result);
           replace_once(a,b,c,&result);
           zobject ret = zobj_from_str(result.arr);
           free(result.arr);
           return ret;
        }
        else
        {
            return z_err(ArgumentError,"Error replace_once() takes three arguments\n");
        }
}
zobject SLEEP(zobject* args,int32_t argc)
{
    if(argc!=1)
        return z_err(ArgumentError,"Error sleep() takes 1 argument!");
    if(args[0].type=='i' && args[0].type!='l')
    {
      zobject r = args[0];
      PromoteType(&r,'l');
      #ifdef _WIN32
      #include <windows.h>
         Sleep(r.l);
      #else
        usleep(r.l*1000);
      #endif
      zobject ret = nil;
      return ret;
    }
    else
    {
      return z_err(TypeError,"Error sleep() takes an integer argument!");
    }
}


zobject TOBYTE(zobject* args,int32_t argc)
{
  if(argc!=1)
    return z_err(ArgumentError,"Error byte() takes 1 argument!");
  if(args[0].type == Z_INT)
  {
    if(args[0].i>255 || args[0].i<0)
      return z_err(ValueError,"The integer must be in range 0 to 255!");
  }
  else if(args[0].type == Z_INT64)
  {
    if(args[0].l>255 || args[0].l<0)
      return z_err(ValueError,"The integer must be in range 0 to 255!");
    args[0].i = args[0].l;
  }
  else if(args[0].type == Z_BYTE || args[0].type == Z_BOOL)
  {
    args[0].type = Z_BYTE;
    return args[0];
  }
  else
    ;//return z_err(TypeError,"Cannot convert type "+fullform(args[0].type)+" to byte!");
  args[0].type = Z_BYTE;
  return args[0];
}
zobject writelines(zobject* args,int32_t argc)
{
     if(argc==2)
        {
            if(args[0].type!='j')
                return z_err(TypeError,"Error first argument of writelines() should be a list!");
            if(args[1].type=='u')
            {
                zlist* lines = (zlist*)args[0].ptr;
                uint32_t f = 0;
                //string data = "";
                zobject m;
                zfile* p = (zfile*)args[1].ptr;
                FILE* currF = p->fp;
                while(f<lines->size)
                {
                    m = (lines->arr[f]);
                    if(m.type!='s')
                      return z_err(ValueError,"List provided to writelines should consist of string elements only!");
                    zstr* line = (zstr*)m.ptr;
                    fputs(line->val,currF);
                    if(f!=lines->size-1)
                        fputc('\n',currF);
                    f+=1;
                }
                return nil;
            }
            return z_err(ArgumentError,"Error writelines() needs a filestream to write!");
        }
        else
        {
            return z_err(ValueError,"Error writelines() takes two arguments!");
            exit(0);
        }
}
zobject readlines(zobject* args,int32_t argc)
{
    if(argc==1)
    {
      if(args[0].type!='u')
          return z_err(TypeError,"Argument not a filestream!");
      signed char ch;
      zfile fobj = *(zfile*)args[0].ptr;
      if(!fobj.open)
        return z_err(ValueError,"Error the file stream is closed!");
      FILE* currF = fobj.fp;
      zlist* lines = vm_alloc_zlist();
      //string reg; 
      int32_t k = 0;
/*
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
              zstr* line = vm_alloc_zstr(reg.length());
              strcpy(line->val,reg.c_str());
              zlist_push(lines,zobj_from_str_ptr(line));
              reg = "";

          }
          else
          {
            reg+=ch;
          }
      }
      zstr* line = vm_alloc_zstr(reg.length());
      strcpy(line->val,reg.c_str());
      zlist_push(lines,zobj_from_str_ptr(line));
      zobject ret;
      ret.type = 'j';
      ret.ptr = (void*)lines;
      return ret;*/
    }
    else
    {
        return z_err(ArgumentError,"Error readlines() takes one argument!");
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
zobject FREAD(zobject* args,int32_t argc)
{
    if(argc!=3)
        return z_err(ArgumentError,"Error fread() takes 3 arguments!");
    if (args[0].type != Z_BYTEARR)
        return z_err(TypeError, "Error first argument to fread must be a bytearray!");
    if(args[1].type!='i' && args[1].type!='l')
        return z_err(TypeError,"Error second argument of fread() should be an integer!");
    if(args[2].type!='u')
      return z_err(TypeError,"Error third argument of fread() should be a file stream!");
    zobject a = args[1];
    PromoteType(&a,'l');
    int64_t e = a.l;
    zbytearr* p = (zbytearr*)args[0].ptr;
    zbytearr_resize(p,e);
    zfile fobj = *(zfile*)args[2].ptr;
    if(!fobj.open)
      return z_err(ValueError,"Error the file stream is closed!");
    FILE* currF = fobj.fp;
    unsigned long long int read = 0;
    if((read = fread(p->arr,1,e,currF))!=(size_t)e)
        return z_err(FileIOError,"Error unable to read specified bytes from the file.");
    if(currF == stdin)
      clean_stdin();//the newline character can cause problem with future inputs or REPL
    return zobj_from_int64(read);;
}
zobject FWRITE(zobject* args,int32_t argc)
{
    zobject ret = nil;
    size_t S = 0;
    if(argc==2)
    {
    if(!validateArgTypes("fwrite","cu",args,argc,&ret))
      return ret;
     S = ((zbytearr*)args[0].ptr)->size; 
    }
    else if(argc==3)
    {
       if(args[0].type!='j' || args[1].type!='u' || (args[2].type!='i' && args[2].type!='l'))
         return z_err(TypeError,"Invalid Argument types");
       if(args[2].type=='i')
         S = (size_t)args[2].i;
        else
         S = (size_t)args[2].l;
    }
    else
    {
      return z_err(ArgumentError,"Error fwrite takes either 2 or 3 arguments");
    }
    zbytearr* l = (zbytearr*)args[0].ptr;
    if(S > l->size)
      return z_err(ValueError,"Error the bytearray needs to have specified number of bytes!");
    if(l->size == 0)
      return ret;
    zfile fobj = *(zfile*)args[1].ptr;
    if(!fobj.open)
      return z_err(ValueError,"Error the file stream is closed!");
    FILE* currF = fobj.fp;
    int64_t written = 0;
    if((written = fwrite(l->arr,1,S,currF))!=(int64_t)S)
    {
       // string what = strerror(errno);
        return z_err(FileIOError,"Error unable to write the bytes to file!");

    }
    return zobj_from_int64(written);
}
zobject FSEEK(zobject* args,int32_t argc)
{
    if(argc!=3)
        return z_err(ArgumentError,"Error fseek() takes 3 arguments!");
    zobject ret = nil;
    if(!validateArgTypes("fseek","uii",args,argc,&ret))
      return ret;
    int32_t w = 0;
    int32_t whence = args[2].i;
    zfile fobj = *(zfile*)args[0].ptr;
    if(!fobj.open)
      return z_err(ValueError,"Error the file stream is closed!");
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
        return z_err(ValueError,"Error invalid option ");
    if(fseek(currF,where,w)!=0)
      {
        //string what = strerror(errno);
        return z_err(FileSeekError,strerror(errno));
      }
    return ret;
}
//
zlist* makeListCopy(zlist);
zdict* makeDictCopy(zdict);
//
zdict* makeDictCopy(zdict v)
{
  zdict* d = vm_alloc_zdict();
  for(size_t i=0;i<d->capacity;i++)
  {
    if(d->table[i].stat != OCCUPIED)
      continue;
    zobject key,val;
    key = d->table[i].key;
    val = d->table[i].val;
    zdict_set(d,key,val);
  }
  return d;
}
zlist* makeListCopy(zlist v)
{
  zlist* p = vm_alloc_zlist();
  for(size_t i = 0;i<v.size;i++)
  {
    zobject e = v.arr[i];
    zobject elem;
    elem = e;
    zlist_push(p,elem);
  }
  return p;
}
zobject COPY(zobject* args,int32_t argc)
{
  if(argc!=1)
      return z_err(ArgumentError,"Error clone() takes one argument!");
  if(args[0].type=='a')
  {
     zdict v = *(zdict*)args[0].ptr;
     zobject ret = nil;
     ret.type = 'a';
     ret.ptr = (void*)makeDictCopy(v);
     return ret;

  }
  else if(args[0].type=='j')
  {
    zlist v = *(zlist*)args[0].ptr;
    zobject ret = nil;
    ret.type = 'j';
    ret.ptr = (void*)makeListCopy(v);
    return ret;
  }
  else
    return z_err(TypeError,"Error clone() takes a list or dictionary as an argument!");
}
zobject POW(zobject* args,int32_t argc)
{
    if(argc!=2)
    {
        return z_err(ArgumentError,"pow() takes 2 arguments!");
    }
    zobject a = args[0];
    zobject b = args[1];
    zobject c;
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
      PromoteType(&a,t);
      PromoteType(&b,t);
    }
    else
    {
      //return z_err(TypeError,"Error pow() unsupported for "+fullform(a.type)+" and "+fullform(b.type));
    }
      if(t=='i')
      {
        c.type = 'i';
        if(!exponen_overflows_i32(a.i,b.i))
        {
          c.i = pow(a.i,b.i);
          return c;
        }

        if(exponen_overflows_i64((int64_t)a.i,(int64_t)b.i))
        {
          return z_err(OverflowError,"Integer Overflow occurred in pow()");
        }
        c.type = 'l';
        c.l = pow((int64_t)(a.i) , (int64_t)(b.i));
        return c;
      }
      else if(t=='f')
      {
        if(exponen_overflows_double(a.f,b.f))
        {
              return z_err(OverflowError,"Floating Point Overflow occurred in pow()");
        }
        c.type = 'f';
        c.f = pow(a.f,b.f);
        return c;
      }
      else if(t=='l')
      {
          if(exponen_overflows_i64(a.l,b.l))
          {
              return z_err(OverflowError,"Integer Overflow occurred in pow().");
          }
          c.type = 'l';
          c.l = pow(a.l,b.l);
          return c;
      }
        return c;//to avoid warning otherwise this stmt is never executed
}
zobject CLOCK(zobject* args,int32_t argc)
{
  if(argc!=0)
    return z_err(ArgumentError,"Error clock() takes 0 arguments!");
  zobject ret = nil;
  ret.type = 'f';
  ret.f = (double)(clock());
  return ret;
}

////////////////////
//Builtin Methods
//Methods work exactly like functions but their first argument should always
//be an object
//these are just functions that work on multiple supported types
//for example POP functions works on both list,bytearrays and mutable strings
zobject POP(zobject* args,int32_t argc)
{
  if(args[0].type!=Z_LIST && args[0].type!=Z_BYTEARR )
        ;// return z_err(NameError,"Error type "+fullform(args[0].type)+" has no method named pop()");
  if(argc!=1)
    return z_err(ArgumentError,"Error method pop() takes 0 arguments!");//args[0] is self
  if(args[0].type == 'c')
  {
    zobject ret = nil;

    zbytearr* p = (zbytearr*)args[0].ptr;
    uint8_t res;
    zbytearr_pop(p,&res);
    ret.type = Z_BYTE;
    ret.i = res;
    return ret;
  }
  else
  {
    zlist* p = (zlist*)args[0].ptr;
    zobject ret = nil;
    if(p->size!=0)
    {
      zlist_pop(p,&ret);
    }
    else
      return z_err(ValueError,"List is empty.No value to pop!");
    return ret;
  }
  return nil;
}

zobject CLEAR(zobject* args,int32_t argc)
{
  if(args[0].type!='j' && args[0].type != Z_BYTEARR)
        ; //return z_err(NameError,"Error type "+fullform(args[0].type)+" has no method named clear()");
  if(argc!=1)
    return z_err(ArgumentError,"Error method clear() takes 0 arguments!");
  if(args[0].type == Z_BYTEARR)
  {
    zbytearr* arr = (zbytearr*)args[0].ptr;
    arr->size = 0;
  }
  else
  {
    zlist* p = (zlist*)args[0].ptr;
    zlist_resize(p,0);
  }
  return nil;
}
zobject PUSH(zobject* args,int32_t argc)
{
  if(args[0].type!='j' && args[0].type!='c')
        ;// return z_err(NameError,"Error type "+fullform(args[0].type)+" has no method named push()");
  if(argc!=2)
    return z_err(ArgumentError,"Error method push() takes 1 argument!");
  if(args[0].type == 'j')
  {
    zlist* p = (zlist*)args[0].ptr;

    zlist_push(p,args[1]);
    zobject ret = nil;
    return ret;
  }
  else
  {
    zbytearr* p = (zbytearr*)args[0].ptr;
    if(args[1].type != Z_BYTE)
     return z_err(TypeError,"Can only push a byte to a bytearray!");
    zbytearr_push(p,args[1].i);
    zobject ret = nil;
    return ret;
  }
}

zobject FIND_METHOD(zobject* args,int32_t argc)
{
  if(args[0].type!='j' && args[0].type!=Z_BYTEARR && args[0].type!=Z_STR)
    ;//return z_err(NameError,"Error type "+fullform(args[0].type)+" has no method named find()");
  if(argc!=2)
    return z_err(ArgumentError,"Error method find() takes 1 arguments!");
  if(args[0].type == 'j')
  {
    zlist* p = (zlist*)args[0].ptr;
    zobject ret = nil;
    for(size_t k=0;k<p->size;k+=1)
    {
      if(zobject_equals(p->arr[k],args[1]))
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
    if(args[1].type != Z_STR)
      return z_err(TypeError,"Argument 1 of str.find() must be a string!");
    zstr* tofind = (zstr*)args[1].ptr;
    zstr* str = (zstr*)args[0].ptr;
    for(size_t i = 0;i< str->len; i++)
    {
      if(str->val[i] == tofind->val[0] && (i + tofind->len - 1 < str->len) && strncmp(str->val+i,tofind->val,tofind->len) == 0)
        return zobj_from_int64(i);
    }
    return nil;
  }
  else
  {
    zbytearr* p = (zbytearr*)args[0].ptr;
    zobject ret = nil;
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
zobject INSERTBYTEARRAY(zobject* args,int32_t argc)
{
  if(argc==3)
  {
    zbytearr* p = (zbytearr*)args[0].ptr;
    zobject idx = args[1];
    zobject val = args[2];
    if(idx.type!='i' && idx.type!='l')
      return z_err(TypeError,"Error method insert() expects an integer argument for position!");
    PromoteType(&idx,'l');
    if(idx.l < 0)
      return z_err(ValueError,"Error insertion position is negative!");
    if((size_t)idx.l > p->size)
          return z_err(ValueError,"Error insertion position out of range!");
    if(val.type=='c')
    {
      zbytearr* subarr = (zbytearr*)val.ptr;
      zbytearr_insert_arr(p,idx.l,subarr);
    }
    else if(val.type == Z_BYTE)
    {
      zbytearr_insert(p,idx.l,val.i);
    }
    else
      return z_err(TypeError,"Error method insert() takes a byte or bytearray argument!");
    zobject ret = nil;
    return ret;
  }
  else
    return z_err(ArgumentError,"Error method insert() takes 2 arguments!");
}

zobject INSERTSTR(zobject* args,int32_t argc)
{
  if(argc==3)
  {
    zstr* p = (zstr*)args[0].ptr;
    //string p = ((zstr*)args[0].ptr) -> val;
    zobject idx = args[1];
    zobject val = args[2];
    if(idx.type!='i' && idx.type!='l')
      return z_err(TypeError,"Error method insert() expects an integer argument for position!");
    PromoteType(&idx,'l');
    if(idx.l < 0)
      return z_err(ValueError,"Error insertion position is negative!");
    if((size_t)idx.l > p->len)
          return z_err(ValueError,"Error insertion position out of range!");
    if(val.type==Z_STR)
    {
      zstr* sub = (zstr*)val.ptr;
      zstr* result = vm_alloc_zstr(sub->len+p->len );
      if(idx.l == 0)
      {
          memcpy(result->val,sub->val,sub->len);
          memcpy(result->val + sub->len,p->val,p->len);
      }
      else if(idx.l == p->len)
      {
          memcpy(result->val,p->val,p->len);
          memcpy(result->val+p->len,sub->val,sub->len);
      }
      else
      {
          size_t before = idx.l;
          size_t after = p->len - before;
          memcpy(result->val,p->val,before);
          memcpy(result->val + before,sub->val,sub->len);
          memcpy(result->val + idx.l+sub->len, p->val + idx.l,after);
      }
      return zobj_from_str_ptr(result);
    }
    else
      return z_err(TypeError,"Error method insert() takes a string argument!");
  }
  else
    return z_err(ArgumentError,"Error method insert() takes 2 arguments!");
}

zobject INSERT(zobject* args,int32_t argc)
{
  if(args[0].type == 'c')
    return INSERTBYTEARRAY(args,argc);
  else if(args[0].type == Z_STR)
    return INSERTSTR(args,argc);
  if(args[0].type!='j')
        ;// return z_err(NameError,"Error type "+fullform(args[0].type)+" has no method named insert()");
  if(argc==3)
  {
    zlist* p = (zlist*)args[0].ptr;
    zobject idx = args[1];
    zobject val = args[2];
    if(idx.type!='i' && idx.type!='l')
      return z_err(TypeError,"Error method insert() expects an integer argument for position!");
    PromoteType(&idx,'l');
    if(idx.l < 0)
      return z_err(ValueError,"Error insertion position is negative!");
    if((size_t)idx.l > p->size)
          return z_err(ValueError,"Error insertion position out of range!");
    if(val.type=='j')
    {
      zlist* sublist = (zlist*)val.ptr;
      zlist_insert_list(p,idx.l,sublist);
    }
    else
    {
      zlist_insert(p,idx.l,val);
    }
    zobject ret = nil;
    return ret;
  }
  else
    return z_err(ArgumentError,"Error method insert() takes 3 arguments!");
}
zobject ERASE(zobject* args,int32_t argc)
{
  if(args[0].type=='j')
  {
    if(argc!=2 && argc!=3)
      return z_err(ArgumentError,"Error erase() takes 2 or 3 arguments!");
    zlist* p = (zlist*)args[0].ptr;
    zobject idx1 = args[1];
    zobject idx2;
    if(argc==3)
     idx2 = args[2];
    else
      idx2 = idx1;
    if(idx1.type!='i' && idx1.type!='l')
        return z_err(TypeError,"Error method erase() expects an integer argument for start!");
    if(idx2.type!='i' && idx2.type!='l')
        return z_err(TypeError,"Error method erase() expects an integer argument for end!");
    PromoteType(&idx1,'l');
    PromoteType(&idx2,'l');
    if(idx1.l < 0 || idx2.l < 0)
        return z_err(ValueError,"Error index is negative!");
    if((size_t)idx1.l >= p->size || (size_t)idx2.l >= p->size)
        return z_err(ValueError,"Error index out of range!");
    zlist_erase_range(p,idx1.l,idx2.l);
    zobject ret = nil;
    return ret;
  }
  else if(args[0].type=='a')
  {
     if(argc!=2)
       return z_err(ArgumentError,"Error dictionary method erase() takes 2 arguments!");
    zdict* d = (zdict*)args[0].ptr;
    zobject key = args[1];
    if(key.type!='i' && key.type!='l' && key.type!='f' && key.type!='s' && key.type!='m' && key.type!='b')
      ;//return z_err(TypeError,"Error key of type "+fullform(key.type)+" not allowed.");
    bool b = zdict_erase(d,key);
    zobject ret;
    ret.type = Z_BOOL;
    ret.i = b;
    return ret;
  }
  else if(args[0].type == 'c')
  {
    if(argc!=2 && argc!=3)
      return z_err(ArgumentError,"Error erase() takes 2 or 3 arguments!");
    zbytearr* p = (zbytearr*)args[0].ptr;
    zobject idx1 = args[1];
    zobject idx2;
    if(argc==3)
     idx2 = args[2];
    else
      idx2 = idx1;
    if(idx1.type!='i' && idx1.type!='l')
        return z_err(TypeError,"Error method erase() expects an integer argument for start!");
    if(idx2.type!='i' && idx2.type!='l')
        return z_err(TypeError,"Error method erase() expects an integer argument for end!");
    PromoteType(&idx1,'l');
    PromoteType(&idx2,'l');
    if(idx1.l < 0 || idx2.l < 0)
        return z_err(ValueError,"Error index is negative!");
    if((size_t)idx1.l >= p->size || (size_t)idx2.l >= p->size)
        return z_err(ValueError,"Error index out of range!");
    zbytearr_erase_range(p,idx1.l,idx2.l);
    zobject ret = nil;
    return ret;
  }
  else if(args[0].type == Z_STR)
  {
    if(argc!=2 && argc!=3)
      return z_err(ArgumentError,"Error erase() takes 2 or 3 arguments!");
    zstr* p = (zstr*)args[0].ptr;
    zobject idx1 = args[1];
    zobject idx2;
    if(argc==3)
     idx2 = args[2];
    else
      idx2 = idx1;
    if(idx1.type!='i' && idx1.type!='l')
        return z_err(TypeError,"Error method erase() expects an integer argument for start!");
    if(idx2.type!='i' && idx2.type!='l')
        return z_err(TypeError,"Error method erase() expects an integer argument for end!");
    PromoteType(&idx1,'l');
    PromoteType(&idx2,'l');
    if(idx1.l < 0 || idx2.l < 0)
        return z_err(ValueError,"Error index is negative!");
    if((size_t)idx1.l >= p->len || (size_t)idx2.l >= p->len)
        return z_err(ValueError,"Error index out of range!");
    if(idx1.l >= idx2.l)
        return z_err(IndexError,"erase(): idx1 must be less than idx2");

    size_t len = p->len - (idx2.l - idx1.l + 1);
    zstr* result = vm_alloc_zstr(len);
    size_t k = 0;
    size_t i = 0;
    while(k<p->len)
    {
        if(k == idx1.l)
        {
            k = idx2.l+1;
            continue;
        }
        result->val[i++] = p->val[k]; 
        k++;
    }
    return zobj_from_str_ptr(result);
  }
  else
  {
      //return z_err(NameError,"Error type "+fullform(args[0].type)+" has no member named erase.");
  }
}
zobject asMap(zobject* args,int32_t argc)
{
    if(args[0].type!='j')
      ;//return z_err(NameError,"Error type "+fullform(args[0].type)+" has no member named asMap()");
    if(argc!=1)
      return z_err(ArgumentError,"Error list member asMap() takes 0 arguments!");
    zlist l = *(zlist*)args[0].ptr;
    zobject x;
    x.type = 'l';
    size_t i = 0;
    zdict* d = vm_alloc_zdict();

    for(;i<l.size;i++)
    {
       x.l = i;
       zdict_emplace(d,x,l.arr[i]);
    }
    zobject ret = nil;
    ret.type = 'a';
    ret.ptr = (void*)d;
    return ret;
}
zobject ASLIST(zobject* args,int32_t argc)
{
    if(args[0].type!='a')
      ;//return z_err(NameError,"Error type "+fullform(args[0].type)+" has no member named asList()");
    if(argc!=1)
      return z_err(ArgumentError,"Error dictionary member asList() takes 0 arguments!");
    zdict d = *(zdict*)args[0].ptr;
    zlist* list = vm_alloc_zlist();
    for(size_t i=0;i<d.capacity;i++)
    {
      if(d.table[i].stat != OCCUPIED)
        continue;
      zlist* sub = vm_alloc_zlist();
      zlist_push(sub,d.table[i].key);
      zlist_push(sub,d.table[i].val);
      
      zobject x;
      x.type = 'j';
      x.ptr = (void*)sub;
      zlist_push(list,x);
    }
    zobject ret = nil;
    ret.type = 'j';
    ret.ptr = (void*)list;
    return ret;
}
zobject REVERSE_METHOD(zobject* args,int32_t argc)
{
  if(args[0].type!='j' && args[0].type != Z_STR)
        ;// return z_err(NameError,"Error type "+fullform(args[0].type)+" has no method named reverse()");
  if(argc!=1)
    return z_err(ArgumentError,"Error method reverse() takes 0 arguments!");
  if(args[0].type == Z_STR)
  {
    zstr* str = (zstr*)args[0].ptr;
    zstr* p = vm_alloc_zstr(str->len);
    size_t i = 0;
    while(i < str->len)
    {
      p->val[i] = str->val[str->len - i - 1];
      i+=1;
    }
    return zobj_from_str_ptr(p);
  }
  else
  {
    zlist* p = (zlist*)args[0].ptr;
    if(p -> size == 0)
      return nil;
    size_t l = 0;
    size_t h = p->size - 1;

    while(l < h)
    {
      zobject tmp = p->arr[l];
      p->arr[l] = p->arr[h];
      p->arr[h] = tmp;
      l++;
      h--;
    }
  }
  zobject ret = nil;
  return ret;
}
zobject EMPLACE(zobject* args,int32_t argc)
{
  if(args[0].type!='a')
        ;// return z_err(NameError,"Error type "+fullform(args[0].type)+" has no method named emplace()");
  if(argc!=3)
    return z_err(ArgumentError,"Error method emplace() takes 2 arguments!");
  zdict* p = (zdict*)args[0].ptr;
  zobject key = args[1];
  if(key.type!='i' && key.type!='l' && key.type!='f' && key.type!='s' && key.type!='m' && key.type!='b')
    ;//return z_err(TypeError,"Error key of type "+fullform(key.type)+" not allowed.");

  zdict_emplace(p,key,args[2]);
  zobject ret = nil;
  return ret;

}
zobject HASKEY(zobject* args,int32_t argc)
{
  if(args[0].type!='a')
      ;//   return z_err(NameError,"Error type "+fullform(args[0].type)+" has no method named hasKey()");
  if(argc!=2)
    return z_err(ArgumentError,"Error method hasKey() takes 1 argument!");
  zdict* p = (zdict*)args[0].ptr;

  zobject ret = nil;
  zobject tmp;
  ret.type= 'b';
  ret.i = zdict_get(p,args[1],&tmp);

  return ret;

}

//String methods
zobject SUBSTR_METHOD(zobject* args,int32_t argc)
{
    if(args[0].type != Z_STR)
        ;//return z_err(NameError,"Error type "+fullform(args[0].type)+" has no member named substr");
    if(argc!=3)
        return z_err(ArgumentError,"Error str.substr() takes 2 arguments");
    if(args[1].type!='i' && args[1].type!='l')
        return z_err(TypeError,"Error first argument of str.substr() should be an integer");
    if(args[2].type!='i' && args[2].type!='l')
        return z_err(TypeError,"Error second argument of str.substr() should be an integer");
    PromoteType(&args[1],'l');
    PromoteType(&args[2],'l');
    zstr* str = args[0].ptr;
    if(args[2].l == 0 || args[1].l >= str->len)
        return zobj_from_str("");
    if(args[2].l < 0)
        return z_err(ValueError,"substr(): count must be positive");
    
    size_t start_idx = args[1].l;
    size_t end_idx = start_idx + args[2].l - 1;
    if(end_idx >= str->len)
        end_idx = str->len - 1;
    zstr* result = vm_alloc_zstr(end_idx - start_idx + 1);
    memcpy(result->val,str->val+start_idx,end_idx - start_idx + 1);
    return zobj_from_str_ptr(result);

  // Following line no longer required, but I wanted to keep "fuck you micrsoft"
  //return nil; //stupid MSVC gives warning, fuck you microsoft 
}
zobject REPLACE_METHOD(zobject* args,int32_t argc)
{
    if(args[0].type != Z_STR)
    {
        char error_buffer[50];
        snprintf(error_buffer,50,"Type %s has no method named replace()",fullform(args[0].type));
        return z_err(NameError,error_buffer);
    }
    if(argc==3)
    {
        if(args[1].type!='s')
            return z_err(TypeError,"Error first argument given to replace() must be a string!");
        if(args[2].type!='s')
            return z_err(TypeError,"Error second argument given to replace() must be a string!");
        zstr* a = (zstr*)args[1].ptr;
        zstr* b = (zstr*)args[2].ptr;
        zstr* c = (zstr*)args[0].ptr;
        dyn_str result;
        dyn_str_init(&result);
        replace_all(a,b,c,&result);
        zstr* str = vm_alloc_zstr(result.length);
        strcpy(str->val,result.arr);
        free(result.arr);
        return zobj_from_str_ptr(str);
    }
    else
        return z_err(ArgumentError,"Error method replace() takes two arguments");
    
}
zobject REPLACE_ONCE_METHOD(zobject* args,int32_t argc)
{
    if(args[0].type != Z_STR)
    {
        char error_buffer[50];
        snprintf(error_buffer,50,"Type %s has no method named replace()",fullform(args[0].type));
        return z_err(NameError,error_buffer);
    }
    if(argc==3)
    {
        if(args[1].type!='s')
            return z_err(TypeError,"Error first argument given to replace() must be a string!");
        if(args[2].type!='s')
            return z_err(TypeError,"Error second argument given to replace() must be a string!");
        zstr* a = (zstr*)args[1].ptr;
        zstr* b = (zstr*)args[2].ptr;
        zstr* c = (zstr*)args[0].ptr;
        dyn_str result;
        dyn_str_init(&result);
        replace_once(a,b,c,&result);
        zstr* str = vm_alloc_zstr(result.length);
        strcpy(str->val,result.arr);
        free(result.arr);
        return zobj_from_str_ptr(str);
    }
    else
        return z_err(ArgumentError,"Error method replace() takes two arguments");
}

////////////////////
///////////////
void initFunctions()
{
  nil.type = Z_NIL;
  bmap_init(&funcs);
  bmap_emplace(&funcs,"print",&print);
  bmap_emplace(&funcs,"println",&println);
  bmap_emplace(&funcs,"printf",&PRINTF);
  bmap_emplace(&funcs,"input",&input);
  bmap_emplace(&funcs,"typeof",&TYPEOF);
  bmap_emplace(&funcs,"len",&LEN);
  bmap_emplace(&funcs,"open",&OPEN);
  bmap_emplace(&funcs,"read",&READ);
  bmap_emplace(&funcs,"close",&CLOSE);
  bmap_emplace(&funcs,"rand",&RAND);
  bmap_emplace(&funcs,"shuffle",&SHUFFLE);
  bmap_emplace(&funcs,"reverse",&REVERSE);
  bmap_emplace(&funcs,"getenv",&GETENV);
  bmap_emplace(&funcs,"system",&SYSTEM);
  bmap_emplace(&funcs,"readlines",&readlines);
  bmap_emplace(&funcs,"writelines",&writelines);
  bmap_emplace(&funcs,"write",&WRITE);
  bmap_emplace(&funcs,"getFileSize",getFileSize);
  bmap_emplace(&funcs,"fread",&FREAD);
  bmap_emplace(&funcs,"fwrite",&FWRITE);
  bmap_emplace(&funcs,"substr",&SUBSTR);
  bmap_emplace(&funcs,"find",&FIND);
  bmap_emplace(&funcs,"replace_once",&REPLACE_ONCE);
  bmap_emplace(&funcs,"replace",&REPLACE);
  bmap_emplace(&funcs,"sleep",&SLEEP);
  bmap_emplace(&funcs,"exit",&EXIT);
  bmap_emplace(&funcs,"split",&SPLIT);
  bmap_emplace(&funcs,"int",&TOINT);
  bmap_emplace(&funcs,"int32",&TOINT32);
  bmap_emplace(&funcs,"int64",&TOINT64);
  bmap_emplace(&funcs,"float",&TOFLOAT);
  bmap_emplace(&funcs,"tonumeric",&tonumeric);
  bmap_emplace(&funcs,"isnumeric",&isnumeric);
  bmap_emplace(&funcs,"fseek",&FSEEK);
  bmap_emplace(&funcs,"bytes",&BYTES);
 // bmap_emplace(&funcs,"ListFromBytes",&makeList);
  bmap_emplace(&funcs,"str",&STR);
  bmap_emplace(&funcs,"byte",& TOBYTE);
  bmap_emplace(&funcs,"clone",&COPY);
  bmap_emplace(&funcs,"pow",&POW);
  bmap_emplace(&funcs,"obj_info",&OBJINFO);
  bmap_emplace(&funcs,"isalpha",&Z_ISALPHA);
  bmap_emplace(&funcs,"ftell",&FTELL);
  bmap_emplace(&funcs,"rewind",&REWIND);
  bmap_emplace(&funcs,"clock",&CLOCK);
  bmap_emplace(&funcs,"isInstanceOf",&isInstanceOf);
  bmap_emplace(&funcs,"ascii",&ASCII);
  bmap_emplace(&funcs,"char",&TOCHAR);
  bmap_emplace(&funcs,"bytearray",&BYTEARRAY);
  bmap_emplace(&funcs,"moduleInfo",&moduleInfo);
  bmap_emplace(&funcs,"format",&FORMAT);
  bmap_emplace(&funcs,"fflush",&FFLUSH);
  bmap_emplace(&funcs,"tolower",&TOLOWER);
  bmap_emplace(&funcs,"toupper",&TOUPPER);

}
bmap methods;
void initMethods()
{
  bmap_init(&methods);
  nil.type = Z_NIL;
  bmap_emplace(&methods,"push",&PUSH);
  bmap_emplace(&methods,"pop",&POP);
  bmap_emplace(&methods,"clear",&CLEAR);
  bmap_emplace(&methods,"insert",&INSERT);
  bmap_emplace(&methods,"find",&FIND_METHOD);
  bmap_emplace(&methods,"asMap",&asMap);
  bmap_emplace(&methods,"erase",&ERASE);
  bmap_emplace(&methods,"reverse",&REVERSE_METHOD);
  bmap_emplace(&methods,"emplace",&EMPLACE);
  bmap_emplace(&methods,"hasKey",&HASKEY);
  bmap_emplace(&methods,"asList",&ASLIST);
  bmap_emplace(&methods,"substr",&SUBSTR_METHOD);
  bmap_emplace(&methods,"replace",REPLACE_METHOD);
  bmap_emplace(&methods,"replace_once",REPLACE_ONCE_METHOD);

}
zobject callmethod(const char* name,zobject* args,int32_t argc)
{
    BuiltinFunc fn;
    if(!bmap_get(&methods,name,&fn))
    {
        char buffer[50];
        snprintf(buffer,50,"Error %s type has no method named %s",fullform(args[0].type),name);
        return z_err(NameError,buffer);
    }
    return fn(args,argc);
}
bool function_exists(const char* name)
{
  BuiltinFunc tmp;
  return (bmap_get(&funcs,name,&tmp));
}
