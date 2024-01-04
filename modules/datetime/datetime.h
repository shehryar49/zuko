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
  EXPORT ZObject init();
  EXPORT ZObject TIME(ZObject* args,int32_t n);
  EXPORT ZObject CTIME(ZObject* args,int32_t n);
  EXPORT ZObject LOCALTIME(ZObject* args,int32_t n);
  EXPORT ZObject GMTIME(ZObject* args,int32_t n);
  EXPORT ZObject TMKLASS__del__(ZObject* args,int32_t n);
  
}