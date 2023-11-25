#include "process.h"
#include <unistd.h>
using namespace std;

ZObject init(ZObject* rrr)
{
  Module* prcMod = vm_allocModule();
  prcMod->members.emplace("fork",ZObjFromFunction("process.fork",&FORK));
  prcMod->members.emplace("getpid",ZObjFromFunction("process.getpid",&GETPID));
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


