#include "process.h"
#include <unistd.h>
using namespace std;

ZObject init(ZObject* rrr)
{
  Module* prcMod = vm_allocModule();
  Module_addNativeFun(prcMod,"fork",&FORK);
  Module_addNativeFun(prcMod,"getpid",&GETPID);
  return ZObjFromModule(prcMod);
}

ZObject FORK(ZObject* args,int32_t n)
{
  if(n != 0)
    return Z_Err(ArgumentError,"0 arguments needed!");
  int pid = fork();
  if(pid < 0)
    return Z_Err(Error,"fork() syscall failed.");
  return ZObjFromInt64(pid);
}
ZObject GETPID(ZObject* args,int32_t n)
{
  if(n != 0)
    return Z_Err(ArgumentError,"0 arguments needed!");
  int pid = getpid();
  if(pid < 0)
    return Z_Err(Error,"getpid() syscall failed.");
  return ZObjFromInt64(pid);
}


