/*
Datetime Module Zuko
Written by Shahryar Ahmad 
5 March 2023
The code is completely free to use and comes without any guarantee/warrantee
*/
#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT 
#endif
#include "zapi.h"
extern "C"
{
  EXPORT zobject init();
  EXPORT zobject TIME(zobject* args,int32_t n);
  EXPORT zobject CTIME(zobject* args,int32_t n);
  EXPORT zobject LOCALTIME(zobject* args,int32_t n);
  EXPORT zobject GMTIME(zobject* args,int32_t n);
  EXPORT zobject TMKLASS__del__(zobject* args,int32_t n);
  
}