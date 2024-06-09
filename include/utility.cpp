#include "utility.h"
#include "convo.h"
#include "vm.h"
#include "zobject.h"
#include <algorithm>
using namespace std;

int len(string s)
{
  return s.length()-1;
}
string substr(int x,int y,const string& s)
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
}
vector<string> split(string s,const string& x)
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
}
string IntToHex(int i)
{
    if(i==0)
        return "0x00";
    int r;
    string res = "";

    while(i!=0)
    {
        r = i%16;
        if(r<10)
        {
          res+=str((int)r);
        }
        else
        {
           res+= (char) (r+87);
        }
        i = i>>4;//i = i/16
    }
    std::reverse(res.begin(),res.end());
    if(res.length()==1)
    {
        res = "0"+res;
    }
    res = "0x"+res;
    return res;
}

unsigned char tobyte(const string& s)
{
    //s.length() is always 2

    if(s.length() != 2)
      return 0;
    char x = tolower(s[0]);
    char y = tolower(s[1]);
    unsigned char b = 0;
    b = (isdigit(y)) ? y-48 : y-87;
    b += (isdigit(x)) ? (x-48)<<4 : (x-87)<<4;
    return b;
}
int32_t hexToInt32(const string& s)
{
    int32_t res = 0;
    int32_t p = 1;
    for(int32_t i=s.length()-1;i>=0;i--)
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
int64_t hexToInt64(const string& s)
{
    int64_t res = 0;
    int64_t p = 1;
    for(int32_t i=s.length()-1;i>=0;i--)
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
string addlnbreaks(string s,bool& hadErr) // adds escape sequences
{

    unsigned int k = 0;
    bool escaped = false;//check if last char was
    string r = "";
    while(k<s.length())
    {
        if(s[k]=='\\')
        {
            if(escaped)
            {
              escaped = false;
              r+="\\";
            }
            else
              {
              escaped = true;
              }
        }
        else if(escaped)
        {
            if(s[k]=='n')
            {
                r+='\n';
            }
            else if(s[k]=='r')
            {
                r+='\r';
            }
            else if(s[k]=='t')
            {
                r+='\t';
            }
            else if(s[k]=='v')
            {
                r+='\v';
            }
            else if(s[k]=='b')
            {
                r+='\b';
            }
            else if(s[k]=='a')
            {
                r+='\a';
            }
            else if(s[k] == 'x')
            {
              if(k+2 >= s.length())
              {
                hadErr = true;
                return "Expected characters after '\\x' ";
              }
              string tmp = s.substr(k+1,2);
              unsigned char ch = tobyte(tmp);
              r += ch;
              k += 2;
            }
            else if(s[k]=='"')
            {
                r+='"';
            }
            else
            {
                hadErr = true;
                return "Unknown escape character: \\"+s.substr(k,1);
               
            }
            escaped = false;
        }
        else if(!escaped)
        {
            r+=s[k];
        }
        k+=1;
    }
   if(escaped)
   {
       hadErr = true;
       return "Error string contains non terminated escape chars";
   }
	return r;
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
string& readfile(string filename)
{
  FILE* fp = fopen(filename.c_str(), "r");
  static string src;
  src = "";
  if (!fp)
  {
      printf("Error opening file: %s\n", strerror(errno));
      exit(0);
  }
  signed char ch;
  int32_t expect = 0;//continuation bytes to expect
  while ((ch = fgetc(fp)) != EOF)
  {
      if(expect)
      {
        if(ch & 0x80) //msb is on
        {
          expect-=1;
          src += ch;
          continue;
        }
        else
        {
          printf("Error the file %s does not seem to be a utf-8 text file.\n",filename.c_str());
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
        printf("Error the file %s does not seem to be a utf-8 text file.\n",filename.c_str());
        exit(0);
      }
      src += ch;
  }
  if(expect)
  {
    printf("Error the file %s does not seem to be a utf-8 text file.\n",filename.c_str());
    exit(0);
  }
  fclose(fp);
  return src;
}
void WriteByteCode(const char* fname,vector<uint8_t>& bytecode,std::unordered_map<size_t,ByteSrc>& LineNumberTable,vector<string>& files)
{

   FILE* f = fopen(fname,"wb");
   if(!f)
   {
    printf("error writing bytecode file \n");
    return;
   }
    //write line number table
    int total = LineNumberTable.size();
    fwrite(&total,sizeof(int),1,f);

    for(auto e: LineNumberTable)
    {
      size_t Pos = e.first;
      ByteSrc src = e.second;
      fwrite(&Pos,sizeof(size_t),1,f);
      Pos = src.ln;
      fwrite(&Pos,sizeof(size_t),1,f);
      string s = files[src.fileIndex];
      int L  = s.length();
      fwrite(&L,sizeof(int),1,f);
      fwrite(s.c_str(),sizeof(char),s.length(),f);
    }
    //
   uint8_t* arr = new uint8_t[bytecode.size()];
   total = vm.total_constants;
   fwrite(&total,sizeof(int),1,f);
   for(int i=0;i<vm.total_constants;i+=1)
   {
     string s = zobjectToStr(vm.constants[i]);
    // printf("writing constant %s\n",s.c_str());
     int L = s.length();
     fwrite(&L,sizeof(int),1,f);
     fwrite(s.c_str(),sizeof(char),s.length(),f);
   }
   int sz = bytecode.size();
   fwrite(&sz,sizeof(int),1,f);
    for(size_t k=0;k<bytecode.size();k+=1)
    {
      arr[k] = bytecode[k];
    }
    fwrite(arr,sizeof(uint8_t),bytecode.size(),f);
    fclose(f);
    delete[] arr;
}

const char* getOS()
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
string replace(int startpos,int endpos,string x,string s)
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

string zobjectToStr(const zobject& a)
{
        string IntToHex(int);
        if(a.type=='i')
          return str(a.i);
        else if(a.type=='l')
          return str(a.l);
        else if(a.type=='f')
          return str(a.f);
        else if(a.type=='m')
          return IntToHex(a.i);
        else if(a.type=='c')
          return "<bytearray>";
        else if(a.type=='e')
        {
          return "<Error Object>";
        }
        else if(a.type=='b')
        {
            if(a.i)
                return "true";
            return "false";
        }
        else if(a.type=='u')
        {
            char buff[100];
            snprintf(buff,100,"<File object at %p>",a.ptr);
            string s = buff;
            return s;
        }
				else if(a.type=='w')
				{
          zfun* p = (zfun*)a.ptr;
					return "<Function "+(string)p->name +">";
				}
        else if(a.type=='y')
          return "<Native Function>";
				else if(a.type=='v')
				  return "<Class "+(string)(((zclass*)a.ptr) -> name)+">";
		    else if(a.type=='o')
				  return "<"+(string)((zclass_object*)a.ptr) -> _klass->name+" Object "+to_string((long long int)a.ptr)+" >";
        else if(a.type=='q')
        {
            char buff[100];
            snprintf(buff,100,"<Module Object at %p>",a.ptr);
            string s = buff;
            return s;
        }
        else if(a.type=='r')
        {
          return "<Native Method>";
        }
        else if(a.type=='z')
        {
          return "<Coroutine Object>";
        }
        else if(a.type=='g')
        {
          return "<Coroutine>";
        }
        else if(a.type=='j')
        {

            string zlistToStr(zlist*,vector<void*>* = nullptr);
            zlist* p = (zlist*)a.ptr;
            return zlistToStr(p);
        }
        else if(a.type==Z_STR)
        {
          zstr* str = (zstr*)a.ptr;
          return str->val;
        }
        else if(a.type=='a')
        {
						string DictToStr(zdict*,vector<void*>* = nullptr);
            zdict* p = (zdict*)a.ptr;
            return DictToStr(p);
        }
        return "nil";
    }

string zlistToStr(zlist* p,vector<void*>* prev)
{
  vector<void*> seen;
  if(prev!=nullptr)
    seen = *prev;
	seen.push_back(p);
  string res="[";
  for(size_t k=0;k<p->size;k+=1)
  {
      if(p->arr[k].type=='j')
      {
         if( std::find(seen.begin(), seen.end(), p->arr[k].ptr) != seen.end())
           res+="[...]";
         else
         {
            res+=zlistToStr((zlist*)p->arr[k].ptr,&seen);
         }
      }
			else if(p->arr[k].type=='a')
      {
				 string DictToStr(zdict*,vector<void*>* = nullptr);

         if( std::find(seen.begin(), seen.end(), p->arr[k].ptr) != seen.end())
           res+="{...}";
         else
         {
            res+=DictToStr((zdict*)p->arr[k].ptr,&seen);
         }
      }
			else if(p->arr[k].type=='s')
			{
				res+= "\""+unescape(*(string*)p->arr[k].ptr)+"\"";
			}
      else
      {
        res+=zobjectToStr(p->arr[k]);
      }
      if(k!=p->size-1)
        res+=",";
  }
  return res+"]";
}

string DictToStr(zdict* p,vector<void*>* prev)
{

    string res = "{";
    size_t k = 0;
		vector<void*> seen;
		if(prev!=nullptr)
		  seen = *prev;
		seen.push_back((void*)p);
		zdict d = *p;
    for(size_t i=0;i<d.capacity;i++)
    {
       if(d.table[i].stat != OCCUPIED)
         continue;
       
       zobject key = d.table[i].key;
       zobject val = d.table[i].val;
       if(key.type=='s')
          res+= "\""+unescape(*(string*)key.ptr)+"\"";
       else if(key.type=='a')
			 {
				 if( std::find(seen.begin(), seen.end(), key.ptr) != seen.end())
					 res+="{...}";
				 else
				 {
						res+=DictToStr((zdict*)key.ptr,&seen);
				 }
			 }
			 else if(key.type=='j')
			 {
				 if(std::find(seen.begin(),seen.end(),key.ptr)!=seen.end())
				   res+="[...]";
				 else
				   res+=zlistToStr((zlist*)key.ptr,&seen);
			 }
			 else
          res+=zobjectToStr(key);
					/////////////////
       res+=(" : ");
			 //////////////
       if(val.type=='s')
          res+= "\""+unescape(*(string*)val.ptr)+"\"";
			 else if(val.type=='a')
			 {
				 if( std::find(seen.begin(), seen.end(), val.ptr) != seen.end())
					 res+="{...}";
				 else
				 {
						res+=DictToStr((zdict*)val.ptr,&seen);
				 }
			 }
			 else if(val.type=='j')
			 {
				 if(std::find(seen.begin(),seen.end(),val.ptr)!=seen.end())
					 res+="[...]";
				 else
					 res+=zlistToStr((zlist*)val.ptr,&seen);
			 }
       else
          res+=zobjectToStr(val);
       res+=',';
       k+=1;
    }
    if(res.length() > 0)
      res.pop_back();
    res+="}";
   return res;
}