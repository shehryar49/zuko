#include "process.h"
#include <unistd.h>
using namespace std;

PltObject init(PltObject* rrr)
{
  Module* prcMod = vm_allocModule();
  prcMod->members.emplace("fork",PObjFromFunction("process.fork",&FORK));
  prcMod->members.emplace("getpid",PObjFromFunction("process.getpid",&GETPID));
  return PObjFromModule(prcMod);
}

PltObject FORK(PltObject* args,int32_t n)
{
  if(n != 0)
    return Plt_Err(ARGUMENT_ERROR,"0 arguments needed!");
  int pid = fork();
  if(pid < 0)
    return Plt_Err(UNKNOWN_ERROR,"fork() syscall failed.");
  return PObjFromInt64(pid);
}
PltObject GETPID(PltObject* args,int32_t n)
{
  if(n != 0)
    return Plt_Err(ARGUMENT_ERROR,"0 arguments needed!");
  int pid = getpid();
  if(pid < 0)
    return Plt_Err(UNKNOWN_ERROR,"getpid() syscall failed.");
  return PObjFromInt64(pid);
}


