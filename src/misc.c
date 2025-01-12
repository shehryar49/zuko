#include "dyn-str.h"
#include "misc.h"
#include "convo.h"
#include "ptr-vector.h"
#include "vm.h"
#include "zobject.h"
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

/*string substr(int x,int y,const string& s)
{
  //Crash safe substr function, the std::string.substr() can cause program to crash(or throws exception whatever)
  //this function allows you to check string for subsequences without worrying about out of range issues
  //returns a string if indexes are valid
  int k = x;
	string p = "";
  int l = s.length();
	while(k<=y && k<l)
	{
		p+= s[k];
		k+=1;
	}
	return p;
}*/
/*vector<string> split(string s,const string& x)
{
	size_t k = 0;
	vector<string> list;
	while((k = s.find(x))!=std::string::npos)
	{
		list.push_back(s.substr(0,k));
		s = s.substr(k+x.length());
	}
	list.push_back(s);
	return list;
}
string lstrip(string s)
{
    while((s.length()>0) && (s[0]==' ' || s[0]=='\t'))
    {
        s = s.substr(1);
    }
    return s;
}
string replace(string x,string y,string s)//Replaces only once
{
	size_t start = s.find(x);
	if(start!=std::string::npos)
	{
		string p1 = substr(0,start-1,s);
		string p2 = substr(start+len(x)+1,len(s),s);
		string result = p1+y+p2;
		return result;
	}
	return s;
}*/
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

unsigned char tobyte(const char* s)
{
    //s.length() is always 2
    if(strlen(s) != 2)
      return 0;
    char x = tolower(s[0]);
    char y = tolower(s[1]);
    unsigned char b = 0;
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
        {
            res+= (s[i]-48) * p;
        }
        else if(s[i] >= 'a' && s[i]<='z')
        {
            res+= (s[i]-87) * p;
        }
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
        {
            res+= (s[i]-48) * p;
        }
        else if(s[i] >= 'a' && s[i]<='z')
        {
            res+= (s[i]-87) * p;
        }
        
        p<<=4;
    }
    return res;
}
char* addlnbreaks(const char* s,bool* hadErr) // adds escape sequences
{

    size_t k = 0;
    bool escaped = false;//check if last char was
    dyn_str r;
    dyn_str_init(&r);
    size_t len = strlen(s);
    while(s[k])
    {
        if(s[k]=='\\')
        {
            if(escaped)
            {
                escaped = false;
                dyn_str_push(&r,'\\');
            }
            else
            {
                escaped = true;
            }
        }
        else if(escaped)
        {
            if(s[k]=='n')
                dyn_str_push(&r,'\n');
            else if(s[k]=='r')
                dyn_str_push(&r,'\r');
            else if(s[k]=='t')
                dyn_str_push(&r,'\t');
            else if(s[k]=='v')
                dyn_str_push(&r,'\v');
            else if(s[k]=='b')
                dyn_str_push(&r,'\b');
            else if(s[k]=='a')
                dyn_str_push(&r,'\a');
            else if(s[k] == 'x')
            {
                if(k+2 >= len)
                {
                    *hadErr = true;
                    return "Expected characters after '\\x' ";
                }
                const char tmp[] = {s[k+1],s[k+2]};
                unsigned char ch = tobyte(tmp);
                dyn_str_push(&r,ch);
                k += 2;
            }
            else if(s[k]=='"')
                dyn_str_push(&r,'"');
            else
            {
                *hadErr = true;
                return "Unknown escape character";
            }
            escaped = false;
        }
        else if(!escaped)
            dyn_str_push(&r,s[k]);
        k+=1;
    }
   if(escaped)
   {
       *hadErr = true;
       return "Error string contains non terminated escape chars";
   }
	return r.arr;
}
#ifdef _WIN32
string REPL_READLINE(const char* msg)
{
  signed char ch;
  string line;
  printf("%s",msg);
  while((ch = fgetc(stdin))!='\n')
  {
    if(ch == EOF) // readline is used with REPL, so on EOF (CTRL+D) we exit
    {
      puts("");
      exit(0);
    }
    line+=ch;
  }
  return line;
}
#else
  //use GNU Readline library
  #define REPL_READLINE readline
#endif
char* readfile(const char* filename)
{
  FILE* fp = fopen(filename, "rb");
  if(!fp)
  {
      printf("Error opening file: %s\n", strerror(errno));
      exit(1);
  }
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
/*string replace(int startpos,int endpos,string x,string s)
{
  string p1,p2,p3;
  if(startpos!=0)
  {
     p1 = substr(0,startpos-1,s);
  }
  p2 = x;
  if(endpos!=len(s))
  {
    p3 = substr(endpos+1,len(s),s);
  }
  s = p1+p2+p3;
  return s;
}
string replace_all(string x,string y,string s)//replaces all x strings in s with string y
{
	int  startpos = 0;
	while((size_t)startpos<s.length())
	{
		if(s[startpos]==x[0] && substr(startpos,startpos+len(x),s)==x)
		{
		  s = replace(startpos,startpos+len(x),y,s);
		  startpos+=len(y)+1;
		  continue;
	    }
		startpos+=1;
	}
	return s;
}

string unescape(string s)
{
    //This function replaces real escape sequences with printable ones
    s = replace_all("\\","\\\\",s);
    s = replace_all("\n","\\n",s);
    s = replace_all("\r","\\r",s);
    s = replace_all("\v","\\v",s);
    s = replace_all("\t","\\t",s);
    s = replace_all("\a","\\b",s);
  	s = replace_all("\"","\\\"",s);
	  return s;
}
*/
char misc_buffer[100];

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
        return strdup("<Error Object>");
    else if(a.type=='b')
    {
        if(a.i)
            return strdup("true");
        return strdup("false");
    }
    else if(a.type=='u')
    {
        snprintf(misc_buffer,100,"<File object at %p>",a.ptr);
        return strdup(misc_buffer);
    }
    else if(a.type=='w')
    {
        zfun* p = (zfun*)a.ptr;
        char buffer[100];
        snprintf(buffer,100,"<Function %s>",p->name);
        return strdup(buffer);
    }
    else if(a.type=='y')
        return strdup("<Native Function>");
    else if(a.type=='v')
    {
        snprintf(misc_buffer,100,"<class %s>",((zclass*)a.ptr)->name);
		return strdup(misc_buffer);
    }
	else if(a.type=='o')
    {
        snprintf(misc_buffer,100,"<%s object>",((zclass*)a.ptr)->name);
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
