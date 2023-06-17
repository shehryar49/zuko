#ifndef CGI_H_
#define CGI_H_
#include "PltObject.h"
#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif
using namespace std;

extern "C"
{
    PltObject init();
    //Functions
    PltObject FormData(PltObject*,int);//returns a dictionary containing all the data sent by client
    //handles GET,POST and multipart/form POST requests
    PltObject cookies(PltObject*,int);
}
#endif