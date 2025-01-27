// This code needs some major changes...
// It hasn't been updated in a while
#include "dis.h"
#include "opcode.h"
#include "misc.h"
#include "vm.h"
#include <string.h>



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

void dis(uint8_t* bytecode)
{

  size_t k = 26;
  uint8_t* program = &(bytecode[0]);


  unsigned char inst = 0;
  printf("0-25  <init Error Classes>\n");
  while(inst != OP_EXIT)
  {
      inst = program[k];

      if(inst==LOAD)
      {
          printf("%zu  LOAD ",k);
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
          continue;
      }
      else if(inst==OP_EXIT)
      {
        printf("%zu  OP_EXIT\n",k);

      }
      else if(inst == LOAD_INT64)
      {
            k++;
            FOO1.bytes[0] = program[k];
            FOO1.bytes[1] = program[k+1];
            FOO1.bytes[2] = program[k+2];
            FOO1.bytes[3] = program[k+3];
            FOO1.bytes[4] = program[k+4];
            FOO1.bytes[5] = program[k+5];
            FOO1.bytes[6] = program[k+6];
            FOO1.bytes[7] = program[k+7];
            printf("LOAD_INT64 %lld\n",FOO1.l);
            k+=8;
            continue;
 
      }
      else if(inst==CONDITIONAL_RETURN_I32)
      {
        printf("%zu  CONDITIONAL_RETURN_I32\n",k);
        k+=4;
      }     
      else if(inst==CONDITIONAL_RETURN_LOCAL)
      {
        printf("%zu  CONDITIONAL_RETURN_LOCAL\n",k);
        k+=4;
      } 
      else if(inst==LOAD_STR)
      {
        printf("%zu  LOAD_STR ",k);
        k+=1;
        int32_t i;
        memcpy(&i, program+k, 4);
        k+=3;
        printf("%d\n",i);
      }
      else if(inst==RETURN_INT32)
      {
        printf("%zu  RETURN_INT32 ",k);
        k+=1;
        int32_t i;
        memcpy(&i, program+k, 4);
        k+=3;
        printf("%d\n",i);
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
        printf("%zu  NPOP_STACK ",k);
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
      else if(inst == LOOP)
      {
        printf("%zu LOOP ",k);
        bool is_global = program[++k];
        k++;
        int32_t var;
        memcpy(&var,program+k,4);
        k+=4;
        int32_t end;
        if(is_global)
        {
          memcpy(&end,program+k,4);
          k+=4;
        }
        int32_t where;
        memcpy(&where,program+k,4);
        k+=3;
        if(is_global)
          printf("%d %d %d %d\n",is_global,var,end,where);
        else
          printf("%d %d %d\n",is_global,var,where);
      }
      else if(inst == DLOOP)
      {
        printf("%zu DLOOP ",k);
        bool is_global = program[++k];
        k++;
        int32_t var;
        memcpy(&var,program+k,4);
        k+=4;
        int32_t end;
        if(is_global)
        {
          memcpy(&end,program+k,4);
          k+=4;
        }
        int32_t where;
        memcpy(&where,program+k,4);
        k+=3;
        if(is_global)
          printf("%d %d %d %d\n",is_global,var,end,where);
        else
          printf("%d %d %d\n",is_global,var,where);
      }
      else if(inst == SETUP_LOOP)
      {
        printf("%zu SETUP_LOOP ",k);
        bool is_global = program[++k];
        k++;
        int32_t var;
        memcpy(&var,program+k,4);
        k+=4;
        int32_t end;
        if(is_global)
        {
          memcpy(&end,program+k,4);
          k+=4;
        }
        int32_t where;
        memcpy(&where,program+k,4);
        k+=3;
        if(is_global)
          printf("%d %d %d %d\n",is_global,var,end,where);
        else
          printf("%d %d %d\n",is_global,var,where);
      }
      else if(inst == SETUP_DLOOP)
      {
        printf("%zu SETUP_DLOOP ",k);
        bool is_global = program[++k];
        k++;
        int32_t var;
        memcpy(&var,program+k,4);
        k+=4;
        int32_t end;
        if(is_global)
        {
          memcpy(&end,program+k,4);
          k+=4;
        }
        int32_t where;
        memcpy(&where,program+k,4);
        k+=3;
        if(is_global)
          printf("%d %d %d %d\n",is_global,var,end,where);
        else
          printf("%d %d %d\n",is_global,var,where);
      }      
      else if(inst==INPLACE_INC)
      {
        printf("%zu  INPLACE_INC ",k);
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
        printf("%zu  LOAD_BYTE %d\n",k,program[k+1]);
        k++;
      }
      else if(inst==POP_STACK)
      {
        printf("%zu  POP_STACK\n",k);

      }
      else if(inst==POP_EXCEP_TARG)
      {
        printf("%zu  POP_EXCEPT_TARG\n",k);

      }
      else if(inst==CALLMETHOD)
      {
        k+=1;
        printf("%zu  CALLMETHOD  ",k-1);
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
        printf("%zu  LOAD_GLOBAL ",k);
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
        printf("%zu  ONERR_GOTO ",k);
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
        printf("%zu  IMPORT ",k);
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
        printf("%zu  ASSIGN ",k);
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
        printf("%zu  ASSIGN_GLOBAL ",k);
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
        printf("%zu  ASSIGNINDEX\n",k);
      }
      else if(inst==OP_RETURN)
      {
          printf("%zu  RETURN\n",k);
      }
      else if(inst==YIELD)
      {
          printf("%zu  YIELD\n",k);
      }
      else if(inst==YIELD_AND_EXPECTVAL)
      {
          printf("%zu  YIELD_AND_EXPECTVAL\n",k);
      }
      else if(inst==MEMB)
      {
          printf("%zu  MEMB  ",k);
          k+=1;
          FOO.bytes[0] = program[k++];
          FOO.bytes[1] = program[k++];
          FOO.bytes[2] = program[k++];
          FOO.bytes[3] = program[k];
          printf("%d\n",FOO.x);
      }
      else if(inst==SELFMEMB)
      {
          printf("%zu  SELFMEMB  ",k);
          k+=1;
          FOO.bytes[0] = program[k++];
          FOO.bytes[1] = program[k++];
          FOO.bytes[2] = program[k++];
          FOO.bytes[3] = program[k];
          printf("%d\n",FOO.x);
      }
      else if(inst==ASSIGNSELFMEMB)
      {
          printf("%zu  ASSIGNSELFMEMB  ",k);
          k+=1;
          FOO.bytes[0] = program[k++];
          FOO.bytes[1] = program[k++];
          FOO.bytes[2] = program[k++];
          FOO.bytes[3] = program[k];
          printf("%d\n",FOO.x);
      }
      else if(inst==LOAD_LOCAL)
      {
          printf("%zu  LOAD_LOCAL  ",k);
          k+=1;
          FOO.bytes[0] = program[k++];
          FOO.bytes[1] = program[k++];
          FOO.bytes[2] = program[k++];
          FOO.bytes[3] = program[k];
          printf("%d\n",FOO.x);
      }
      else if(inst==NEG)
      {
          printf("%zu  NEG\n",k);
      }
      else if(inst==XOR)
      {
          printf("%zu  ^\n",k);
      }
      else if(inst==NOT)
      {
          printf("%zu  NOT\n",k);
      }
      else if(inst==SMOREQ)
      {
          printf("%zu  <=\n",k);
      }
      else if(inst==GROREQ)
      {
          printf("%zu  >=\n",k);
      }
      else if(inst==SMALLERTHAN)
      {
          printf("%zu  <\n",k);
      }
      else if(inst==GREATERTHAN)
      {
          printf("%zu  >\n",k);
      }
      else if(inst==BITWISE_AND)
      {
          printf("%zu  &\n",k);
      }
      else if(inst==IS)
      {
          printf("%zu  IS\n",k);
      }
      else if(inst==BITWISE_OR)
      {
          printf("%zu  |\n",k);
      }
      else if(inst==RSHIFT)
      {
          printf("%zu  >>\n",k);
      }
      else if(inst==LSHIFT)
      {
          printf("%zu  <<\n",k);
      }
      else if(inst==COMPLEMENT)
      {
          printf("%zu  >\n",k);
      }
      else if(inst==INDEX)
      {
          printf("%zu  INDEX\n",k);
      }
      else if(inst==CALL)
      {
          printf("%zu  CALL ",k);
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
        printf("%zu  CALLFORVAL ",k);
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
        printf("%zu  CALLUDF  %d\n",k-1,N);
      }
      else if(inst==THROW)
      {
          printf("%zu  THROW\n",k);
      }  
      else if(inst==INC_GLOBAL)
      {
        printf("%zu  INC_GLOBAL ",k);
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
          printf("%zu  ADD\n",k);
      }
      else if(inst==EQ)
      {
          printf("%zu  EQ\n",k);
      }
      else if(inst==MUL)
      {
        printf("%zu  MUL\n",k);
      }
      else if(inst==MOD)
      {
        printf("%zu  MOD\n",k);
      }
      else if(inst==SUB)
      {
        printf("%zu  SUB\n",k);
      }
      else if(inst==DIV)
      {
        printf("%zu  DIV\n",k);
      }
      else if(inst==SMALLERTHAN)
      {
          printf("%zu  SMALLER_THAN\n",k);
      }
      else if(inst==NOTEQ)
      {
        printf("%zu  NOTEQ\n",k);
      }
      else if(inst==LOAD_NIL)
      {
        printf("%zu  LOAD_NIL\n",k);
      }
      else if(inst==LOAD_TRUE)
      {
        printf("%zu  LOAD_TRUE\n",k);
      }
      else if(inst==LOAD_FALSE)
      {
        printf("%zu  LOAD_FALSE\n",k);
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
      else if(inst==CALL_DIRECT)
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
          printf("%d  CALL_DIRECT  %d\n",orgk,where);
          k+=1;
          continue;
      }
      else if(inst == INDEX_FAST)
      {
        printf("%zu INDEX_FAST",k);
        int is_global1 = program[++k];
        int is_global2 = program[++k];
        k+=1;
        int idx1,idx2;
        memcpy(&idx1,program+k,4);
        k+=4;
        memcpy(&idx2,program+k,4);
        k+=3;
        printf(" %d %d %d %d\n",is_global1,is_global2,idx1,idx2);
      }
      else if(inst == LOADVAR_ADDINT32)
      {
        printf("%zu LOADVAR_ADDINT32",k);
        int is_global = program[++k];
        k+=1;
        int idx1,idx2;
        memcpy(&idx1,program+k,4);
        k+=4;
        memcpy(&idx2,program+k,4);
        k+=3;
        printf(" %d %d %d\n",is_global,idx1,idx2);
      }
      else if(inst == LOADVAR_SUBINT32)
      {
        printf("%zu LOADVAR_SUBINT32",k);
        int is_global = program[++k];
        k+=1;
        int idx1,idx2;
        memcpy(&idx1,program+k,4);
        k+=4;
        memcpy(&idx2,program+k,4);
        k+=3;
        printf(" %d %d %d\n",is_global,idx1,idx2);
      }
      else if(inst==CMP_JMPIFFALSE)
      {
          printf("%zu CMP_JMPIFFALSE",k);
          uint8_t op = program[++k];
          k+=1;
          int32_t where;
          memcpy(&where,program+k,4);
          k+=3;
          printf(" %d %d\n",op,where);
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
      else if(inst==GOTOIFFALSE)
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
          printf("%d  GOTOIFFALSE  %d\n",orgk,where);
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
      printf("%zu GC\n",k);
      }
      else
      {
          printf("%zu  OPCODE: %d\n",k,inst);
          printf("Faulty Bytecode.You probably messed it up!\n");
          exit(0);
      }
      k+=1;
  }
}
