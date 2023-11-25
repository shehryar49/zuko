#ifndef CGI_H_
#define CGI_H_
#include "ZObject.h"
#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif
using namespace std;

extern "C"
{
    EXPORT ZObject init();
    //Functions
    EXPORT ZObject FormData(ZObject*,int);//returns a dictionary containing all the data sent by client
    //handles GET,POST and multipart/form POST requests
    EXPORT ZObject cookies(ZObject*,int);
}
#endif