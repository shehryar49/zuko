#include "process.h"
#include "zobject.h"
#include <unistd.h>
using namespace std;

zobject init(zobject* rrr)
{
  zmodule* prcMod = vm_allocModule();
  Module_addNativeFun(prcMod,"fork",&FORK);
  Module_addNativeFun(prcMod,"getpid",&GETPID);
  return zobj_from_module(prcMod);
}

zobject FORK(zobject* args,int32_t n)
{
  if(n != 0)
    return Z_Err(ArgumentError,"0 arguments needed!");
  int pid = fork();
  if(pid < 0)
    return Z_Err(Error,"fork() syscall failed.");
  return zobj_from_int64(pid);
}
zobject GETPID(zobject* args,int32_t n)
{
  if(n != 0)
    return Z_Err(ArgumentError,"0 arguments needed!");
  int pid = getpid();
  if(pid < 0)
    return Z_Err(Error,"getpid() syscall failed.");
  return zobj_from_int64(pid);
}


