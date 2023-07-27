/*
Datetime Module Plutonium
Written by Shahryar Ahmad 
5 March 2023
The code is completely free to use and comes without any guarantee/warrantee
*/
#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT 
#endif
#include "PltObject.h"
extern "C"
{
  EXPORT PltObject init();
  EXPORT PltObject TIME(PltObject* args,int32_t n);
  EXPORT PltObject CTIME(PltObject* args,int32_t n);
  EXPORT PltObject LOCALTIME(PltObject* args,int32_t n);
  EXPORT PltObject GMTIME(PltObject* args,int32_t n);
  EXPORT PltObject TMKLASS__del__(PltObject* args,int32_t n);
  
}