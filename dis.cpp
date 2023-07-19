#define BUILD_FOR_LINUX
#include "include/opcode.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <unordered_map>
#include <string>
using namespace std;



struct Src
{
  string fname;
  size_t ln;
};
union FOO
{
  int32_t x;
  unsigned char bytes[4];
}FOO;
union FOO1
{
  long long int l;
  unsigned char bytes[8];
}FOO1;
union FOO2
{
  double f;
  unsigned char bytes[8];
}FOO2;
int main(int argc,char** argv)
{
  bool showLtable = false;;
  if(argc!=3 && argc!=2)
  {
    printf("usage: dis <bytecode file>\n");
    exit(0);
  }
  if(argc==3)
  {
  string opt = argv[2];
  if(opt=="--ltable")
    showLtable = true;
  else
  {
    printf("usage: dis <bytecode file>\n");
    exit(0);
    }
  }
  FILE* fp = fopen(argv[1],"rb");
  int total;
  fread(&total,sizeof(int),1,fp);
  std::unordered_map<size_t,Src> LineNumberTable;
  size_t ltableSz = 0;
  for(int i=1;i<=total;i+=1)
  {
    size_t Pos;
    fread(&Pos,sizeof(size_t),1,fp);
    size_t ln;
    fread(&ln,sizeof(size_t),1,fp);
    int L;
    fread(&L,sizeof(int),1,fp);
    char* str = new char[L];
    fread(str,sizeof(char),L,fp);
    ltableSz+=sizeof(size_t)*2+sizeof(int)+L;
    string fname = str;
    Src src;
    src.fname = str;
    src.ln = ln;
    LineNumberTable.emplace(Pos,src);
    delete[] str;

  }
  printf("ltableSz = %ld\n",ltableSz);
  int total_constants = 0;
  fread(&total_constants,sizeof(int),1,fp);
  vector<string> constants(total_constants);
  size_t constSz = 0;
  for(int i = 0;i<total_constants;i+=1)
  {
    int L;
    fread(&L,sizeof(int),1,fp);
    char* str = new char[L+1];
    L = fread(str,sizeof(char),L,fp);
    str[L] = 0;
    constSz+=L;
    constants[i] = str;
    delete[] str;
  }
  printf("constants size = %ld\n",constSz);
  int program_size;
  fread(&program_size,sizeof(int),1,fp);
  printf("program_size = %d\n",program_size);
  unsigned char* program = new unsigned char[program_size];
  fread(program,sizeof(unsigned char),program_size,fp);
  fclose(fp);
  int k = 26;
  unsigned char inst;
     printf("0-25  <init Error Classes>\n");
     while(k<program_size)
     {
         inst = program[k];
         if(showLtable && LineNumberTable.find(k)!=LineNumberTable.end())
         {
           printf("%s line %ld ",LineNumberTable[k].fname.c_str(),LineNumberTable[k].ln);
         }
         if(inst==LOAD)
         {
             printf("%d  LOAD ",k);
             k+=1;
             char t = program[k];
             k+=1;
             printf("%c",t);
             if(t=='i' || t=='j' || t=='a' || t=='v')
             {
                int M;
                memcpy(&M,program+k,sizeof(int32_t));
                printf("  %d\n",M);
                k+=4;
             }
             else if(t=='l')
             {

                FOO1.bytes[0] = program[k];
                FOO1.bytes[1] = program[k+1];
                FOO1.bytes[2] = program[k+2];
                FOO1.bytes[3] = program[k+3];
                FOO1.bytes[4] = program[k+4];
                FOO1.bytes[5] = program[k+5];
                FOO1.bytes[6] = program[k+6];
                printf("  %lld\n",FOO1.l);
                k+=8;
             }
             else if(t=='f')
             {
                FOO2.bytes[0] = program[k];
                FOO2.bytes[1] = program[k+1];
                FOO2.bytes[2] = program[k+2];
                FOO2.bytes[3] = program[k+3];
                printf("  %f\n",FOO2.f);
                k+=4;
             }
             else if(t=='b')
             {
                printf("  %d\n",program[k]);
                k+=1;
             }

             else if(t=='m')
             {
                printf("  %d\n",program[k]);
                k+=1;
             }
             else if(t=='v')
             {
                 string name = "";
                 while(program[k]!=0)
                 {
                     name+= (char)program[k];
                     k+=1;
                 }
                 k+=1;
                printf("  %s\n",name.c_str());
             }
             else if(t=='s')
             {
                 string name = "";
                 while(program[k]!=0)
                 {
                     name+= (char)program[k];
                     k+=1;
                 }
                 k+=1;
                printf("  %s\n",name.c_str());
             }
             continue;
         }
         else if(inst==OP_EXIT)
         {
           printf("%d  OP_EXIT\n",k);

         }
         
         else if(inst==LOAD_CONST)
         {
           printf("%d  LOAD_CONST ",k);
           k+=1;
           FOO.bytes[0] = program[k];
           k+=1;
           FOO.bytes[1] = program[k];
           k+=1;
           FOO.bytes[2] = program[k];
           k+=1;
           FOO.bytes[3] = program[k];
           printf("%d(%s)\n",FOO.x,(constants[FOO.x]).c_str());
         }
         
         else if(inst==LOAD_STR)
         {
           printf("%d  LOAD_STR ",k);
           k+=1;
           FOO.bytes[0] = program[k];
           k+=1;
           FOO.bytes[1] = program[k];
           k+=1;
           FOO.bytes[2] = program[k];
           k+=1;
           FOO.bytes[3] = program[k];
           printf("%d\n",FOO.x);
         }
         else if(inst==BUILD_CLASS)
         {
           int orgk = k;
                        k+=1;
           FOO.bytes[0] = program[k];
                         k+=1;
           FOO.bytes[1] = program[k];
                         k+=1;
           FOO.bytes[2] = program[k];
                         k+=1;
           FOO.bytes[3] = program[k];
           int where = FOO.x;
                         k+=1;
           FOO.bytes[0] = program[k];
                         k+=1;
           FOO.bytes[1] = program[k];
                         k+=1;
           FOO.bytes[2] = program[k];
           k+=1;
           FOO.bytes[3] = program[k];
          int n = FOO.x;
          printf("%d  BUILD_CLASS  %d  %d\n",orgk,where,n);
          k+=1;
          continue;
         }
         else if(inst==BUILD_DERIVED_CLASS)
         {
           int orgk = k;
                        k+=1;
           FOO.bytes[0] = program[k];
                         k+=1;
           FOO.bytes[1] = program[k];
                         k+=1;
           FOO.bytes[2] = program[k];
                         k+=1;
           FOO.bytes[3] = program[k];
           int where = FOO.x;
                         k+=1;
           FOO.bytes[0] = program[k];
                         k+=1;
           FOO.bytes[1] = program[k];
                         k+=1;
           FOO.bytes[2] = program[k];
           k+=1;
           FOO.bytes[3] = program[k];
          int n = FOO.x;
          printf("%d  BUILD_DERIVED_CLASS  %d  %d\n",orgk,where,n);
          k+=1;
          continue;
         }
         else if(inst==NPOP_STACK)
         {
           printf("%d  NPOP_STACK ",k);
           k+=1;
           FOO.bytes[0] = program[k];
           k+=1;
           FOO.bytes[1] = program[k];
           k+=1;
           FOO.bytes[2] = program[k];
           k+=1;
           FOO.bytes[3] = program[k];
           printf("%d\n",FOO.x);
         }

         else if(inst==INPLACE_INC)
         {
           printf("%d  INPLACE_INC ",k);
           k+=1;
           FOO.bytes[0] = program[k];
           k+=1;
           FOO.bytes[1] = program[k];
           k+=1;
           FOO.bytes[2] = program[k];
           k+=1;
           FOO.bytes[3] = program[k];
           printf("%d\n",FOO.x);
         }
         else if(inst==LOAD_BYTE)
         {
           printf("%d  LOAD_BYTE %d\n",k,program[++k]);
         }
         else if(inst==POP_STACK)
         {
           printf("%d  POP_STACK\n",k);

         }
         else if(inst==POP_EXCEP_TARG)
         {
           printf("%d  POP_EXCEPT_TARG\n",k);

         }
         else if(inst==CONT)
         {
           int orgk = k;
                        k+=1;
           FOO.bytes[0] = program[k];
                         k+=1;
           FOO.bytes[1] = program[k];
                         k+=1;
           FOO.bytes[2] = program[k];
                         k+=1;
           FOO.bytes[3] = program[k];
           int where = FOO.x;
                         k+=1;
           FOO.bytes[0] = program[k];
                         k+=1;
           FOO.bytes[1] = program[k];
                         k+=1;
           FOO.bytes[2] = program[k];
           k+=1;
           FOO.bytes[3] = program[k];
          int n = FOO.x;
          printf("%d  CONT  %d  %d\n",orgk,where,n);
          k+=1;
          continue;
         }
         else if(inst==CALLMETHOD)
         {
           k+=1;
           printf("%d  CALLMETHOD  ",k-1);
           FOO.bytes[0] = program[k];
            k+=1;
            FOO.bytes[1] = program[k];
            k+=1;
            FOO.bytes[2] = program[k];
            k+=1;
            FOO.bytes[3] = program[k];
            k+=1;
            int N = program[k];
            printf("%d  %d\n",FOO.x,N);
         }
         else if(inst==LOAD_GLOBAL)
         {
           printf("%d  LOAD_GLOBAL ",k);
           k+=1;
           FOO.bytes[0] = program[k];
           k+=1;
           FOO.bytes[1] = program[k];
           k+=1;
           FOO.bytes[2] = program[k];
           k+=1;
           FOO.bytes[3] = program[k];
           printf("%d\n",FOO.x);
         }
         else if(inst==ONERR_GOTO)
         {
           printf("%d  ONERR_GOTO ",k);
           k+=1;
           FOO.bytes[0] = program[k];
           k+=1;
           FOO.bytes[1] = program[k];
           k+=1;
           FOO.bytes[2] = program[k];
           k+=1;
           FOO.bytes[3] = program[k];
           printf("%d\n",FOO.x);
         }
         else if(inst==IMPORT)
         {
           printf("%d  IMPORT ",k);
           k+=1;
           FOO.bytes[0] = program[k];
           k+=1;
           FOO.bytes[1] = program[k];
           k+=1;
           FOO.bytes[2] = program[k];
           k+=1;
           FOO.bytes[3] = program[k];
           printf("%d\n",FOO.x);
         }
         else if(inst==ASSIGN)
         {
           printf("%d  ASSIGN ",k);
           k+=1;
           FOO.bytes[0] = program[k];
           k+=1;
           FOO.bytes[1] = program[k];
           k+=1;
           FOO.bytes[2] = program[k];
           k+=1;
           FOO.bytes[3] = program[k];
           printf("%d\n",FOO.x);
         }
         else if(inst==ASSIGN_GLOBAL)
         {
           printf("%d  ASSIGN_GLOBAL ",k);
           k+=1;
           FOO.bytes[0] = program[k];
           k+=1;
           FOO.bytes[1] = program[k];
           k+=1;
           FOO.bytes[2] = program[k];
           k+=1;
           FOO.bytes[3] = program[k];
           printf("%d\n",FOO.x);
         }

         else if(inst==ASSIGNMEMB)
         {
             int orgk = k;
                           k+=1;
              FOO.bytes[0] = program[k];
                            k+=1;
              FOO.bytes[1] = program[k];
                            k+=1;
              FOO.bytes[2] = program[k];
                            k+=1;
              FOO.bytes[3] = program[k];
             int where = FOO.x;
             printf("%d  ASSIGNMEMB  %d\n",orgk,where);
         }
         else if(inst==ASSIGNINDEX)
         {
           printf("%d  ASSIGNINDEX\n",k);
         }
         else if(inst==RETURN)
         {
             printf("%d  RETURN\n",k);
         }
         else if(inst==YIELD)
         {
             printf("%d  YIELD\n",k);
         }
         else if(inst==YIELD_AND_EXPECTVAL)
         {
             printf("%d  YIELD_AND_EXPECTVAL\n",k);
         }
         else if(inst==MEMB)
         {
             printf("%d  MEMB  ",k);
             k+=1;
             FOO.bytes[0] = program[k++];
             FOO.bytes[1] = program[k++];
             FOO.bytes[2] = program[k++];
             FOO.bytes[3] = program[k];
             printf("%d\n",FOO.x);
         }
         else if(inst==SELFMEMB)
         {
             printf("%d  SELFMEMB  ",k);
             k+=1;
             FOO.bytes[0] = program[k++];
             FOO.bytes[1] = program[k++];
             FOO.bytes[2] = program[k++];
             FOO.bytes[3] = program[k];
             printf("%d\n",FOO.x);
         }
         else if(inst==ASSIGNSELFMEMB)
         {
             printf("%d  ASSIGNSELFMEMB  ",k);
             k+=1;
             FOO.bytes[0] = program[k++];
             FOO.bytes[1] = program[k++];
             FOO.bytes[2] = program[k++];
             FOO.bytes[3] = program[k];
             printf("%d\n",FOO.x);
         }
         else if(inst==LOAD_LOCAL)
         {
             printf("%d  LOAD_LOCAL  ",k);
             k+=1;
             FOO.bytes[0] = program[k++];
             FOO.bytes[1] = program[k++];
             FOO.bytes[2] = program[k++];
             FOO.bytes[3] = program[k];
             printf("%d\n",FOO.x);
         }

         else if(inst==NEG)
         {
             printf("%d  NEG\n",k);
         }
         else if(inst==XOR)
         {
             printf("%d  ^\n",k);
         }

         else if(inst==NOT)
         {
             printf("%d  NOT\n",k);
         }
         else if(inst==SMOREQ)
         {
             printf("%d  <=\n",k);
         }
         else if(inst==GROREQ)
         {
             printf("%d  >=\n",k);
         }
         else if(inst==SMALLERTHAN)
         {
             printf("%d  <\n",k);
         }
         else if(inst==GREATERTHAN)
         {
             printf("%d  >\n",k);
         }
         else if(inst==BITWISE_AND)
         {
             printf("%d  &\n",k);
         }
         else if(inst==IS)
         {
             printf("%d  IS\n",k);
         }
         else if(inst==BITWISE_OR)
         {
             printf("%d  |\n",k);
         }
         else if(inst==RSHIFT)
         {
             printf("%d  >>\n",k);
         }
         else if(inst==LSHIFT)
         {
             printf("%d  <<\n",k);
         }
         else if(inst==COMPLEMENT)
         {
             printf("%d  >\n",k);
         }
         else if(inst==INDEX)
         {
             printf("%d  INDEX\n",k);
         }
         else if(inst==CALL)
         {
             printf("%d  CALL ",k);
             k+=1;
             FOO.bytes[0] = program[k];
             k+=1;
             FOO.bytes[1] = program[k];
             k+=1;
             FOO.bytes[2] = program[k];
             k+=1;
             FOO.bytes[3] = program[k];
             k+=1;
                int howmany = program[k];
                k+=1;
                printf("%d  %d\n",FOO.x,howmany);
             continue;
         }
         else if(inst==CALLFORVAL)
         {
            printf("%d  CALLFORVAL ",k);
            k+=1;
            FOO.bytes[0] = program[k];
            k+=1;
            FOO.bytes[1] = program[k];
            k+=1;
            FOO.bytes[2] = program[k];
            k+=1;
            FOO.bytes[3] = program[k];
            k+=1;
            int howmany = program[k];
            k+=1;
            printf("%d  %d\n",FOO.x,howmany);
            continue;
         }
         else if(inst==CALLUDF)
         {
           k+=1;
           int N = program[k];
           printf("%d  CALLUDF  %d\n",k-1,N);
         }
         else if(inst==THROW)
         {
             printf("%d  THROW\n",k);
         }
         
         else if(inst==INC_GLOBAL)
         {
           printf("%d  INC_GLOBAL ",k);
           k+=1;
           FOO.bytes[0] = program[k];
           k+=1;
           FOO.bytes[1] = program[k];
           k+=1;
           FOO.bytes[2] = program[k];
           k+=1;
           FOO.bytes[3] = program[k];
           printf("%d\n",FOO.x);
         }
         else if(inst==ADD)
         {
             printf("%d  ADD\n",k);
         }
         else if(inst==EQ)
         {
             printf("%d  EQ\n",k);
         }
         else if(inst==MUL)
         {
            printf("%d  MUL\n",k);
         }
         else if(inst==MOD)
         {
            printf("%d  MOD\n",k);
         }
         else if(inst==SUB)
         {
            printf("%d  SUB\n",k);
         }
         else if(inst==DIV)
         {
            printf("%d  DIV\n",k);
         }
         else if(inst==SMALLERTHAN)
         {
             printf("%d  SMALLER_THAN\n",k);
         }
         else if(inst==NOTEQ)
         {
            printf("%d  NOTEQ\n",k);
         }
         else if(inst==LOAD_NIL)
         {
            printf("%d  LOAD_NIL\n",k);
         }
         else if(inst==LOAD_TRUE)
         {
            printf("%d  LOAD_TRUE\n",k);
         }
         else if(inst==LOAD_FALSE)
         {
            printf("%d  LOAD_FALSE\n",k);
         }
         else if(inst==JMP)
         {
int orgk = k;
                           k+=1;
              FOO.bytes[0] = program[k];
                            k+=1;
              FOO.bytes[1] = program[k];
                            k+=1;
              FOO.bytes[2] = program[k];
                            k+=1;
              FOO.bytes[3] = program[k];
             int where = FOO.x;
             printf("%d  JMP  %d\n",orgk,where);
             k+=1;
             continue;
         }
         else if(inst==LOAD_INT32)
         {
int orgk = k;
                           k+=1;
              FOO.bytes[0] = program[k];
                            k+=1;
              FOO.bytes[1] = program[k];
                            k+=1;
              FOO.bytes[2] = program[k];
                            k+=1;
              FOO.bytes[3] = program[k];
             int where = FOO.x;
             printf("%d  LOAD_INT32  %d\n",orgk,where);
             k+=1;
             continue;
         }
         else if(inst==JMPNPOPSTACK)
         {
int orgk = k;
                           k+=1;
              FOO.bytes[0] = program[k];
                            k+=1;
              FOO.bytes[1] = program[k];
                            k+=1;
              FOO.bytes[2] = program[k];
                            k+=1;
              FOO.bytes[3] = program[k];
               int N = FOO.x;
               k+=1;
              FOO.bytes[0] = program[k];
                k+=1;
              FOO.bytes[1] = program[k];
                k+=1;
              FOO.bytes[2] = program[k];
                k+=1;
              FOO.bytes[3] = program[k];
             int where  = FOO.x;
             printf("%d  JMPNPOPSTACK  %d  %d\n",orgk,N,where);
             k+=1;
             continue;
         }
         else if(inst==GOTONPSTACK)
         {
int orgk = k;
                           k+=1;
              FOO.bytes[0] = program[k];
                            k+=1;
              FOO.bytes[1] = program[k];
                            k+=1;
              FOO.bytes[2] = program[k];
                            k+=1;
              FOO.bytes[3] = program[k];
               int N = FOO.x;
               k+=1;
              FOO.bytes[0] = program[k];
                k+=1;
              FOO.bytes[1] = program[k];
                k+=1;
              FOO.bytes[2] = program[k];
                k+=1;
              FOO.bytes[3] = program[k];
             int where  = FOO.x;
             printf("%d  GOTONPSTACK  %d  %d\n",orgk,N,where);
             k+=1;
             continue;
         }
         else if(inst==GOTO)
         {
int orgk = k;
                           k+=1;
              FOO.bytes[0] = program[k];
                            k+=1;
              FOO.bytes[1] = program[k];
                            k+=1;
              FOO.bytes[2] = program[k];
                            k+=1;
              FOO.bytes[3] = program[k];
             int where = FOO.x;
             printf("%d  GOTO  %d\n",orgk,where);
             k+=1;
             continue;
         }
         else if(inst==BREAK)
         {
              int orgk = k;
                           k+=1;
              FOO.bytes[0] = program[k];
                            k+=1;
              FOO.bytes[1] = program[k];
                            k+=1;
              FOO.bytes[2] = program[k];
                            k+=1;
              FOO.bytes[3] = program[k];
              int where = FOO.x;
                            k+=1;
              FOO.bytes[0] = program[k];
                            k+=1;
              FOO.bytes[1] = program[k];
                            k+=1;
              FOO.bytes[2] = program[k];
              k+=1;
              FOO.bytes[3] = program[k];
             int n = FOO.x;
             printf("%d  BREAK  %d  %d\n",orgk,where,n);
             k+=1;
             continue;
         }
         else if(inst==LOAD_FUNC)
          {
                int orgk = k;
                k += 1;
                int32_t p;
                memcpy(&p, program+k, sizeof(int32_t));
                k += 4;
                int32_t idx;
                memcpy(&idx, program+k, sizeof(int32_t));
                k += 4;
                k++;
                int i2 = (int32_t)program[k];
                printf("%d LOAD_FUNC %d %d %d\n",orgk,p,idx,i2);
          }
        else if(inst==LOAD_CO)
          {
            int orgk = k;
            k += 1;
            int32_t p;
            memcpy(&p, program+k, sizeof(int32_t));
            k += 4;
            int32_t idx;
            memcpy(&idx, program+k, sizeof(int32_t));
            k += 4;
            int args = program[k];
            printf("%d LOAD_CO %d %d %d\n",orgk,p,idx,args);
          }
         else if(inst==JMPIF)
         {
             int orgk = k;
                           k+=1;
              FOO.bytes[0] = program[k];
                            k+=1;
              FOO.bytes[1] = program[k];
                            k+=1;
              FOO.bytes[2] = program[k];
                            k+=1;
              FOO.bytes[3] = program[k];
             int where = FOO.x;
             printf("%d  JMPIF  %d\n",orgk,where);
             k+=1;
             continue;
         }
         else if(inst==JMPIFFALSE)
         {
             int orgk = k;
                           k+=1;
              FOO.bytes[0] = program[k];
                            k+=1;
              FOO.bytes[1] = program[k];
                            k+=1;
              FOO.bytes[2] = program[k];
                            k+=1;
              FOO.bytes[3] = program[k];
             int where = FOO.x;
             printf("%d  JMPIFFALSE  %d\n",orgk,where);
             k+=1;
             continue;
         }
         else if(inst==JMPIFFALSENOPOP)
         {
             int orgk = k;
                           k+=1;
              FOO.bytes[0] = program[k];
                            k+=1;
              FOO.bytes[1] = program[k];
                            k+=1;
              FOO.bytes[2] = program[k];
                            k+=1;
              FOO.bytes[3] = program[k];
             int where = FOO.x;
             printf("%d  JMPIFFALSENOPOP  %d\n",orgk,where);
             k+=1;
             continue;
         }
         else if(inst==NOPOPJMPIF)
         {
             int orgk = k;
                           k+=1;
              FOO.bytes[0] = program[k];
                            k+=1;
              FOO.bytes[1] = program[k];
                            k+=1;
              FOO.bytes[2] = program[k];
                            k+=1;
              FOO.bytes[3] = program[k];
             int where = FOO.x;
             printf("%d  NOPOPJMPIF  %d\n",orgk,where);
             k+=1;
             continue;
         }
         else if(inst==GC)
         {
          printf("%d GC\n",k);
         }
         else
         {
             printf("%d  OPCODE: %d\n",k,inst);
             printf("Faulty Bytecode.You probably messed it up!\n");
             exit(0);
         }
         k+=1;
     }
}
