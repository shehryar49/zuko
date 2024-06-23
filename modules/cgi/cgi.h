#ifndef CGI_H_
#define CGI_H_

#include "zapi.h"
#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif



extern "C"
{
    EXPORT zobject init();
    //Functions
    EXPORT zobject formdata(zobject*,int);//returns a dictionary representing submitted form, handles GET, POST and POST multipart
    EXPORT zobject cookies(zobject*,int); // returns a dictionary representing cookies sent by the client

    void unload();
}
#endif