#ifndef CGI_H_
#define CGI_H_
#include "zapi.h"
#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif
using namespace std;

extern "C"
{
    EXPORT zobject init();
    //Functions
    EXPORT zobject FormData(zobject*,int);//returns a dictionary containing all the data sent by client
    //handles GET,POST and multipart/form POST requests
    EXPORT zobject cookies(zobject*,int);
}
#endif