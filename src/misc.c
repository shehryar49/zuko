#include "dyn-str.h"
#include "misc.h"
#include "convo.h"
#include "module.h"
#include "ptr-vector.h"
#include "vm.h"
#include "zobject.h"
#include <stdio.h>
#include <ctype.h>
#include <errno.h>


void reverse_str(char* str,size_t len)
{
  size_t l = 1;
  size_t h = len -1;
  while( l < h)
  {
    char c = str[l];
    str[l] = str[h];
    str[h] = c;
    l++;
    h--;
  }
}
void replace_once(zstr* x,zstr* y,zstr* str,dyn_str* result)
{
    for(size_t i = 0; i < str->len; i++)
    {
        if(str->val[i] == x->val[0] && (i+x->len -1 < str->len) && strncmp(str->val+i,x->val,x->len) == 0)
        {
            dyn_str_append(result,y->val);
            i += x->len;
            if( i < str->len)
                dyn_str_append(result,str->val + i);
            break;
        }
        else
            dyn_str_push(result,str->val[i]);
    }
}
void replace_all(zstr* x,zstr* y,zstr* str,dyn_str* result)
{
    for(size_t i=0;i<str->len;i++)
    {
        if(str->val[i] == x->val[0] && (i + x->len - 1 < str->len) && strncmp(str->val+i,x->val,x->len) == 0)
        {
            dyn_str_append(result,y->val);
            i += x->len-1;
            continue;
        }
        else
            dyn_str_push(result,str->val[i]);
    }
}
char* int_to_hex(int i)
{
    if(i==0)
        return strdup("0x00");
    int r;
    dyn_str res;
    dyn_str_init(&res); 
    while(i!=0)
    {
        r = i%16;
        if(r<10)
          dyn_str_push(&res,48 + r);
        else
          dyn_str_push(&res,r+87);
        i = i>>4;//i = i/16
    }
    reverse_str(res.arr,res.length);
    dyn_str_prepend(&res,"0x");
    return res.arr;
}

uint8_t hex_to_uint8(const char* s)
{
    //s.length() is always 2
    if(strlen(s) != 2)
      return 0;
    char x = tolower(s[0]);
    char y = tolower(s[1]);
    uint8_t b = 0;
    b = (isdigit(y)) ? y-48 : y-87;
    b += (isdigit(x)) ? (x-48)<<4 : (x-87)<<4;
    return b;
}
int32_t hex_to_int32(const char* s)
{
    int32_t res = 0;
    int32_t p = 1;
    int len = strlen(s);
    for(int32_t i=len-1;i>=0;i--)
    {
        if(s[i] >= '0' && s[i]<='9')
            res+= (s[i]-48) * p;
        else if(s[i] >= 'a' && s[i]<='z')
            res+= (s[i]-87) * p;
        p<<=4;//p*=16
    }
    return res;
}
int64_t hex_to_int64(const char* s)
{
    int64_t res = 0;
    int64_t p = 1;
    int32_t len = strlen(s);
    for(int32_t i=len-1;i>=0;i--)
    {
        if(s[i] >= '0' && s[i]<='9')
            res+= (s[i]-48) * p;
        else if(s[i] >= 'a' && s[i]<='z')
            res+= (s[i]-87) * p;
        p<<=4;
    }
    return res;
}
#ifdef _WIN32
char* REPL_READLINE(const char* msg)
{
  signed char ch;
  printf("%s",msg);
  dyn_str line;
  dyn_str_init(&line);
  while((ch = fgetc(stdin))!='\n')
  {
    if(ch == EOF) // on EOF (CTRL+D) we exit
    {
      puts("");
      exit(0);
    }
    dyn_str_push(&line, ch);
  }
  return line.arr;
}
#else
  //use GNU Readline library
  #define REPL_READLINE readline
#endif
char* readfile(const char* filename)
{
    FILE* fp = fopen(filename, "rb");
    if(!fp)
        return NULL;
    fseek(fp,0,SEEK_END);
    size_t total = ftell(fp);
    rewind(fp);
    char* src = malloc(sizeof(char)* (total+1));
    size_t k = 0;
    signed char ch;
    int32_t expect = 0;//continuation bytes to expect
    while ((ch = fgetc(fp)) != EOF)
    {
        if(expect)
        {
            if(ch & 0x80) //msb is on
            {
                expect-=1;
                src[k++] = ch;
                continue;
            }
            else
            {
                printf("Error the file %s does not seem to be a utf-8 text file.\n",filename);
                exit(0);
            }
        }
        if(!(ch & 0x80)) //single byte codepoint
        ;
        else if((ch & 0xe0) == 0xc0) // 2 byte codepoint
            expect = 1;
        else if((ch & 0xf0) == 0xe0) // 3 byte codepoint
            expect = 2;
        else if((ch & 0xf8) == 0xf0) // 4 byte codepoint
            expect = 3;
        else
        {
            printf("Error the file %s does not seem to be a utf-8 text file.\n",filename);
            exit(0);
        }
        src[k++] = ch;
    }
    if(expect)
    {
        printf("Error the file %s does not seem to be a utf-8 text file.\n",filename);
        exit(0);
    }
    fclose(fp);
    src[total] = 0;
    return src;
}

const char* get_os_name()
{
  #ifdef __linux__
    return "Linux";
  #elif _WIN64
    return "Windows 64 bit";
  #elif _WIN32
    return "Windows 32 bit";
  #elif __FreeBSD__
    return "FreeBSD";
  #elif __APPLE__ || __MACH__
    return "Mac OSX";
  #elif __unix || __unix__
    return "Unix";
  #endif
  return "OS Unknown";
}

char misc_buffer[100];
void print_with_no_escape_seq(const char* str)
{
    size_t i = 0;
    char ch;
    putc('"',stdout);
    while((ch = str[i]))
    {
        if(ch == '\n')
            fputs("\\n",stdout);
        else if(ch == '\r')
            fputs("\\r",stdout);
        else if(ch == '\t')
            fputs("\\t",stdout);
        else if(ch == '\v')
            fputs("\\v",stdout);
        else if(ch == '\a')
            fputs("\\a",stdout);
        else if(ch == '"')
            fputs("\\\"",stdout);
        else
            putc(ch,stdout);
        i++;
    }
    putc('"',stdout);
}
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
            print_with_no_escape_seq((AS_STR(key)->val));
        else
        {
            char* tmp = zobject_to_str(key);
            printf("%s",tmp);
            free(tmp);
        }
        printf(" : ");
        if(val.type=='j')
            print_zlist((zlist*)val.ptr,seen);
        else if(val.type=='a')
            print_zdict((zdict*)val.ptr,seen);
        else if(val.type=='s')
            print_with_no_escape_seq((AS_STR(val)->val));
        else
        {
            char* tmp = zobject_to_str(val);
            printf("%s",tmp);
            free(tmp);
        }
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
            print_with_no_escape_seq(((zstr*)val.ptr)->val);
        else
        {
            char* s = zobject_to_str(val);
            printf("%s",s);
            free(s);
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
void print_zobject(zobject a)
{
    if(a.type == Z_INT)
        printf("%"PRId32,a.i);
    else if(a.type == Z_INT64)
        printf("%"PRId64,a.l);
    else if(a.type == Z_FLOAT)
    {
        char* tmp = double_to_str(a.f);
        fputs(tmp,stdout);
        free(tmp);
    }
    else if(a.type == Z_BYTE)
        printf("0x%02x",a.i & 0xff);
    else if(a.type == Z_BYTEARR)
        print_zbytearray((zbytearr*)a.ptr);
    else if(a.type == Z_ERROBJ)
        fputs("<error object>",stdout);
    else if(a.type == Z_BOOL)
        fputs((a.i) ? "true" : "false",stdout);
    else if(a.type == Z_FILESTREAM)
        fputs("<file object>",stdout);
    else if(a.type == Z_FUNC)
    {
        zfun* p = (zfun*)a.ptr;
        printf("<function %s>",p->name);
    }
    else if(a.type == Z_NATIVE_FUNC)
        fputs("<native function>",stdout);
    else if(a.type == Z_CLASS)
        printf("<class %s>",((zclass*)a.ptr)->name);
	else if(a.type == Z_OBJ)
    {
        zclass_object* tmp = (zclass_object*)a.ptr;
        zclass* c = tmp->_klass;
        printf("<%s object>",c->name);
    }
    else if(a.type == Z_MODULE)
        printf("<module %s>",((zmodule*)a.ptr)->name);
    else if(a.type == Z_COROUTINE_OBJ)
        fputs("<coroutine object>",stdout);
    else if(a.type == Z_COROUTINE)
        fputs("<coroutine>",stdout);
    else if(a.type == Z_LIST)
    {
        ptr_vector seen;
        ptr_vector_init(&seen);
        print_zlist(AS_LIST(a),&seen); 
        ptr_vector_destroy(&seen);
    }
    else if(a.type == Z_STR)
        fputs(AS_STR(a)->val,stdout);
    else if(a.type == Z_DICT)
    {
        ptr_vector seen;
        ptr_vector_init(&seen);
        print_zdict(AS_DICT(a),&seen);
        ptr_vector_destroy(&seen);
    }
    else if(a.type == Z_NIL)
        fputs("nil",stdout);
}
char* zobject_to_str(zobject a)
{
    if(a.type=='i')
        return int32_to_str(a.i);
    else if(a.type=='l')
        return int64_to_str(a.l);
    else if(a.type=='f')
        return double_to_str(a.f);
    else if(a.type=='m')
    {
        char buffer[10];
        snprintf(buffer,10,"0x%02x",a.i & 0xff);
        return strdup(buffer);
    }
    else if(a.type=='c')
        return strdup("<bytearray>");
    else if(a.type=='e')
        return strdup("<error object>");
    else if(a.type=='b')
    {
        if(a.i)
            return strdup("true");
        return strdup("false");
    }
    else if(a.type=='u')
    {
        snprintf(misc_buffer,100,"<file object at %p>",a.ptr);
        return strdup(misc_buffer);
    }
    else if(a.type=='w')
    {
        zfun* p = (zfun*)a.ptr;
        char buffer[100];
        snprintf(buffer,100,"<function %s>",p->name);
        return strdup(buffer);
    }
    else if(a.type=='y')
        return strdup("<native function>");
    else if(a.type=='v')
    {
        snprintf(misc_buffer,100,"<class %s>",((zclass*)a.ptr)->name);
		return strdup(misc_buffer);
    }
	else if(a.type=='o')
    {
        zclass_object* tmp = (zclass_object*)a.ptr;
        zclass* c = tmp->_klass;
        snprintf(misc_buffer,100,"<%s object>",c->name);
		return strdup(misc_buffer);
    }
    else if(a.type=='q')
    {
        char buff[100];
        snprintf(buff,100,"<Module Object at %p>",a.ptr);
        return strdup(buff);
    }
    else if(a.type=='r')
        return strdup("<Native Method>");
    else if(a.type=='z')
        return strdup("<Coroutine Object>");
    else if(a.type=='g')
        return strdup("<Coroutine>");
    else if(a.type=='j')
    {
        char* zlist_to_str(zlist*,ptr_vector*);
        zlist* p = (zlist*)a.ptr;
        ptr_vector seen;
        ptr_vector_init(&seen);
        char* str = zlist_to_str(p,&seen);
        ptr_vector_destroy(&seen);
        return str;
    }
    else if(a.type==Z_STR)
    {
        zstr* str = (zstr*)a.ptr;
        return strdup(str->val);
    }
    else if(a.type=='a')
    {
        char* zdict_to_str(zdict*,ptr_vector*);
        zdict* p = (zdict*)a.ptr;
        ptr_vector seen;
        ptr_vector_init(&seen);
        char* str = zdict_to_str(p,&seen);
        ptr_vector_destroy(&seen);
        return str;
    }
    return strdup("nil");
}

char* zlist_to_str(zlist* p,ptr_vector* seen)
{
  ptr_vector_push(seen,p);
  dyn_str res;
  dyn_str_init(&res);
  dyn_str_push(&res,'[');
  for(size_t k=0;k<p->size;k+=1)
  {
    if(p->arr[k].type=='j')
    {
        if(ptr_vector_search(seen,p->arr[k].ptr) != -1)
            dyn_str_append(&res,"[...]");
        else
        {
            char* str = zlist_to_str((zlist*)p->arr[k].ptr,seen);
            dyn_str_append(&res,str);
            free(str);
        }
    }
	else if(p->arr[k].type=='a')
    {
        char* dict_to_str(zdict*,ptr_vector*);
        if( ptr_vector_search(seen,p->arr[k].ptr) != -1)
            dyn_str_append(&res,"{...}");
        else
        {
            char* str = zdict_to_str((zdict*)p->arr[k].ptr,seen);
            dyn_str_append(&res,str);
            free(str);
        }
    }
    else if(p->arr[k].type=='s')
        dyn_str_append(&res,AS_STR(p->arr[k])->val); // DO UNESCAPE
    else
    {
        char* str = zobject_to_str(p->arr[k]);
        dyn_str_append(&res,str);
        free(str);
    }
    if(k!=p->size-1)
        dyn_str_push(&res,',');
  }
  dyn_str_push(&res,']');
  return res.arr;
}

char* zdict_to_str(zdict* p,ptr_vector* seen)
{
    ptr_vector_push(seen,p);
    zdict d = *p;
    size_t k = 0;
    dyn_str res;
    dyn_str_init(&res);
    dyn_str_push(&res,'{');
    for(size_t i=0;i<d.capacity;i++)
    {
        if(d.table[i].stat != OCCUPIED)
            continue;
        zobject key = d.table[i].key;
        zobject val = d.table[i].val;
        if(key.type=='s')
            dyn_str_append(&res,AS_STR(key)->val); //DO UNESCAPE
        else if(key.type=='a')
        {
            if( ptr_vector_search(seen,key.ptr) != -1)
                dyn_str_append(&res,"{...}");
            else
            {
                char* str = zdict_to_str((zdict*)key.ptr,seen);
                dyn_str_append(&res,str);
                free(str);
            }
        }
        else if(key.type=='j')
        {
            if(ptr_vector_search(seen,key.ptr) != -1)
                dyn_str_append(&res,"[...]");
            else
            {
                char* str = zlist_to_str((zlist*)key.ptr,seen);
                dyn_str_append(&res,str);
                free(str);
            }
        }
		else
        {
            char* str = zobject_to_str(key);
            dyn_str_append(&res,str);
            free(str);
        }

        dyn_str_push(&res,':');

        if(val.type=='s')
            dyn_str_append(&res,AS_STR(val)->val); // DO UNESCAPE
        else if(val.type=='a')
        {
            if( ptr_vector_search(seen,val.ptr) != -1)
                dyn_str_append(&res,"{...}");
            else
            {
                char* str = zdict_to_str((zdict*)val.ptr,seen);
                dyn_str_append(&res,str);
                free(str);
            }
        }
        else if(val.type=='j')
        {
            if(ptr_vector_search(seen,val.ptr) != -1)
                dyn_str_append(&res,"[...]");
            else
            {
                char* str = zlist_to_str((zlist*)val.ptr,seen);
                dyn_str_append(&res,str);
                free(str);
            }
        }
        else
        {
            char* str = zobject_to_str(val);
            dyn_str_append(&res,str);
            free(str);
        }
        
        dyn_str_push(&res,',');
        k+=1;
    }
    if(res.arr[res.length -1] == ',')
        res.arr[res.length - 1] = 0;
    dyn_str_push(&res,'}');
    return res.arr;
}
