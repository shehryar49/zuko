#ifndef VM_H_
#define VM_H_
#include "plutonium.h"
typedef PltObject*(*Nativefunc)(PltObject*,int,PltObject*);
using namespace std;
struct ByteSrc
{
  short fileIndex;//index of filename in files vector
  size_t ln;
};// source of byte i.e it's filename and line number
extern std::unordered_map<size_t,ByteSrc> LineNumberTable;
//extern size_t line_num;
extern string source_code;
extern string filename;
extern vector<string> files;
extern vector<string> sources;
inline bool isNumeric(char t)
{
    if(t=='i' || t=='f' || t=='l')
        return true;
    return false;
}
void PromoteType(PltObject& a,char t)
{
     if(a.type=='i')
     {
        if(t=='l')//promote to long long int
        {
            a.type = 'l';
            a.l = (long long int)a.i;
        }
        else if(t=='f')
        {
            a.type = 'f';
            a.f = (float)a.i;
        }
     }
     else if(a.type=='f')
     {
        if(t=='l')//promote to long long int
        {
            a.type = 'l';
            a.l = (long long int)a.f;
        }
        else if(t=='i')
        {
            a.type = 'i';
            a.f = (int)a.f;
        }
        else if(t=='f')
            return;
     }
     else if(a.type=='l')
     {
        if(t=='f')//only this one is needed
        {
            a.type = 'f';
            a.f = (float)a.l;
        }
     }
}
string fullform(char t)
{
    if(t=='i')
    {
        return "Integer 32 bit";
    }
    else if(t=='l')
        return "Integer 64 bit";
    else if(t=='f')
        return "Float";
    else if(t=='s')
        return "String";
    else if(t=='j')
        return "List";
    else if(t=='m')
        return "Byte";
    else if(t=='c')
        return "Custom";
    else if(t=='b')
        return "Boolean";
    else if(t=='u')
        return "File Stream";
    else if(t=='a')
        return "Dictionary";
    else if(t=='q')
        return "Module";
    else if(t=='w')
        return "Function";
    else if(t=='v')
        return "Class";
    else if(t=='o')
        return "Class Instance";
    else if(t=='n')
        return "nil";
    else if(t=='y')
      return "Native Function";
    else if(t=='r')
      return "Native Method";
    else if(t==PLT_POINTER)
      return "Native Pointer";
    else if(t=='e')
      return "Error Object";
    else if(t=='g')
      return "Generator";
    else if(t=='z')
      return "Generator Object";
    else
        return "Unknown";
}

     #ifdef BUILD_FOR_WINDOWS
     vector<HINSTANCE> moduleHandles;//for freeing later
     #endif // BUILD_FOR_WINDOWS
     #ifdef BUILD_FOR_LINUX
     vector<void*> moduleHandles;
     #endif
struct MemInfo
{
  char type;
  bool isMarked;
};
#define GC_THRESHHOLD 4196
const char* ErrNames[] = {"TypeError","ValueError","MathError","NameError","IndexError","ArgumentError","UnknownError","FileIOError","KeyError","OverflowError","FileOpenError","FileSeekError","ImportError","ThrowError","MaxRecursionError","AccessError"};

PltList* duplicateList(PltList);
class VM
{
private:

    vector<unsigned char*> callstack;
    unsigned char* program;
    unsigned int program_size;
    //Some registers
    int i1;
    int i2;
    int i3;
    long long int l1;
    float f1;
    unsigned char m1;
    PltObject p1;
    PltObject p2;
    PltObject p3;
    PltObject p4;
    string s1;
    char c1;
    char c2;
    PltList pl1;//plutonium list 1
    PltList* pl_ptr1;//plutonium list pointer 1
    Dictionary pd1;
    Dictionary* pd_ptr1;
    unsigned char* k;
    vector<size_t> tryStackCleanup;
    vector<int> Limits = {0};
    size_t orgk = 0;
    
public:
  vector<PltObject> STACK;
  vector<func> nativeFunctions;//addresses of native functions
  //referenced in bytecode
  //this trick might increase performance by keeping only the needed functions close to
  //the VM
  vector<string> nameTable;
  PltObject* constants;
  int total_constants;//total constants stored in the array constants
  std::unordered_map<void*,MemInfo> memory;
  size_t allocated = 0;
  vector<unsigned char*> except_targ;
  vector<size_t> tryLimitCleanup;
  //int Gc_Cycles = 0;
  void load(vector<unsigned char> b)
  {
      program = new unsigned char[b.size()];
      for(int k=0;k<b.size();k+=1)
      {
          program[k] = b[k];
      }
      program_size = b.size();
  }
  void spitErr(ErrCode e,string msg,bool resolveLineNum)//used to show a runtime error
{
     if(except_targ.size()!=0)
     {
       size_t T = STACK.size() - tryStackCleanup.back();
       STACK.erase(STACK.end()-T,STACK.end());
       STACK.push_back(Plt_Err(e,msg));
       T = Limits.size() - tryLimitCleanup.back();
       Limits.erase(Limits.end()-T,Limits.end());
       k = except_targ.back();
       except_targ.pop_back();
       tryStackCleanup.pop_back();
       tryLimitCleanup.pop_back();
       return;
     }
    if(resolveLineNum)
    {
      line_num = LineNumberTable[orgk].ln;
      filename = files[LineNumberTable[orgk].fileIndex];
    }
    string type = "UnknownError";
    if(e>=1 && e<=16)
     type = ErrNames[(int)e-1];
    printf("\nFile %s\n",filename.c_str());
    printf("%s at line %ld\n",type.c_str(),line_num);
    auto it = std::find(files.begin(),files.end(),filename);
    size_t i = it-files.begin();
    source_code = sources[i];
    int l = 1;
    string line = "";
    int k = 0;
  //  printf("source_code = %s\n",source_code.c_str());
  //  printf("source size = %ld\n",source_code.size());
//    exit(0);
    while(l<=line_num)
    {
        if(source_code[k]=='\n')
            l+=1;
        else if(l==line_num)
            line+=source_code[k];
        k+=1;
        if(k>=source_code.length())
          break;
    }
    printf("%s\n",lstrip(line).c_str());
    printf("%s\n",msg.c_str());
    if(callstack.size()!=0 && e!=MAX_RECURSION_ERROR)//printing stack trace for max recursion is stupid
    {
        printf("<stack-trace>\n");
      while(callstack.size()!=0) //print stack trace
      {
          size_t L = callstack.back() - program;
          L-=1;
          while(LineNumberTable.find(L)==LineNumberTable.end())
            L-=1;//L is now the index of CALLUDF opcode now
          //which is ofcourse present in the LineNumberTable
          printf("  --by %s line %ld\n",files[LineNumberTable[L].fileIndex].c_str(),LineNumberTable[L].ln);
          callstack.pop_back();
      }
    }
    exit(0);
}
  void DoThreshholdBusiness()
  {
    if(allocated > GC_THRESHHOLD)
    {
      mark();
      collectGarbage();
    }
  }
  PltList* allocList()
  {
  //  printf("list allocation requested.\n");

    PltList* p = new PltList;
    if(!p)
    {
      printf("error allocating memory!\n");
      exit(0);
    }
    allocated+=sizeof(PltList);
    MemInfo m;
    m.type = 'j';
    m.isMarked = false;
    memory.emplace((void*)p,m);
    return p;
  }
  Generator* allocGen()
  {
    Generator* p = new Generator;
    if(!p)
    {
      printf("error allocating memory!\n");
      exit(0);
    }
    allocated+=sizeof(Generator);
    MemInfo m;
    m.type = 'z';
    m.isMarked = false;
    memory.emplace((void*)p,m);
    return p;
  }
  FileObject* allocFileObject()
  {
  //  printf("list allocation requested.\n");

    FileObject* p = new FileObject;
    if(!p)
    {
      printf("error allocating memory!\n");
      exit(0);
    }
    allocated+=sizeof(PltList);
    MemInfo m;
    m.type = 'u';
    m.isMarked = false;
    memory.emplace((void*)p,m);
    return p;
  }
  Dictionary* allocDict()
  {
//    printf("dictionary allocation requested.\n");

    Dictionary* p = new Dictionary;
    if(!p)
    {
      printf("error allocating memory!\n");
      exit(0);
    }
    allocated+=sizeof(Dictionary);
    MemInfo m;
    m.type = 'a';
    m.isMarked = false;
    memory.emplace((void*)p,m);
    return p;
  }
  void markObject(PltObject obj)
  {

    if(obj.type=='a' || obj.type=='o' || obj.type=='v' || obj.type=='c' || obj.type=='q' )
    {
      if(!memory[obj.ptr].isMarked)
      {
         Dictionary d = *(Dictionary*)obj.ptr;
         memory[obj.ptr].isMarked = true;
         for(auto e: d)
         {
           markObject(e.first);
           markObject(e.second);
         }
      }
    }
    else if(obj.type=='j')
    {
      if(!memory[obj.ptr].isMarked)
      {
         PltList d = *(PltList*)obj.ptr;
         memory[obj.ptr].isMarked = true;
         for(auto e: d)
         {
           markObject(e);
         }
      }
    }
    else if(obj.type=='z')
    {
      memory[obj.ptr].isMarked = true;
      Generator* g = (Generator*)obj.ptr;
      for(auto e: g->locals)
        markObject(e);
    }
    else if( obj.type=='u')
      memory[obj.ptr].isMarked = true;
  }
  void mark()
  {
    for(auto e: STACK)
    {
      markObject(e);
    }
  }
  void collectGarbage()
  {
   // printf("collecting garbage\n");
   // Gc_Cycles+=1;
    vector<void*> toFree;
    for(auto e: memory)
    {
      MemInfo m = e.second;
      if(m.isMarked)
        memory[e.first].isMarked = false;
      else
      {
        toFree.push_back(e.first);
      }
    }

    for(auto e: toFree)
    {
    //  printf("deleting something\n");
      char type = memory[e].type;
      if(type=='j')
      {
        delete (PltList*)e;
        allocated-=sizeof(PltList);
      }
      else if(type=='a' || type=='q' || type=='v' || type=='o')
      {
        delete (Dictionary*)e;
        allocated-=sizeof(Dictionary);
      }
      else if(type=='u')
      {
         delete (FileObject*)e;
      }
      else if(type=='z')
        delete (Generator*)e;
      else if(type=='c')
      {
      //  printf("gc handling object exposed from module\n");
        PltObject reg1;
        reg1.type = 's';
        reg1.s = ".destroy";
        Dictionary p = *(Dictionary*)e;
        PltObject dummy;
        dummy.type = 'c';
        dummy.ptr = e;
        if(p.find(reg1)!=p.end())
        {
          Method f = (Method)p[reg1].ptr;
          PltObject rr;
          f(&dummy,NULL,0,&rr);
        }
        delete (Dictionary*)e;
      }
      memory.erase(e);
    }
  }
  void scanForHeapObjects(PltObject obj,vector<void*> seen = {})
  {
    if(obj.type=='c' || obj.type=='a' || obj.type=='v' || obj.type=='o' || obj.type=='q')
    {
      if(std::find(seen.begin(),seen.end(),obj.ptr)!=seen.end() )
        return;
      seen.push_back(obj.ptr);
      allocated+=sizeof(Dictionary);
      MemInfo m;
      m.type = obj.type;
      m.isMarked = false;
      memory.emplace((void*)obj.ptr,m);
      Dictionary d = *(Dictionary*)obj.ptr;
      for(auto e: d)
      {
        char a = e.first.type;
        char b = e.second.type;
        if(a=='a' || a=='j' || a=='c' || a=='v' || a=='o' || a=='q')
        {
          scanForHeapObjects(e.first,seen);
        }
        if(b=='a' || b=='j' || b=='c' || a=='v' || a=='o' || a=='q')
          scanForHeapObjects(e.second,seen);
      }
    }
    else if(obj.type=='j')
    {
      if(std::find(seen.begin(),seen.end(),obj.ptr)!=seen.end() )
        return;
      seen.push_back(obj.ptr);
      allocated+=sizeof(PltList);
      MemInfo m;
      m.type = 'j';
      m.isMarked = false;
      memory.emplace((void*)obj.ptr,m);
      PltList l = *(PltList*)obj.ptr;
      for(auto e: l)
      {
        scanForHeapObjects(e,seen);
      }
    }
  }
  void interpret()
  {
     srand(time(0));
     k = program;
     unsigned char inst;
     vector<string> executing = {""};

     while(*k!=OP_EXIT)
     {
         inst = *k;
         switch(inst)
         {
         case LOAD_GLOBAL:
         {
           k+=1;
           memcpy(&i1,k,4);
           p1 = STACK[i1];
           STACK.push_back(p1);
           k+=3;
           break;
         }
         case INC_GLOBAL:
         {
           orgk = k - program;
           k+=1;
           memcpy(&i1,k,4);
           k+=3;
           c1 = STACK[i1].type;;
           if(c1=='i')
           {
             if(STACK[i1].i==INT_MAX)
             {
               STACK[i1].l = (long long int)INT_MAX+1;
               STACK[i1].type = 'l';
             }
             else
               STACK[i1].i+=1;
           }
           else if(c1=='l')
           {
             if(STACK[i1].l==LLONG_MAX)
             {
               spitErr(OVERFLOW_ERROR,"Error numeric overflow",true);
               continue;
             }
             STACK[i1].l+=1;
           }
           else if(c1=='f')
           {
             if(STACK[i1].f==FLT_MAX)
             {
               spitErr(OVERFLOW_ERROR,"Error numeric overflow",true);
               continue;
             }
             STACK[i1].f+=1;
           }
          else
          {
             spitErr(TYPE_ERROR,"Error cannot add numeric constant to type "+fullform(c1),true);
             continue;
          }
           break;
         }
         case LOAD:
         {
             orgk = k - program;
             k+=1;
             c1 = *k;
            // printf("loading %c\n",t);
             k+=1;
             if(c1=='j')
             {
                int listSize;
                memcpy(&listSize,k,sizeof(int));
                k+=3;
                PltList list;
                if(listSize!=0)
                {
                  list = {STACK.end()-listSize,STACK.end()};
                  STACK.erase(STACK.end()-listSize,STACK.end());
                }
                PltObject a;
                PltList* p = allocList();
                *p = list;
                a.type = 'j';
                a.ptr = (void*)p;
              //  a = list;
                STACK.push_back(a);
                DoThreshholdBusiness();
             }
             else if(c1=='a')
             {
                int DictSize;
                memcpy(&DictSize,k,sizeof(int));
                k+=3;
  //              printf("dictsize = %d\n",DictSize);
    //            printf("STACK.size = %ld\n",STACK.size());
                Dictionary dict;
                while(DictSize!=0)
                {
                   PltObject val = STACK[STACK.size()-1];
                   STACK.pop_back();
                   PltObject key = STACK[STACK.size()-1];
                   STACK.pop_back();
//                   printf("emplacing %s:  %s\n",key.PltObjectPltObjectToStr().c_str(),val.PltObjectPltObjectToStr().c_str());
                   if(dict.find(key)!=dict.end())
                   {
                       spitErr(VALUE_ERROR,"Error duplicate keys in dictionary!",true);
                       continue;
                   }
                   dict.emplace(key,val);
  //                 printf("fine\n");
                   DictSize-=1;
                }
    //            printf("here1\n");
                PltObject a;
                Dictionary* p = allocDict();

                *p = dict;
                a.ptr = (void*)p;
                a.type = 'a';
                STACK.push_back(a);
                DoThreshholdBusiness();
      //          printf("done\n");
             }
             else if(c1=='b')
             {
                 int b = *k;
                 PltObject a;
                 a.i = (bool)b;
                 a.type = 'b';
             //    printf("loaded bool %d\n",a.i);
                 STACK.push_back(a);

             }
             else if(c1=='s')
             {
                 string str = "";
                while(*k!=0)
                {
                    str+=(char)*k;
                    k+=1;
                }
                PltObject a;
                a.s = str;
                a.type = 's';
                STACK.push_back(a);
             //   printf("loaded %s\n",str.c_str());
             //   printf("will mov to %d\n",k+1);
                break;
             }
             else if(c1=='v')
             {
               memcpy(&i1,k,sizeof(int));
               k+=3;
               STACK.push_back(STACK[Limits.back()+i1]);
             }

             break;
         }
         case LOAD_CONST:
         {

           k+=1;
           memcpy(&i1,k,4);
           k+=3;
           STACK.push_back(constants[i1]);
           break;
         }
         case ASSIGN:
         {
           k+=1;
           memcpy(&i1,k,4);
           k+=3;
           p1 = STACK[STACK.size()-1];
           STACK.pop_back();
           STACK[Limits.back()+i1] = p1;
           break;
         }
         case ASSIGN_GLOBAL:
         {
           k+=1;
           memcpy(&i1,k,4);
           k+=3;
           p1 = STACK[STACK.size()-1];
           STACK.pop_back();
           STACK[i1] = p1;
           break;
         }
         case ASSIGNINDEX:
         {
           orgk = k - program;
           p1 = STACK.back();//the index
           STACK.pop_back();
           p2 = STACK.back();//the val i.e rhs
           STACK.pop_back();
           p3 = STACK.back();
           STACK.pop_back();
           if(p3.type=='j')
           {
              pl_ptr1 = (PltList*)p3.ptr;
              size_t idx = 0;
              if(p1.type=='i')
                idx = p1.i;
              else if(p1.type=='l')
                idx = p1.l;
              else
              {
                spitErr(TYPE_ERROR,"Error type "+fullform(p1.type)+" cannot be used to index list!",true);
                continue;
              }
              if(idx<0 || idx > pl_ptr1->size())
              {
                spitErr(VALUE_ERROR,"Error index "+PltObjectToStr(p1)+" out of range for list of size "+to_string(pl_ptr1->size()),true);
              continue;
              }
              (*pl_ptr1)[idx] = p2;
            }
            else if(p3.type=='a')
            {
              pd_ptr1 = (Dictionary*)p3.ptr;
              if(pd_ptr1->find(p1)==pd_ptr1->end())
              {
                spitErr(VALUE_ERROR,"Error given key is not present in dictionary!",true);
              continue;
              }
              (*pd_ptr1)[p1] = p2;
            }
            else
            {
                spitErr(TYPE_ERROR,"Error indexed assignment unsupported for type "+fullform(p3.type),true);
                continue;
            }

             break;
         }
         case CALL:
         {
               orgk = k - program;
               k+=1;
               memcpy(&i1,k,4);
               k+=4;
              int howmany = *k;
              pl1 = {STACK.end() - howmany, STACK.end()};
             p1 = nativeFunctions[i1](pl1);
             if(p1.type=='e')
             {
               spitErr((ErrCode)p1.i,p1.s,true);
               continue;
             }
             STACK.erase(STACK.end()-howmany,STACK.end());
             break;
         }
         case CALLFORVAL:
         {
               orgk = k-program;
               k+=1;
               memcpy(&i1,k,4);
               k+=4;
               i2 = *k;
             pl1 = {STACK.end() - i2, STACK.end()};
             p1 = nativeFunctions[i1](pl1);
             if(p1.type=='e')
             {

               spitErr((ErrCode)p1.i,p1.s,true);
               continue;
             }
             STACK.erase(STACK.end()-i2,STACK.end());
             STACK.push_back(p1);
             break;
         }
         case CALLMETHOD:
         {
             orgk = k - program;
             k++;
             memcpy(&i1,k,4);
             k+=4;
             string method_name = nameTable[i1];
             i2 = *k;
             pl1 = {STACK.end()-i2,STACK.end()};//args
             STACK.erase(STACK.end()-i2,STACK.end());
             p1 = STACK.back();//Parent object from which member is being called
             STACK.pop_back();
             if(p1.type=='q')
             {
               pd1 = *(Dictionary*)p1.ptr;
               p2.type= 's';
               p2.s = method_name;
               if(pd1.find(p2)==pd1.end())
               {
                 spitErr(NAME_ERROR,"Error the module has no member "+method_name+"!",true);
                 continue;
               }
               p3 = pd1[p2];
               if(p3.type!='y')
               {
                 spitErr(TYPE_ERROR,"Error member of module is not a function so cannot be called.",true);
                 continue;
               }
               Function f = (Function)p3.ptr;
               PltObject* argArr = new PltObject[pl1.size()];
               for(i3=0;i3<pl1.size();i3+=1)
                 argArr[i3] = pl1[i3];
               p4.type='n';
               f(argArr,i3,&p4);
               if(p4.type=='c' || p4.type=='j' || p4.type=='a' || p4.type=='o' || p4.type=='q' || p4.type=='v')
                 scanForHeapObjects(p4);
               delete[] argArr;
                       if(p4.type=='e')
                       {
                           //The module raised an error
                           int eCode = p4.i;
                           p4.s = method_name+"():  "+p4.s;
                           spitErr((ErrCode)eCode,p4.s,true);
                           continue;
                       }
                       if(fullform(p4.type)=="Unknown" && p4.type!='n')
                       {
                           spitErr(VALUE_ERROR,"Error invalid response from module!",true);
                           continue;
                       }
                    STACK.push_back(p4);
             }
             else if(p1.type=='c')
             {
               pd1 = *(Dictionary*)p1.ptr;
               p2.type= 's';
               p2.s = method_name;
               if(pd1.find(p2)==pd1.end())
               {
                 spitErr(NAME_ERROR,"Error the object does not have the requested method!",true);
                 continue;
               }
               p3 = pd1[p2];
               if(p3.type!='r')
               {
                 spitErr(TYPE_ERROR,"Error member of object is not a method so cannot be called.",true);
                 continue;
               }
               Method f = (Method)p3.ptr;
               PltObject* argArr = new PltObject[pl1.size()];
               for(i3=0;i3<pl1.size();i3+=1)
                 argArr[i3] = pl1[i3];
               p4.type='n';
               f(&p1,argArr,i3,&p4);
               if(p4.type=='c' || p4.type=='j' || p4.type=='a')
                 scanForHeapObjects(p4);
               delete[] argArr;
              if(p4.type=='q')
              {
                spitErr(VALUE_ERROR,"Error invalid response from module!",true);
                continue;
              }
                       if(p4.type=='e')
                       {
                           //The module raised an error
                           int eCode = p4.i;
                           p4.s = method_name+"():  "+p4.s;
                           spitErr((ErrCode)eCode,p4.s,true);
                           continue;
                       }
                       if(fullform(p4.type)=="Unknown" && p4.type!='n')
                       {
                           spitErr(VALUE_ERROR,"Error invalid response from module!",true);
                           continue;
                       }
                    STACK.push_back(p4);
             }
             else if(p1.type=='j' || p1.type=='a')
             {
               pl1.insert(pl1.begin(),p1);
               PltObject callmethod(string,const vector<PltObject>&);
               p3 = callmethod(method_name,pl1);
               if(p3.type=='e')
             {
               spitErr((ErrCode)p3.i,p3.s,true);
               continue;
             }
               STACK.push_back(p3);
             }
             else if(p1.type=='o')
             {
               Dictionary* pd_ptr1= (Dictionary*)p1.ptr;
               p2.type = 's';
               p2.s = method_name;
               if(pd_ptr1->find(p2)==pd_ptr1->end())
               {
                 p2.s ="@"+method_name;
                 if(pd_ptr1->find(p2)!=pd_ptr1->end())
                 {
                    bool flag=true;
                    p3.type='s';
                    p3.s = executing.back();
                    p3.s = p3.s.substr(0,p3.s.find('.'));
                  //  printf("A.s = %s\n",A.s.c_str());
                    if(p3.s==p1.s)
                    {

                    }
                    else
                    {
                      spitErr(NAME_ERROR,"Error "+method_name+" is private member of class instance!",true);
                      continue;
                    }
                 }
                 else
                 {
                   spitErr(NAME_ERROR,"Error class instance has no member "+method_name,true);
                   continue;
                 }
               }
               p4 = (*pd_ptr1)[p2];
               if(p4.type!='w')
               {
                 spitErr(NAME_ERROR,"Error member "+method_name+" of object is not callable!",true);
                 continue;
               }
                 if(i2+1!=p4.extra)
                 {
                   spitErr(ARGUMENT_ERROR,"Error function "+p4.s+" takes "+to_string(p4.extra-1)+" arguments,"+to_string(i2)+" given!",true);
                 continue;
                 }
                 callstack.push_back(k+1);
                if(callstack.size()>=1000)
                {
                    spitErr(MAX_RECURSION_ERROR,"Error max recursion limit 1000 reached.",true);
                    continue;
                }
                executing.push_back(p1.s+"."+p2.s);
                STACK.insert(STACK.end(),pl1.begin(),pl1.end());
                STACK.push_back(p1);
                Limits.push_back(STACK.size()-(i2+1));
                k = program + p4.i;
                continue;
             }
             else if(p1.type=='z')
             {
               Generator* g= (Generator*)p1.ptr;
               if(method_name=="isAlive")
               {
                PltObject isAlive;
                isAlive.type = 'b';
                isAlive.i = (g->state!=STOPPED);
                STACK.push_back(isAlive);
                k++;
                continue;
               }
               if(method_name!="resume")
               {
                   spitErr(NAME_ERROR,"Error generator has no member "+method_name,true);
                   continue;
               }

               if(g->state==STOPPED)
               {
                spitErr(VALUE_ERROR,"Error the generator has terminated cannot resume it!",true);
                continue;
               }
               
               if(g->state==RUNNING)
               {
                spitErr(VALUE_ERROR,"Error the generator already running cannot resume it!",true);
                continue;
               }
                callstack.push_back(k+1);
                if(callstack.size()>=1000)
                {
                    spitErr(MAX_RECURSION_ERROR,"Error max recursion limit 1000 reached.",true);
                    continue;
                }
                executing.push_back(p1.s);
                STACK.push_back(p1);
                Limits.push_back(STACK.size());
                STACK.insert(STACK.end(),g->locals.begin(),g->locals.end());
               // printf("resuming to %d\n",g->curr);
               g->state = RUNNING;
                k = program + g->curr;
                continue;
             }
             else
             {
               spitErr(TYPE_ERROR,"Error member operator '.' not supported for type "+fullform(p1.type),true);
               continue;
             }
             break;
         }
         case ASSIGNMEMB:
         {
           orgk = k - program;
           PltObject val = STACK.back();
           STACK.pop_back();
           PltObject Parent= STACK.back();
           STACK.pop_back();
           string mname = "";
           k++;
           while(*k!=0)
           {
             mname+= (char)*k;
             k++;
           }
          // k++;

           if(Parent.type!='c' && Parent.type!='o' && Parent.type!='q')
           {
             spitErr(TYPE_ERROR,"Error member operator '.' unsupported for "+fullform(Parent.type),true);
             continue;
           }
           if(Parent.type=='e')
           {
             spitErr(TYPE_ERROR,"Error member assignment unsupported for "+fullform(Parent.type),true);
             continue;
           }
            Dictionary* d = (Dictionary*)Parent.ptr;
            PltObject key = PltObjectFromString(mname);
            if(d->find(key)==d->end())
            {
              if(Parent.type=='o')
              {
                key.s = "@"+mname;
                //printf("reg.s = %s\n",reg.s.c_str());
                if(d->find(key)!=d->end())
                {
                  //private member
                  string A = executing.back();
                  A = A.substr(0,A.find('.'));
               //   printf("A = %s\n",A.c_str());
                  if(Parent.s!=A)
                  {
                    spitErr(ACCESS_ERROR,"Error cannot access private member "+mname+" of class "+Parent.s+"'s object!",true);
                  continue;
                  }
                }
                else
                {
                  spitErr(NAME_ERROR,"Error the object has no member named "+mname,true);
                  continue;
                }
              }
              else
              {
                spitErr(NAME_ERROR,"Error the object has no member named "+mname,true);
                continue;
              }

            }
            (*d)[key] = val;
            break;
         }
         case IMPORT:
         {
           orgk = k - program;
           k+=1;
           string name;
           while(*k!=0)
           {
             name+=(char)*k;
              k++;
           }
           typedef void(*mfunc)(PltObject*);
           #ifdef BUILD_FOR_WINDOWS
           name = "C:\\plutonium\\modules\\"+name+".dll";
           HINSTANCE module = LoadLibraryA(name.c_str());
           mfunc f = (mfunc)GetProcAddress(module, "init");
           if(!module)
           {
             spitErr(IMPORT_ERROR,"Error importing module "+to_string(GetLastError()),true);
           }

           #endif
           #ifdef BUILD_FOR_LINUX
           name = "/opt/plutonium/modules/"+name+".so";
           void* module = dlopen(name.c_str(),RTLD_LAZY);
           if(!module)
           {
             spitErr(IMPORT_ERROR,"Error importing module "+(std::string)(dlerror()),true);
             continue;
           }
           mfunc f = (mfunc)dlsym(module, "init");
           #endif
           if(!f)
           {
             spitErr(IMPORT_ERROR,"Error init() function not found in the module",true);
             continue;
           }

           PltObject Q;
           f(&Q);
           if(Q.type!='q')
           {
             spitErr(VALUE_ERROR,"Error module's init() should return a module object!",true);
             continue;
           }
           moduleHandles.push_back(module);
           scanForHeapObjects(Q);
           STACK.push_back(Q);
           break;
         }
         case RETURN:
         {
            k = callstack[callstack.size()-1]-1;
            callstack.pop_back();
            executing.pop_back();
            PltObject val = STACK[STACK.size()-1];
            while(STACK.size()!=Limits.back())
            {
              STACK.pop_back();
            }
            Limits.pop_back();
            STACK.push_back(val);
             break;
         }
         case YIELD:
         {
            executing.pop_back();
            PltObject val = STACK[STACK.size()-1];
            STACK.pop_back();
            PltList locals = {STACK.end()-(STACK.size()-Limits.back()),STACK.end()};
      //      printf("storing locals: ");
      //      for(auto e: locals)
        //      printf("%s  ",PltObjectToStr(e).c_str());
            STACK.erase(STACK.end()-(STACK.size()-Limits.back()),STACK.end());
            PltObject genObj = STACK.back();
            STACK.pop_back();
            Generator* g = (Generator*)genObj.ptr;
            g->locals = locals;
            g->curr = k - program+1;
            g->state = SUSPENDED;
            Limits.pop_back();
            STACK.push_back(val);
            k = callstack[callstack.size()-1]-1;
            callstack.pop_back();
             break;
         }
         case GEN_STOP:
         {
            executing.pop_back();
            PltObject val = STACK[STACK.size()-1];
            STACK.pop_back();
            PltList locals = {STACK.end()-(STACK.size()-Limits.back()),STACK.end()};
      //      printf("storing locals: ");
      //      for(auto e: locals)
        //      printf("%s  ",PltObjectToStr(e).c_str());
            STACK.erase(STACK.end()-(STACK.size()-Limits.back()),STACK.end());
            PltObject genObj = STACK.back();
            STACK.pop_back();
            Generator* g = (Generator*)genObj.ptr;
            g->locals = locals;
            g->curr = k - program+1;
            g->state = STOPPED;
            Limits.pop_back();
            STACK.push_back(val);
            k = callstack[callstack.size()-1]-1;
            callstack.pop_back();
             break;
         }
         case POP_STACK:
         {
           STACK.pop_back();
           break;
         }
         case NPOP_STACK:
         {
           k+=1;
           memcpy(&i1,k,4);
           k+=3;
           STACK.erase(STACK.end()-i1,STACK.end());
           break;
         }
         case GOTO_CALLSTACK:
         {
              k = callstack[callstack.size()-1]-1;
              callstack.pop_back();
              executing.pop_back();
              STACK.erase(STACK.begin()+Limits.back(),STACK.end());
              Limits.pop_back();
              break;
         }
         case LSHIFT:
         {
           orgk = k - program;
           p2 = STACK[STACK.size()-1];
           STACK.pop_back();
           p1 = STACK[STACK.size()-1];
           STACK.pop_back();
                        if(p1.type=='o')
             {
               pd_ptr1 = (Dictionary*)p1.ptr;
               p3.type = 's';
               p3.s = "__lshift__";
               auto it = pd_ptr1->find(p3);
               if(it!=pd_ptr1->end())
               {
                p3 = it->second;
                if(p3.type=='w' && p3.extra==2)
                {
                  callstack.push_back(k+1);
                  if(callstack.size()>=1000)
                  {
                      spitErr(MAX_RECURSION_ERROR,"Error max recursion limit 1000 reached.",true);
                      continue;
                  }
                  executing.push_back(p1.s+".__lshift__");
                  Limits.push_back(STACK.size());
                  STACK.push_back(p2);
                  STACK.push_back(p1);
                  k = program + p3.i;
                  continue;
                }
               }
                spitErr(TYPE_ERROR,"Error operator '<<' unsupported for types "+fullform(p1.type)+" and "+fullform(p2.type),true);
                continue;
             }
            if(p1.type!=p2.type)
            {
              spitErr(TYPE_ERROR,"Error operands should have same type for '<<' ",true);
              continue;
            }
            p3.type = p1.type;
            if(p1.type=='i')
            {
             p3.i = p1.i << p2.i;
            }
            else if(p1.type=='l')
            {
             p3.l = p1.l << p2.l;
            }
            else
            {
              spitErr(TYPE_ERROR,"Error operator << unsupported for type "+fullform(p1.type),true);
              continue;
            }
            STACK.push_back(p3);
           break;
         }
         case RSHIFT:
         {
           orgk = k - program;
           p2 = STACK[STACK.size()-1];
           STACK.pop_back();
           p1 = STACK[STACK.size()-1];
           STACK.pop_back();
                        if(p1.type=='o')
             {
               pd_ptr1 = (Dictionary*)p1.ptr;
               p3.type = 's';
               p3.s = "__rshift__";
               auto it = pd_ptr1->find(p3);
               if(it!=pd_ptr1->end())
               {
                p3 = it->second;
                if(p3.type=='w' && p3.extra==2)
                {
                  callstack.push_back(k+1);
                  if(callstack.size()>=1000)
                  {
                      spitErr(MAX_RECURSION_ERROR,"Error max recursion limit 1000 reached.",true);
                      continue;
                  }
                  executing.push_back(p1.s+".__rshift__");
                  Limits.push_back(STACK.size());
                  STACK.push_back(p2);
                  STACK.push_back(p1);
                  k = program + p3.i;
                  continue;
                }
               }
                spitErr(TYPE_ERROR,"Error operator '>>' unsupported for types "+fullform(p1.type)+" and "+fullform(p2.type),true);
                continue;
             }
            if(p1.type!=p2.type)
            {
              spitErr(TYPE_ERROR,"Error operands should have same type for '>>' ",true);
              continue;
            }
            p3.type = p1.type;
            if(p1.type=='i')
            {
             p3.i = p1.i >> p2.i;
            }
            else if(p1.type=='l')
            {
             p3.l = p1.l >> p2.l;
            }
            else
            {
              spitErr(TYPE_ERROR,"Error operator >> unsupported for type "+fullform(p1.type),true);
              continue;
            }
            STACK.push_back(p3);
           break;
         }
         case BITWISE_AND:
         {
           orgk = k - program;
           p2 = STACK[STACK.size()-1];
           STACK.pop_back();
           p1 = STACK[STACK.size()-1];
           STACK.pop_back();
                        if(p1.type=='o')
             {
               pd_ptr1 = (Dictionary*)p1.ptr;
               p3.type = 's';
               p3.s = "__bitwiseand__";
               auto it = pd_ptr1->find(p3);
               if(it!=pd_ptr1->end())
               {
                p3 = it->second;
                if(p3.type=='w' && p3.extra==2)
                {
                  callstack.push_back(k+1);
                  if(callstack.size()>=1000)
                  {
                      spitErr(MAX_RECURSION_ERROR,"Error max recursion limit 1000 reached.",true);
                      continue;
                  }
                  executing.push_back(p1.s+".__bitwiseand__");
                  Limits.push_back(STACK.size());
                  STACK.push_back(p2);
                  STACK.push_back(p1);
                  k = program + p3.i;
                  continue;
                }
               }
                spitErr(TYPE_ERROR,"Error operator '&' unsupported for types "+fullform(p1.type)+" and "+fullform(p2.type),true);
                continue;
             }
            if(p1.type!=p2.type)
            {
              spitErr(TYPE_ERROR,"Error operands should have same type for '&' ",true);
              continue;
            }
            p3.type = p1.type;
            if(p1.type=='i')
            {
             p3.i = p1.i & p2.i;
            }
            else if(p1.type=='l')
            {
             p3.l = p1.l & p2.l;
            }
            else
            {
              spitErr(TYPE_ERROR,"Error operator & unsupported for type "+fullform(p1.type),true);
              continue;
            }
            STACK.push_back(p3);
           break;
         }
         case BITWISE_OR:
         {
           orgk = k - program;
           p2 = STACK[STACK.size()-1];
           STACK.pop_back();
           p1 = STACK[STACK.size()-1];
           STACK.pop_back();
                        if(p1.type=='o')
             {
               pd_ptr1 = (Dictionary*)p1.ptr;
               p3.type = 's';
               p3.s = "__bitwiseor__";
               auto it = pd_ptr1->find(p3);
               if(it!=pd_ptr1->end())
               {
                p3 = it->second;
                if(p3.type=='w' && p3.extra==2)
                {
                  callstack.push_back(k+1);
                  if(callstack.size()>=1000)
                  {
                      spitErr(MAX_RECURSION_ERROR,"Error max recursion limit 1000 reached.",true);
                      continue;
                  }
                  executing.push_back(p1.s+".__bitwiseor__");
                  Limits.push_back(STACK.size());
                  STACK.push_back(p2);
                  STACK.push_back(p1);
                  k = program + p3.i;
                  continue;
                }
               }
                spitErr(TYPE_ERROR,"Error operator '|' unsupported for types "+fullform(p1.type)+" and "+fullform(p2.type),true);
                continue;
             }
            if(p1.type!=p2.type)
            {
              spitErr(TYPE_ERROR,"Error operands should have same type for '|' ",true);
              continue;
            }
            p3.type = p1.type;
            if(p1.type=='i')
            {
             p3.i = p1.i | p2.i;
            }
            else if(p1.type=='l')
            {
             p3.l = p1.l | p2.l;
            }
            else
            {
              spitErr(TYPE_ERROR,"Error operator | unsupported for type "+fullform(p1.type),true);
              continue;
            }
            STACK.push_back(p3);
           break;
         }
         case COMPLEMENT:
         {
           orgk = k - program;
           p1 = STACK.back();
           STACK.pop_back();
            if(p1.type=='o')
             {
               pd_ptr1 = (Dictionary*)p1.ptr;
               p3.type = 's';
               p3.s = "__complement__";
               auto it = pd_ptr1->find(p3);
               if(it!=pd_ptr1->end())
               {
                p3 = it->second;
                if(p3.type=='w' && p3.extra==1)
                {
                  callstack.push_back(k+1);
                  if(callstack.size()>=1000)
                  {
                      spitErr(MAX_RECURSION_ERROR,"Error max recursion limit 1000 reached.",true);
                      continue;
                  }
                  executing.push_back(p1.s+".__complement__");
                  Limits.push_back(STACK.size());
                  STACK.push_back(p1);
                  k = program + p3.i;
                  continue;
                }
               }
                spitErr(TYPE_ERROR,"Error operator '~' unsupported for type "+fullform(p1.type),true);
                continue;
             }
           if(p1.type=='i' )
           {
             p2.type= 'i';
             p2.i = ~p1.i;
             STACK.push_back(p2);
           }
           else if(p1.type=='l')
           {
             p2.type= 'l';
             p2.l = ~p1.l;
             STACK.push_back(p2);
           }
           else
           {
              spitErr(TYPE_ERROR,"Error operator '~' unsupported for type "+fullform(p1.type),true);
              continue;
           }
           break;
         }
         case XOR:
         {
           orgk = k - program;
           p2 = STACK[STACK.size()-1];
           STACK.pop_back();
           p1 = STACK[STACK.size()-1];
           STACK.pop_back();
                        if(p1.type=='o')
             {
               pd_ptr1 = (Dictionary*)p1.ptr;
               p3.type = 's';
               p3.s = "__xor__";
               auto it = pd_ptr1->find(p3);
               if(it!=pd_ptr1->end())
               {
                p3 = it->second;
                if(p3.type=='w' && p3.extra==2)
                {
                  callstack.push_back(k+1);
                  if(callstack.size()>=1000)
                  {
                      spitErr(MAX_RECURSION_ERROR,"Error max recursion limit 1000 reached.",true);
                      continue;
                  }
                  executing.push_back(p1.s+".__xor__");
                  Limits.push_back(STACK.size());
                  STACK.push_back(p2);
                  STACK.push_back(p1);
                  k = program + p3.i;
                  continue;
                }
               }
                spitErr(TYPE_ERROR,"Error operator '^' unsupported for types "+fullform(p1.type)+" and "+fullform(p2.type),true);
                continue;
             }
            if(p1.type!=p2.type)
            {
              spitErr(TYPE_ERROR,"Error operands should have same type for operator '^' ",true);
              continue;
            }
            p3.type = p1.type;
            if(p1.type=='i')
            {
             p3.i = p1.i ^ p2.i;
            }
            else if(p1.type=='l')
            {
             p3.l = p1.l ^ p2.l;
            }
            else
            {
              spitErr(TYPE_ERROR,"Error operator '^' unsupported for type "+fullform(p1.type),true);
              continue;
            }
            STACK.push_back(p3);
           break;
         }
         case ADD:
         {
             orgk = k - program;
             p2 = STACK[STACK.size()-1];
             STACK.pop_back();
             p1 = STACK[STACK.size()-1];
             STACK.pop_back();
             if(p1.type=='o')
             {
               pd_ptr1 = (Dictionary*)p1.ptr;
               p3.type = 's';
               p3.s = "__add__";
               auto it = pd_ptr1->find(p3);
               if(it!=pd_ptr1->end())
               {
                p3 = it->second;
                if(p3.type=='w' && p3.extra==2)
                {
                  callstack.push_back(k+1);
                  if(callstack.size()>=1000)
                  {
                      spitErr(MAX_RECURSION_ERROR,"Error max recursion limit 1000 reached.",true);
                      continue;
                  }
                  executing.push_back(p1.s+".__add__");
                  Limits.push_back(STACK.size());
                  STACK.push_back(p2);
                  STACK.push_back(p1);
                  k = program + p3.i;
                  continue;
                }
               }
                spitErr(TYPE_ERROR,"Error operator '+' unsupported for types "+fullform(p1.type)+" and "+fullform(p2.type),true);
                continue;
             }
             if(isNumeric(p1.type) && isNumeric(p2.type))
             {
                if(p1.type=='f' || p2.type=='f')
                    c1 = 'f';
                else if(p1.type=='l' || p2.type=='l')
                    c1 = 'l';
                else if(p1.type=='i' || p2.type=='i')
                    c1 = 'i';
                PromoteType(p1,c1);
                PromoteType(p2,c1);
             }
             else if(p1.type==p2.type)
                 c1 = p1.type;
             else
             {
                 spitErr(TYPE_ERROR,"Error operator '+' unsupported for "+fullform(p1.type)+" and "+fullform(p2.type),true);
             continue;
             }
             if(c1=='j')
             {
               p3.type = 'j';
               PltList res;
               PltList e = *(PltList*)p2.ptr;
               res = *(PltList*)p1.ptr;
               res.insert(res.end(),e.begin(),e.end());
               PltList* p = allocList();
               *p = res;
               p3.ptr = (void*)p;
               STACK.push_back(p3);
               DoThreshholdBusiness();
             }
             else if(c1=='s')
             {
                 p3.type = 's';
                 p3.s = p1.s+p2.s;
                 STACK.push_back(p3);
             }
             else if(c1=='i')
             {
               p3.type = 'i';
               if(!addition_overflows(p1.i,p1.i))
               {
                 p3.i = p1.i+p2.i;
                 STACK.push_back(p3);
                 break;
               }
               if(addition_overflows((long long int)p1.i,(long long int)p2.i))
               {
                 spitErr(OVERFLOW_ERROR,"Integer Overflow occurred",true);
                 continue;
               }
               p3.type = 'l';
               p3.l = (long long int)(p1.i) + (long long int)(p2.i);
               STACK.push_back(p3);
             }
             else if(c1=='f')
             {
               if(addition_overflows(p1.f,p2.f))
               {
                     orgk = k - program;
                     spitErr(OVERFLOW_ERROR,"Floating point overflow during addition",true);
                     continue;

               }
               p3.type = 'f';
               p3.f = p1.f+p2.f;
               STACK.push_back(p3);
             }
             else if(c1=='l')
             {
                 if(addition_overflows(p1.l,p2.l))
                 {
                     orgk = k - program;
                     spitErr(OVERFLOW_ERROR,"Error overflow during solving expression.",true);
                     continue;
                 }
                 p3.type = 'l';
                 p3.l = p1.l+p2.l;
                 STACK.push_back(p3);
                 break;
             }
             else
             {
               spitErr(TYPE_ERROR,"Error operator '+' unsupported for types "+fullform(p1.type)+" and "+fullform(p2.type),true);
             continue;
             }
             break;
         }
         case SMALLERTHAN:
         {
             orgk = k - program;
             p2 = STACK[STACK.size()-1];
             STACK.pop_back();
             p1 = STACK[STACK.size()-1];
             STACK.pop_back();
             if(isNumeric(p1.type) && isNumeric(p2.type))
             {
              if(p1.type=='f' || p2.type=='f')
                    c1 = 'f';
                else if(p1.type=='l' || p2.type=='l')
                    c1 = 'l';
                else if(p1.type=='i' || p2.type=='i')
                    c1 = 'i';
                PromoteType(p1,c1);
                PromoteType(p2,c1);
             }
             else if(p1.type=='o')
             {
               pd_ptr1 = (Dictionary*)p1.ptr;
               p3.type = 's';
               p3.s = "__smallerthan__";
               auto it = pd_ptr1->find(p3);
               if(it!=pd_ptr1->end())
               {
                p3 = it->second;
                if(p3.type=='w' && p3.extra==2)
                {
                callstack.push_back(k+1);
                if(callstack.size()>=1000)
                {
                    spitErr(MAX_RECURSION_ERROR,"Error max recursion limit 1000 reached.",true);
                    continue;
                }
                executing.push_back(p1.s+".__smallerthan__");
                Limits.push_back(STACK.size());
                STACK.push_back(p2);
                STACK.push_back(p1);
                k = program + p3.i;
                continue;
                }
               }
                spitErr(TYPE_ERROR,"Error operator '<' unsupported for types "+fullform(p1.type)+" and "+fullform(p2.type),true);
                 continue;
             }
             else
             {
                 spitErr(TYPE_ERROR,"Error operator '<' unsupported for types "+fullform(p1.type)+" and "+fullform(p2.type),true);
                 continue;
             }
             p3.type = 'b';
             if(p1.type=='i')
               p3.i = (bool)(p1.i < p2.i);
            else if(p1.type=='l')
               p3.i = (bool)(p1.l < p2.l);
            else if(p1.type=='f')
               p3.i = (bool)(p1.f < p2.f);
             STACK.push_back(p3);
             break;
         }
         case GREATERTHAN:
         {
             orgk = k - program;
             p2 = STACK[STACK.size()-1];
             STACK.pop_back();
             p1 = STACK[STACK.size()-1];
             STACK.pop_back();
              if(isNumeric(p1.type) && isNumeric(p2.type))
             {
              if(p1.type=='f' || p2.type=='f')
                    c1 = 'f';
                else if(p1.type=='l' || p2.type=='l')
                    c1 = 'l';
                else if(p1.type=='i' || p2.type=='i')
                    c1 = 'i';
                PromoteType(p1,c1);
                PromoteType(p2,c1);
             }
             else if(p1.type=='o')
             {
               pd_ptr1 = (Dictionary*)p1.ptr;
               p3.type = 's';
               p3.s = "__greaterthan__";
               auto it = pd_ptr1->find(p3);
               if(it!=pd_ptr1->end())
               {
                p3 = it->second;
                if(p3.type=='w' && p3.extra==2)
                {
                callstack.push_back(k+1);
                if(callstack.size()>=1000)
                {
                    spitErr(MAX_RECURSION_ERROR,"Error max recursion limit 1000 reached.",true);
                    continue;
                }
                executing.push_back(p1.s+".__greaterthan__");
                Limits.push_back(STACK.size());
                STACK.push_back(p2);
                STACK.push_back(p1);
                k = program + p3.i;
                continue;
                }
               }
                spitErr(TYPE_ERROR,"Error operator '>' unsupported for types "+fullform(p1.type)+" and "+fullform(p2.type),true);
                 continue;
             }
             else
             {
                 spitErr(TYPE_ERROR,"Error operator '>' unsupported for types "+fullform(p1.type)+" and "+fullform(p2.type),true);
                 continue;
             }
             p3.type = 'b';
             if(p1.type=='i')
               p3.i = (bool)(p1.i > p2.i);
            else if(p1.type=='l')
               p3.i = (bool)(p1.l > p2.l);
            else if(p1.type=='f')
               p3.i = (bool)(p1.f > p2.f);
             STACK.push_back(p3);
             //exit(0);
             break;
         }
         case SMOREQ:
         {
             orgk = k - program;
             p2 = STACK[STACK.size()-1];
             STACK.pop_back();
             p1 = STACK[STACK.size()-1];
             STACK.pop_back();
             if(isNumeric(p1.type) && isNumeric(p2.type))
             {
              if(p1.type=='f' || p2.type=='f')
                    c1 = 'f';
                else if(p1.type=='l' || p2.type=='l')
                    c1 = 'l';
                else if(p1.type=='i' || p2.type=='i')
                    c1 = 'i';
                PromoteType(p1,c1);
                PromoteType(p2,c1);
             }
             else if(p1.type=='o')
             {
               pd_ptr1 = (Dictionary*)p1.ptr;
               p3.type = 's';
               p3.s = "__smallerthaneq__";
               auto it = pd_ptr1->find(p3);
               if(it!=pd_ptr1->end())
               {
                p3 = it->second;
                if(p3.type=='w' && p3.extra==2)
                {
                callstack.push_back(k+1);
                if(callstack.size()>=1000)
                {
                                   orgk = k - program;
                    spitErr(MAX_RECURSION_ERROR,"Error max recursion limit 1000 reached.",true);
                    continue;
                }
                executing.push_back(p1.s+".__smallerthaneq__");
                Limits.push_back(STACK.size());
                STACK.push_back(p2);
                STACK.push_back(p1);
                k = program + p3.i;
                continue;
                }
               }
                                orgk = k - program;
                spitErr(TYPE_ERROR,"Error operator '<=' unsupported for types "+fullform(p1.type)+" and "+fullform(p2.type),true);
                 continue;
             }
             else
             {
                 spitErr(TYPE_ERROR,"Error operator '<=' unsupported for types "+fullform(p1.type)+" and "+fullform(p2.type),true);
                 continue;
             }
             p3.type = 'b';
             if(p1.type=='i')
               p3.i = (bool)(p1.i <= p2.i);
            else if(p1.type=='l')
               p3.i = (bool)(p1.l <= p2.l);
            else if(p1.type=='f')
               p3.i = (bool)(p1.f <= p2.f);
             STACK.push_back(p3);
             break;
         }
         case GROREQ:
         {
             orgk = k - program;
             p2 = STACK[STACK.size()-1];
             STACK.pop_back();
             p1 = STACK[STACK.size()-1];
             STACK.pop_back();
              if(isNumeric(p1.type) && isNumeric(p2.type))
             {
              if(p1.type=='f' || p2.type=='f')
                    c1 = 'f';
                else if(p1.type=='l' || p2.type=='l')
                    c1 = 'l';
                else if(p1.type=='i' || p2.type=='i')
                    c1 = 'i';
                PromoteType(p1,c1);
                PromoteType(p2,c1);
             }
             else if(p1.type=='o')
             {
               pd_ptr1 = (Dictionary*)p1.ptr;
               p3.type = 's';
               p3.s = "__greaterthaneq__";
               auto it = pd_ptr1->find(p3);
               if(it!=pd_ptr1->end())
               {
                p3 = it->second;
                if(p3.type=='w' && p3.extra==2)
                {
                callstack.push_back(k+1);
                if(callstack.size()>=1000)
                {
                                   orgk = k - program;
                    spitErr(MAX_RECURSION_ERROR,"Error max recursion limit 1000 reached.",true);
                    continue;
                }
                executing.push_back(p1.s+".__greaterthaneq__");
                Limits.push_back(STACK.size());
                STACK.push_back(p2);
                STACK.push_back(p1);
                k = program + p3.i;
                continue;
                }
               }
                                orgk = k - program;
                spitErr(TYPE_ERROR,"Error operator '>=' unsupported for types "+fullform(p1.type)+" and "+fullform(p2.type),true);
                 continue;
             }
             else
             {
                 spitErr(TYPE_ERROR,"Error operator '>=' unsupported for types "+fullform(p1.type)+" and "+fullform(p2.type),true);
                 continue;
             }
             p3.type = 'b';
             if(p1.type=='i')
               p3.i = (bool)(p1.i >= p2.i);
            else if(p1.type=='l')
               p3.i = (bool)(p1.l >= p2.l);
            else if(p1.type=='f')
               p3.i = (bool)(p1.f >= p2.f);
             STACK.push_back(p3);
             //exit(0);
             break;
         }
         case EQ:
         {
             orgk = k - program;
             PltObject p2 = STACK[STACK.size()-1];
             STACK.pop_back();
             PltObject p1 = STACK[STACK.size()-1];
             STACK.pop_back();
             if(p1.type=='i' && p2.type=='l')
                PromoteType(p1,'l');
             else if(p1.type=='l' && p2.type=='i')
                PromoteType(p2,'l');
             if(p1.type=='o' && p2.type!='n')
             {
               pd_ptr1 = (Dictionary*)p1.ptr;
               p3.type = 's';
               p3.s = "__eq__";
               auto it = pd_ptr1->find(p3);
               if(it!=pd_ptr1->end())
               {
                p3 = it->second;
                if(p3.type=='w' && p3.extra==2)
                {
                  callstack.push_back(k+1);
                  if(callstack.size()>=1000)
                  {
                      spitErr(MAX_RECURSION_ERROR,"Error max recursion limit 1000 reached.",true);
                      continue;
                  }
                  executing.push_back(p1.s+".__eq__");
                  Limits.push_back(STACK.size());
                  STACK.push_back(p2);
                  STACK.push_back(p1);
                  k = program + p3.i;
                  continue;
                }
               }
             }
             p3.type = 'b';
             p3.i = (bool)(p1==p2);
             STACK.push_back(p3);
             //exit(0);
             break;
         }
         case NOT:
         {
             PltObject a = STACK[STACK.size()-1];
             STACK.pop_back();
             if(a.type=='o')
             {
               pd_ptr1 = (Dictionary*)a.ptr;
               p3.type = 's';
               p3.s = "__not__";
               auto it = pd_ptr1->find(p3);
               if(it!=pd_ptr1->end())
               {
                p3 = it->second;
                if(p3.type=='w' && p3.extra==1)
                {
                  callstack.push_back(k+1);
                  if(callstack.size()>=1000)
                  {
                                     orgk = k - program;
                      spitErr(MAX_RECURSION_ERROR,"Error max recursion limit 1000 reached.",true);
                      continue;
                  }
                  executing.push_back(a.s+".__not__");
                  Limits.push_back(STACK.size());
                  STACK.push_back(a);
                  k = program + p3.i;
                  continue;
                }
               }
                                orgk = k - program;
                spitErr(TYPE_ERROR,"Error operator '!' unsupported for types "+fullform(p1.type)+" and "+fullform(p2.type),true);
                continue;
             }
             if(a.type!='b')
             {
                 orgk = k - program;
                 spitErr(TYPE_ERROR,"Error operator '!' unsupported for type "+fullform(a.type),true);
                 continue;
             }
             a.i = (bool)!(a.i);
             //printf("smaller than: %d\n",c.b);
             STACK.push_back(a);
             //exit(0);
             break;
         }
         case NEG:
         {

             PltObject a = STACK[STACK.size()-1];
             STACK.pop_back();
             if(a.type=='o')
             {
               pd_ptr1 = (Dictionary*)a.ptr;
               p3.type = 's';
               p3.s = "__neg__";
               auto it = pd_ptr1->find(p3);
               if(it!=pd_ptr1->end())
               {
                p3 = it->second;
                if(p3.type=='w' && p3.extra==1)
                {
                  callstack.push_back(k+1);
                  if(callstack.size()>=1000)
                  {
                                     orgk = k - program;
                      spitErr(MAX_RECURSION_ERROR,"Error max recursion limit 1000 reached.",true);
                      continue;
                  }
                  executing.push_back(p1.s+".__neg__");
                  Limits.push_back(STACK.size());
                  STACK.push_back(a);
                  k = program + p3.i;
                  continue;
                }
               }
                                orgk = k - program;
                spitErr(TYPE_ERROR,"Error operator '-' unsupported for types "+fullform(p1.type)+" and "+fullform(p2.type),true);
                continue;
             }
             if(!isNumeric(a.type))
             {
                 orgk = k - program;
                 spitErr(TYPE_ERROR,"Error unary operator '-' unsupported for type "+fullform(a.type),true);
                 exit(0);
             };
             if(a.type=='i')
             {
               if(a.i!=INT_MIN)
                 a.i = -(a.i);
                else
                {
                    a.type = 'l';
                    a.l =-(long long int)(INT_MIN);
                }
             }
             else if(a.type=='l')
             {
                if(a.l==LLONG_MIN)// taking negative of LLONG_MIN results in LLONG_MAX+1 so we have to avoid it
                {
                    orgk = k - program;
                    spitErr(OVERFLOW_ERROR,"Error negation of INT64_MIN causes overflow!",true);
                    continue;
                }
                else if(-a.l == INT_MIN)
                {
                  //  printf("here\n");
                    a.type = 'i';
                    a.i = INT_MIN;
                }
                else
                    a.l = -a.l;
             }
             else if(a.type=='f')
             {
               a.f = -a.f;
             }
             //printf("smaller than: %d\n",c.b);
             STACK.push_back(a);
             break;
             //exit(0);
         }
         case INDEX:
         {
             PltObject i = STACK[STACK.size()-1];
             STACK.pop_back();
             PltObject val = STACK[STACK.size()-1];
             STACK.pop_back();
             //printf("val.type = %c\n",val.type);

             if(val.type=='j')
             {
                 if(i.type!='i' && i.type!='l')
                 {
                    orgk = k - program;
                    spitErr(TYPE_ERROR,"Error index should be integer!",true);
                    continue;
                 }
                 PromoteType(i,'l');
                 if(i.l<0)
                 {
                     orgk = k - program;
                     spitErr(VALUE_ERROR,"Error index cannot be negative!",true);
                     continue;
                 }

                 PltList l = *(PltList*)val.ptr;
                 if(i.l>=l.size())
                 {
                     orgk = k - program;
                    spitErr(VALUE_ERROR,"Error index is out of range!",true);
                    continue;
                 }
                 STACK.push_back(l[i.i]);
                 break;
             }
             else if(val.type=='a')
             {
                 Dictionary d = *(Dictionary*)val.ptr;
                 if(d.find(i)==d.end())
                 {
                     orgk = k - program;
                     spitErr(KEY_ERROR,"Error key "+PltObjectToStr(i)+" not found in the dictionary!",true);
                     continue;
                 }
                 PltObject res = d[i];
                 STACK.push_back(res);
             }
             else if(val.type=='s')
             {
                 if(i.type!='i' && i.type!='l')
                 {
                 orgk = k - program;
                    spitErr(TYPE_ERROR,"Error index should be integer!",true);
                    continue;
                 }
                 PromoteType(i,'l');
                 if(i.l<0)
                 {
                     orgk = k - program;
                     spitErr(VALUE_ERROR,"Error index cannot be negative!",true);
                     continue;
                 }
                 string s = val.s;
                 if(i.l>=s.length())
                 {
                     orgk = k - program;
                     spitErr(VALUE_ERROR,"Error index is out of range!",true);
                 }
                 char c =s[i.l];
                 PltObject a;
                 a.type = 's';
                 s="";
                 s+=c;
                 a.s=s;
                 STACK.push_back(a);
             }
             else if(val.type=='o')
             {
               pd_ptr1 = (Dictionary*)val.ptr;
               p3.type = 's';
               p3.s = "__index__";
               auto it = pd_ptr1->find(p3);
               if(it!=pd_ptr1->end())
               {
                p3 = it->second;
                if(p3.type=='w' && p3.extra==2)
                {
                  callstack.push_back(k+1);
                  if(callstack.size()>=1000)
                  {
                                     orgk = k - program;
                      spitErr(MAX_RECURSION_ERROR,"Error max recursion limit 1000 reached.",true);
                      continue;
                  }
                  executing.push_back(val.s+".__index__");
                  Limits.push_back(STACK.size());
                  STACK.push_back(i);
                                    STACK.push_back(val);
                  k = program + p3.i;
                  continue;
                }
               }
                orgk = k - program;
                spitErr(TYPE_ERROR,"Error operator '[]' unsupported for types "+fullform(val.type)+" and "+fullform(i.type),true);
                continue;
             }
             else
             {
                orgk = k - program;
                spitErr(TYPE_ERROR,"Error operator '[]' unsupported for type "+fullform(val.type),true);
                continue;
             }
             break;
         }
         case NOTEQ:
         {

             PltObject b = STACK[STACK.size()-1];
             STACK.pop_back();
             PltObject a = STACK[STACK.size()-1];
             STACK.pop_back();

             if(a.type=='o' && b.type!='n')
             {
               pd_ptr1 = (Dictionary*)a.ptr;
               p3.type = 's';
               p3.s = "__noteq__";
               auto it = pd_ptr1->find(p3);
               if(it!=pd_ptr1->end())
               {
                p3 = it->second;
                if(p3.type=='w' && p3.extra==2)
                {
                  callstack.push_back(k+1);
                  if(callstack.size()>=1000)
                  {
                                     orgk = k - program;
                      spitErr(MAX_RECURSION_ERROR,"Error max recursion limit 1000 reached.",true);
                      continue;
                  }
                  executing.push_back(a.s+".__noteq__");
                  Limits.push_back(STACK.size());
                  STACK.push_back(b);
                  STACK.push_back(a);
                  k = program + p3.i;
                  continue;
                }
               }

             }
             PltObject c;
             c.type = 'b';
             if(a.type=='i' && b.type=='l')
                PromoteType(a,'l');
             else if(a.type=='l' && b.type=='i')
                PromoteType(b,'l');
             c.i = (bool)!(a==b);
             //printf("smaller than: %d\n",c.b);
             STACK.push_back(c);break;
             //exit(0);
         }
         case AND:
         {

             PltObject b = STACK[STACK.size()-1];
             STACK.pop_back();
             PltObject a = STACK[STACK.size()-1];
             STACK.pop_back();
             if(a.type=='o')
             {
               pd_ptr1 = (Dictionary*)a.ptr;
               p3.type = 's';
               p3.s = "__and__";
               auto it = pd_ptr1->find(p3);
               if(it!=pd_ptr1->end())
               {
                p3 = it->second;
                if(p3.type=='w' && p3.extra==2)
                {
                  callstack.push_back(k+1);
                  if(callstack.size()>=1000)
                  {
                                     orgk = k - program;
                      spitErr(MAX_RECURSION_ERROR,"Error max recursion limit 1000 reached.",true);
                      continue;
                  }
                  executing.push_back(a.s+".__and__");
                  Limits.push_back(STACK.size());
                  STACK.push_back(b);
                  STACK.push_back(a);
                  k = program + p3.i;
                  continue;
                }
               }
                                orgk = k - program;
                spitErr(TYPE_ERROR,"Error operator 'or' unsupported for types "+fullform(a.type)+" and "+fullform(b.type),true);
                continue;
             }
             PltObject c;
             if(a.type!='b' || b.type!='b')
             {
                orgk = k - program;
                spitErr(TYPE_ERROR,"Error operator 'and' unsupported for types "+fullform(a.type)+" and "+fullform(b.type),true);
         continue;
             }
             c.type = 'b';
             c.i = (bool)(a.i && b.i);
             //printf("smaller than: %d\n",c.b);
             STACK.push_back(c);break;
             //exit(0);
         }
         case IS:
         {

             PltObject b = STACK[STACK.size()-1];
             STACK.pop_back();
             PltObject a = STACK[STACK.size()-1];
             STACK.pop_back();
             PltObject c;
             if((a.type!='a' && a.type!='j' && a.type!='o' && a.type!='q' && a.type!='c') || (b.type!='a' && b.type!='j' && b.type!='o' && b.type!='q' && b.type!='c'))
             {
                orgk = k - program;
                spitErr(TYPE_ERROR,"Error operator 'is' unsupported for types "+fullform(a.type)+" and "+fullform(b.type),true);
            continue;
             }
             c.type = 'b';
             c.i = (bool)(a.ptr == b.ptr);
             STACK.push_back(c);
             break;
         }
         case OR:
         {

             PltObject b = STACK[STACK.size()-1];
             STACK.pop_back();
             PltObject a = STACK[STACK.size()-1];
             STACK.pop_back();
             if(a.type=='o')
             {
               pd_ptr1 = (Dictionary*)a.ptr;
               p3.type = 's';
               p3.s = "__or__";
               auto it = pd_ptr1->find(p3);
               if(it!=pd_ptr1->end())
               {
                p3 = it->second;
                if(p3.type=='w' && p3.extra==2)
                {
                  callstack.push_back(k+1);
                  if(callstack.size()>=1000)
                  {
                                     orgk = k - program;
                      spitErr(MAX_RECURSION_ERROR,"Error max recursion limit 1000 reached.",true);
                      continue;
                  }
                  executing.push_back(p1.s+".__or__");
                  Limits.push_back(STACK.size());
                  STACK.push_back(b);
                  STACK.push_back(a);
                  k = program + p3.i;
                  continue;
                }
               }
                                orgk = k - program;
                spitErr(TYPE_ERROR,"Error operator 'or' unsupported for types "+fullform(a.type)+" and "+fullform(b.type),true);
                continue;
             }
             if(a.type!='b' || b.type!='b')
             {
                 orgk = k - program;
                 spitErr(TYPE_ERROR,"Error operator 'or' unsupported for types "+fullform(a.type)+" and "+fullform(b.type),true);
             continue;
             }
             PltObject c;
             c.type = 'b';
             c.i = (bool)(a.i || b.i);
             //printf("smaller than: %d\n",c.b);
             STACK.push_back(c);break;
             //exit(0);
         }
         case MUL:
         {

             PltObject b = STACK[STACK.size()-1];
             STACK.pop_back();
             PltObject a = STACK[STACK.size()-1];
             STACK.pop_back();
             if(a.type=='o')
             {
               pd_ptr1 = (Dictionary*)a.ptr;
               p3.type = 's';
               p3.s = "__mul__";
               auto it = pd_ptr1->find(p3);
               if(it!=pd_ptr1->end())
               {
                p3 = it->second;
                if(p3.type=='w' && p3.extra==2)
                {
                  callstack.push_back(k+1);
                  if(callstack.size()>=1000)
                  {
                                     orgk = k - program;
                      spitErr(MAX_RECURSION_ERROR,"Error max recursion limit 1000 reached.",true);
                      continue;
                  }
                  executing.push_back(p1.s+".__mul__");
                  Limits.push_back(STACK.size());
                  STACK.push_back(b);
                  STACK.push_back(a);
                  k = program + p3.i;
                  continue;
                }
               }
                                orgk = k - program;
                spitErr(TYPE_ERROR,"Error operator '*' unsupported for types "+fullform(a.type)+" and "+fullform(b.type),true);
                continue;
             }
             PltObject c;
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
                 orgk = k - program;
                 spitErr(TYPE_ERROR,"Error operator '*' unsupported for "+fullform(a.type)+" and "+fullform(b.type),true);
         continue;
             }

             if(t=='i')
             {
               c.type = 'i';
               if(!multiplication_overflows(a.i,b.i))
               {
                 c.i = a.i*b.i;
                 STACK.push_back(c);
                 break;
               }
               orgk = k - program;
               if(multiplication_overflows((long long int)a.i,(long long int)b.i))
               {
                 spitErr(OVERFLOW_ERROR,"Overflow occurred",true);
                 continue;
               }
               c.type = 'l';
               c.l = (long long int)(a.i) * (long long int)(b.i);
               STACK.push_back(c);
             }
             else if(t=='f')
             {
               if(multiplication_overflows(a.f,b.f))
               {
                     orgk = k - program;
                     spitErr(OVERFLOW_ERROR,"Floating point overflow during multiplication",true);
                     continue;
               }
               c.type = 'f';
               c.f = a.f*b.f;
               STACK.push_back(c);
             }
             else if(t=='l')
             {
                 if(multiplication_overflows(a.l,b.l))
                 {
                     orgk = k - program;
                     spitErr(OVERFLOW_ERROR,"Error overflow during solving expression.",true);
                     continue;
                 }
                 c.type = 'l';
                 c.l = a.l*b.l;
                 STACK.push_back(c);
             }
             break;
         }
         case MEMB:
         {

             orgk = k - program;
             PltObject a = STACK[STACK.size()-1];
             STACK.pop_back();
             k+=1;
             memcpy(&i1,k,sizeof(int));
             k+=3;
             string mname = nameTable[i1];
          //   printf("%c\n",a.type);
             if(a.type!='c' && a.type!='o' && a.type!='q' && a.type!='e')
             {
               spitErr(TYPE_ERROR,"Error member operator only supported for objects!",true);
               continue;
             }
             if(a.type=='e')
             {
               if(mname=="code")
               {
                 p1.type = 'i';
                 p1.i = a.i;
               }
               else if(mname=="msg")
               {
                 p1.type='s';
                 p1.s = a.s;
               }
               else if(mname=="name")
               {
                 p1.type = 's';
                 if(a.i>=1 && a.i<=16)
                   p1.s = ErrNames[a.i-1];
                 else
                   p1.s = "UnknownError"; 
               }
               else
               {
                 spitErr(NAME_ERROR,"Error object has no member named "+mname,true);
                 continue;
               }
               STACK.push_back(p1);
               k++;
               continue;
             }
             Dictionary d = *(Dictionary*)a.ptr;
             PltObject reg;
             reg.s = mname;
             reg.type = 's';
             PltObject str = reg;
             if(d.find(str)==d.end())
             {
               if(a.type=='o')
               {
                 reg.s = "@"+mname;
                 if(d.find(reg)!=d.end())
                 {;
                   string A = executing.back();
                   A = A.substr(0,A.find('.'));
                   if(a.s!=A)
                   {
                     spitErr(ACCESS_ERROR,"Error cannot access private member "+mname+" of class "+a.s+"'s object!",true);
                     continue;
                   }
                  str.s = "@"+str.s;
                 }
                 else
                 {
                   spitErr(NAME_ERROR,"Error object has no member named "+mname,true);
                   continue;
                 }
               }
               else
               {
                 spitErr(NAME_ERROR,"Error object has no member named "+mname,true);
                 continue;
               }
             }
             PltObject ret = d[str];
             STACK.push_back(ret);
             break;
         }
         case LOAD_FUNC:
         {
           k+=1;
           int p;
           mempcpy(&p,k,sizeof(int));
           k+=4;
           int idx;
           memcpy(&idx,k,sizeof(int));
           k+=4;
           PltObject fn;
           fn.type='w';
           fn.s = nameTable[idx];
           fn.i = p;
           fn.extra = *k;
           STACK.push_back(fn);
          // printf("loaded function %s onto the stack\n",fn.s.c_str());
           break;
         }
         case LOAD_GEN:
         {
           k+=1;
           int p;
           mempcpy(&p,k,sizeof(int));
           k+=4;
           int idx;
           memcpy(&idx,k,sizeof(int));
           k+=4;
           PltObject gen;
           gen.type='g';
           gen.s = nameTable[idx];
           gen.i = p;
           gen.extra = *k;
           STACK.push_back(gen);
           break;
         }
         case BUILD_CLASS:
         {
           k+=1;
           int N;
           memcpy(&N,k,sizeof(int));
           k+=4;
           int idx;
           memcpy(&idx,k,sizeof(int));
           k+=3;
           string name = nameTable[idx];
           PltObject klass;
           klass.type = 'v';
           klass.s = name;
           Dictionary* d = allocDict();
           vector<PltObject> values;
           vector<PltObject> names;
           for(int i =1;i<=N;i++)
           {
             values.push_back(STACK.back());
             STACK.pop_back();
           }
           for(int i =1;i<=N;i++)
           {
             names.push_back(STACK.back());
             STACK.pop_back();
           }
           for(int i=0;i<N;i+=1)
           {
             d->emplace(names[i],values[i]);
           }
           klass.ptr = (void*)d;
           //
           STACK.push_back(klass);
           break;
         }
         case BUILD_DERIVED_CLASS:
         {
           orgk = k - program;
           k+=1;
           memcpy(&i1,k,sizeof(int));
           k+=4;
           int N;
           memcpy(&N,k,sizeof(int));
           k+=3;
           string name = nameTable[i1];
           PltObject klass;
           klass.type = 'v';
           klass.s = name;
           Dictionary* d = allocDict();
           vector<PltObject> values;
           vector<PltObject> names;
           for(int i =1;i<=N;i++)
           {
             values.push_back(STACK.back());
             STACK.pop_back();
           }
           for(int i =1;i<=N;i++)
           {
             names.push_back(STACK.back());
             STACK.pop_back();
           }
           PltObject baseClass = STACK.back();
           STACK.pop_back();
           if(baseClass.type!='v')
           {
             spitErr(TYPE_ERROR,"Error class can not be derived from object of non class type!",true);
             continue;
           }
           Dictionary Base= *(Dictionary*)baseClass.ptr;
           for(int i=0;i<N;i+=1)
           {
               d->emplace(names[i],values[i]);
           }
           for(auto e: Base)
           {
             PltObject q = e.first;
             if(q.s[0]=='@')
               q.s = q.s.substr(1);
             if(d->find(q)==d->end())
             {
               q.s = "@"+q.s;
               if(d->find(q)==d->end())
                 d->emplace(e.first,e.second);
             }
           }
           klass.ptr = (void*)d;
           STACK.push_back(klass);
           break;
         }
         case LOAD_NAME:
         {
           k+=1;
           memcpy(&i1,k,sizeof(int));
           k+=3;
           PltObject ret;
           ret.s = nameTable[i1];
           ret.type = 's';
           STACK.push_back(ret);
           break;
         }
         case CALLUDF:
         {
           orgk = k - program;
           PltObject fn = STACK.back();
           if(fn.type!='v' && fn.type!='w' && fn.type!='g')
           {
             spitErr(TYPE_ERROR,"Error type "+fullform(fn.type)+" not callable!",true);
             continue;
           }
           STACK.pop_back();
           k+=1;
           int N = *k;
           if(fn.type=='w')
           {
           if(N!=fn.extra)
           {
             spitErr(ARGUMENT_ERROR,"Error function "+fn.s+" takes "+to_string(fn.extra)+" arguments,"+to_string(N)+" given!",true);
           continue;
           }
           callstack.push_back(k+1);
          if(callstack.size()==1000)
          {
              spitErr(MAX_RECURSION_ERROR,"Error max recursion limit 1000 reached.",true);
              continue;
          }
          executing.push_back(fn.s);
          Limits.push_back(STACK.size()-N);
          k = program + fn.i;
          continue;
          }
          else if(fn.type=='v')
          {
            Dictionary* makeDictCopy(Dictionary);
            Dictionary* obj = makeDictCopy(*(Dictionary*)fn.ptr);
            bool callconstruct = false;
            if(obj->find(PltObjectFromString("__construct__"))!=obj->end())
            {
              PltObject construct = (*obj)[PltObjectFromString("__construct__")];
              if(construct.type!='w')
              {
                spitErr(TYPE_ERROR,"Error constructor of class "+fn.s+" is not a function!",true);
                continue;
              }
              if(construct.extra-1!=N)
              {
                spitErr(ARGUMENT_ERROR,"Error constructor of class "+fn.s+" takes "+to_string(construct.extra-1)+" arguments,"+to_string(N)+" given!",true);
              continue;
              }
              PltObject r;
              r.type = 'o';
              r.ptr = (void*)obj;
              r.s = fn.s;
              STACK.push_back(r);
              callstack.push_back(k+1);
              k = program+construct.i;
              executing.push_back(fn.s+"."+construct.s);
              STACK.insert(STACK.end()-(N+1),r);
                Limits.push_back(STACK.size()-(N+1));
              continue;
            }
            else
            {
              if(N!=0)
              {
                spitErr(ARGUMENT_ERROR,"Error constructor of class "+fn.s+" takes 0 arguments!",true);
                continue;
              }
            }
            PltObject r;
            r.type = 'o';
            r.ptr = (void*)obj;
            r.s = fn.s;
            STACK.push_back(r);
          }
          else
          {
            if(N!=fn.extra)
            {
              spitErr(ARGUMENT_ERROR,"Error generator "+fn.s+" takes "+to_string(fn.extra)+" arguments,"+to_string(N)+" given!",true);
              continue;
            }
            Generator* g = allocGen();
            g->curr = fn.i;
            g->state = SUSPENDED;
            vector<PltObject> locals = {STACK.end()-N,STACK.end()};
            STACK.erase(STACK.end()-N,STACK.end());
            g->locals = locals;
            PltObject T;
            T.type='z';
            T.ptr = g;
            STACK.push_back(T);
            DoThreshholdBusiness();
          }
          break;
         }
         case MOD:
         {

             PltObject b = STACK[STACK.size()-1];
             STACK.pop_back();
             PltObject a = STACK[STACK.size()-1];
             STACK.pop_back();
             if(a.type=='o')
             {
               pd_ptr1 = (Dictionary*)a.ptr;
               p3.type = 's';
               p3.s = "__mod__";
               auto it = pd_ptr1->find(p3);
               if(it!=pd_ptr1->end())
               {
                p3 = it->second;
                if(p3.type=='w' && p3.extra==2)
                {
                  callstack.push_back(k+1);
                  if(callstack.size()>=1000)
                  {
                                     orgk = k - program;
                      spitErr(MAX_RECURSION_ERROR,"Error max recursion limit 1000 reached.",true);
                      continue;
                  }
                  executing.push_back(a.s+".__mod__");
                  Limits.push_back(STACK.size());
                  STACK.push_back(b);
                  STACK.push_back(a);
                  k = program + p3.i;
                  continue;
                }
               }
                                orgk = k - program;
                spitErr(TYPE_ERROR,"Error operator '%' unsupported for types "+fullform(a.type)+" and "+fullform(b.type),true);
                continue;
             }
             PltObject c;
             char t;
             if(isNumeric(a.type) && isNumeric(b.type))
             {
                if(a.type=='f' || b.type=='f')
                {
                    orgk = k - program;
                    spitErr(TYPE_ERROR,"Error modulo operator % unsupported for floats!",true);
                    continue;
                }
                else if(a.type=='l' || b.type=='l')
                {
                    t = 'l';
                }
                else if(a.type=='i' || b.type=='i')
                    t = 'i';
                PromoteType(a,t);
                PromoteType(b,t);
             }
             else
             {
                 orgk = k - program;
                 spitErr(TYPE_ERROR,"Error operator '%' unsupported for "+fullform(a.type)+" and "+fullform(b.type),true);
                 continue;
             }
          //

             if(t=='i')
             {
               c.type = 'i';
               if(b.i==0)
               {
                   orgk = k - program;
                   spitErr(MATH_ERROR,"Error modulo by zero",true);
                   continue;
               }
               if ( (a.i == INT_MIN) && (b.i == -1) )
               {
                  /* handle error condition */
                  c.type = 'l';
                  c.l = (long long int)a.i % (long long int)b.i;
                  STACK.push_back(c);
               }
               c.i = a.i  % b.i;
               STACK.push_back(c);
             }
             else if(t=='l')
             {
               c.type = 'l';
               if(b.l==0)
               {
                   orgk = k - program;
                   spitErr(MATH_ERROR,"Error modulo by zero",true);
                   continue;
               }
               if ( (a.l == LLONG_MIN) && (b.l == -1) )
               {
                  orgk = k - program;
                  spitErr(OVERFLOW_ERROR,"Error modulo of INT32_MIN by -1 causes overflow!",true);
                  continue;
               }
               c.i = a.i  % b.i;
               STACK.push_back(c);
             }
             break;
         }
         case INPLACE_INC:
         {
           orgk = k - program;
           k+=1;
           memcpy(&i1,k,sizeof(int));
           k+=3;
           char t = STACK[Limits.back()+i1].type;
          if(t=='i')
          {
            if(STACK[Limits.back()+i1].i==INT_MAX)
            {
                STACK[Limits.back()+i1].l = (long long int)INT_MAX+1;
                STACK[Limits.back()+i1].type = 'l';
            }
            else
              STACK[Limits.back()+i1].i+=1;
          }
          else if(t=='l')
          {
            if(STACK[Limits.back()+i1].l==LLONG_MAX)
            {
              spitErr(OVERFLOW_ERROR,"Error numeric overflow",true);
              continue;
            }
            STACK[Limits.back()+i1].l+=1;
          }
          else if(t=='f')
          {
            if(STACK[Limits.back()+i1].f==FLT_MAX)
            {
              spitErr(OVERFLOW_ERROR,"Error numeric overflow",true);
              continue;
            }
            STACK[Limits.back()+i1].f+=1;
          }
         else
         {
            spitErr(TYPE_ERROR,"Error cannot add numeric constant to type "+fullform(t),true);
         continue;
         }
           break;
         }
         case SUB:
         {

             PltObject b = STACK[STACK.size()-1];
             STACK.pop_back();
             PltObject a = STACK[STACK.size()-1];
             STACK.pop_back();
             if(a.type=='o')
             {
               pd_ptr1 = (Dictionary*)a.ptr;
               p3.type = 's';
               p3.s = "__sub__";
               auto it = pd_ptr1->find(p3);
               if(it!=pd_ptr1->end())
               {
                p3 = it->second;
                if(p3.type=='w' && p3.extra==2)
                {
                  callstack.push_back(k+1);
                  if(callstack.size()>=1000)
                  {
                                     orgk = k - program;
                      spitErr(MAX_RECURSION_ERROR,"Error max recursion limit 1000 reached.",true);
                      continue;
                  }
                  executing.push_back(a.s+".__sub__");
                  Limits.push_back(STACK.size());
                  STACK.push_back(b);
                  STACK.push_back(a);
                  k = program + p3.i;
                  continue;
                }
               }
                                orgk = k - program;
                spitErr(TYPE_ERROR,"Error operator '-' unsupported for types "+fullform(a.type)+" and "+fullform(b.type),true);
                continue;
             }
             PltObject c;
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
                 orgk = k - program;
                 spitErr(TYPE_ERROR,"Error operator '-' unsupported for "+fullform(a.type)+" and "+fullform(b.type),true);
                 continue;
             }

          //
             if(t=='i')
             {
               c.type = 'i';
               if(!subtraction_overflows(a.i,b.i))
               {
                 c.i = a.i-b.i;
                 STACK.push_back(c);
                 break;
               }
               if(subtraction_overflows((long long int)a.i,(long long int)b.i))
               {
                 orgk = k - program;
                 spitErr(OVERFLOW_ERROR,"Overflow occurred",true);
               }
               c.type = 'l';
               c.l = (long long int)(a.i) - (long long int)(b.i);
               STACK.push_back(c);
             }
             else if(t=='f')
             {
               if(subtraction_overflows(a.f,b.f))
               {
                     orgk = k - program;
                     spitErr(OVERFLOW_ERROR,"Floating point overflow during subtraction",true);
                     continue;
               }
               c.type = 'f';
               c.f = a.f-b.f;
               STACK.push_back(c);
             }
             else if(t=='l')
             {
                 if(subtraction_overflows(a.l,b.l))
                 {
                     orgk = k - program;
                     spitErr(OVERFLOW_ERROR,"Error overflow during solving expression.",true);
                     continue;
                 }
                 c.type = 'l';
                 c.l = a.l-b.l;
                 STACK.push_back(c);
             }
             break;
         }
         case DIV:
         {
           //  printf("dividing\nSTACK.size  = %ld\n",STACK.size());

             PltObject b = STACK[STACK.size()-1];
             STACK.pop_back();
             PltObject a = STACK[STACK.size()-1];
             STACK.pop_back();
             if(a.type=='o')
             {
               pd_ptr1 = (Dictionary*)a.ptr;
               p3.type = 's';
               p3.s = "__div__";
               auto it = pd_ptr1->find(p3);
               if(it!=pd_ptr1->end())
               {
                p3 = it->second;
                if(p3.type=='w' && p3.extra==2)
                {
                  callstack.push_back(k+1);
                  if(callstack.size()>=1000)
                  {
                                     orgk = k - program;
                      spitErr(MAX_RECURSION_ERROR,"Error max recursion limit 1000 reached.",true);
                      continue;
                  }
                  executing.push_back(a.s+".__div__");
                  Limits.push_back(STACK.size());
                  STACK.push_back(b);
                  STACK.push_back(a);
                  k = program + p3.i;
                  continue;
                }
               }
                                orgk = k - program;
                spitErr(TYPE_ERROR,"Error operator '/' unsupported for types "+fullform(a.type)+" and "+fullform(b.type),true);
                continue;
             }
             PltObject c;
             //printf("a = %s\nb = %s\n",PltObjectPltObjectToStr(a).c_str(),PltObjectPltObjectToStr(b).c_str());
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
                 orgk = k - program;
                 spitErr(TYPE_ERROR,"Error operator '/' unsupported for "+fullform(a.type)+" and "+fullform(b.type),true);
                 continue;
             }
            // printf("here\nLine no. = %ld\nfilename = %s\n",LineNumberTable[k].ln,LineNumberTable[k].filename);
             //printf("filename = %s\n",filename.c_str());
          //
             //printf("t = %c\n",t);
             if(t=='i')
             {
                 if(b.i==0)
                 {
                     orgk = k - program;
                     spitErr(MATH_ERROR,"Error division by zero",true);
                     continue;

                 }
               c.type = 'i';
               if(!division_overflows(a.i,b.i))
               {
               //  printf("here\n");
                 c.i = a.i/b.i;
                 STACK.push_back(c);
                 break;
               }
               if(division_overflows((long long int)a.i,(long long int)b.i))
               {
                 orgk = k - program;
                 spitErr(OVERFLOW_ERROR,"Overflow occurred",true);
                 continue;
               }
               c.type = 'l';
               c.l = (long long int)(a.i) / (long long int)(b.i);
               STACK.push_back(c);
               break;
             }
             else if(t=='f')
             {
                 if(b.f==0)
                 {
                     orgk = k - program;
                     spitErr(MATH_ERROR,"Error division by zero",true);
                     continue;
                 }
               if(division_overflows(a.f,b.f))
               {
                     orgk = k - program;
                     spitErr(OVERFLOW_ERROR,"Floating point overflow during division",true);
                     continue;
               }
               c.type = 'f';
               c.f = a.f/b.f;
               STACK.push_back(c);
             }
             else if(t=='l')
             {
                 if(b.l==0)
                 {
                     orgk = k - program;
                     spitErr(MATH_ERROR,"Error division by zero",true);
                     continue;
                 }
                 if(division_overflows(a.l,b.l))
                 {
                     orgk = k - program;
                     spitErr(OVERFLOW_ERROR,"Error overflow during solving expression.",true);
                     continue;
                 }
                 c.type = 'l';
                 c.l = a.l*b.l;
                 STACK.push_back(c);
             }
             break;
         }
         case JMP:
         {
             k+=1;
             memcpy(&i1,k,sizeof(int));
             k+=3;
             int where = k-program+i1+1;
             k = program+where-1;
             break;
         }
         case JMPNPOPSTACK:
         {
             k+=1;
             int N;
             memcpy(&N,k,sizeof(int));
             k+=4;
             STACK.erase(STACK.end()-N,STACK.end());
             memcpy(&i1,k,sizeof(int));
             k+=3;
             int where = k-program+i1;
             k = program+where;
             break;
         }
         case JMPIF:
         {
             k+=1;
             memcpy(&i1,k,sizeof(int));
             k+=3;
             int where = k-program+i1+1;
             p1 = STACK[STACK.size()-1];
             STACK.pop_back();
             if(p1.i)
                k = program+where-1;
             else
                break;
            break;
         }
         case THROW:
         {
           orgk = k-program;
           p3 = STACK.back();//err message
           STACK.pop_back();
           p2  = STACK.back();
           STACK.pop_back();
           p1.type = 'e';
           p1.i = p2.i;
           p1.s = p3.s;
           if(except_targ.size()==0)
           {
             spitErr((ErrCode)p1.i,p1.s,true);
             continue;
           }

           k = except_targ.back();
           i1 = STACK.size()-tryStackCleanup.back();

           STACK.erase(STACK.end()-i1,STACK.end());
           i1 = Limits.size() - tryLimitCleanup.back();
           Limits.erase(Limits.end()-i1,Limits.end());
           STACK.push_back(p1);

           except_targ.pop_back();
           tryStackCleanup.pop_back();
           tryLimitCleanup.pop_back();
           continue;
           break;
         }
         case ONERR_GOTO:
         {
           k+=1;
           memcpy(&i1,k,4);
           except_targ.push_back((unsigned char*)program+i1);
           tryStackCleanup.push_back(STACK.size());
           tryLimitCleanup.push_back(Limits.size());
           k+=3;
           break;
         }
         case POP_EXCEP_TARG:
         {
           except_targ.pop_back();
           tryStackCleanup.pop_back();
           break;
         }

         case GOTO:
         {
             k+=1;
             memcpy(&i1,k,sizeof(int));
             k = program + i1-1;
             break;
         }
         case BREAK:
         {
             k+=1;
             memcpy(&i1,k,sizeof(int));
             k+=4;
             int  p = i1;
             memcpy(&i1,k,sizeof(int));
             STACK.erase(STACK.end()-i1,STACK.end());
             k = program+p;
             p1.type = 'n';
             STACK.push_back(p1);
             continue;
         }
         case CONT:
         {
             k+=1;
             int p;
             memcpy(&p,k,sizeof(int));
             k+=4;
             memcpy(&i1,k,sizeof(int));
             STACK.erase(STACK.end()-i1,STACK.end());
             k = program+p;
             continue;
         }
         case BACKJMP:
         {
             orgk = k - program;
             k+=1;
             memcpy(&i1,k,sizeof(int));
             i1 = orgk-i1;
             k = program+i1;
             continue;
         }
         case GOTONPSTACK:
         {
             k+=1;
             memcpy(&i1,k,sizeof(int));
             k+=4;
             STACK.erase(STACK.end()-i1,STACK.end());
             memcpy(&i1,k,sizeof(int));
             k = program+i1;
             continue;
         }
         case JMPIFFALSE:
         {
             k+=1;
             memcpy(&i1,k,sizeof(int));
             k+=3;
             i1 = k-program+i1+1;
             p1 = STACK[STACK.size()-1];
             STACK.pop_back();
             if(p1.type=='n')
             {
               k=program+i1;
               continue;
             }
             if(p1.i==0)
                k = program+i1-1;
             break;
         }
         default:
         {
             printf("An InternalError occurred.Error Code: 14\n");
           //  printf("k = %ld\n",k - program);
             exit(0);
             break;
         }

         }//end switch statement
         k+=1;
         //cout<<k<<endl;
     }//end while loop
     if(STACK.size()!=0)
     {
         printf("An InternalError occurred.Error Code: 15\n");
         exit(0);
     }
  }//end function interpret
  ~VM()
  {
      delete[] program;
      STACK.clear();
      mark();//clearing the STACK and marking objects will result in all objects being deleted
      //which is what we want
      collectGarbage();
      for(auto e: moduleHandles)
     {
         #ifdef BUILD_FOR_WINDOWS
            FreeLibrary(e);
         #endif
         #ifdef BUILD_FOR_LINUX
           dlclose(e);
         #endif
     }
  }
};

#endif
