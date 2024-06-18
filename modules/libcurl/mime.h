#ifndef ZUKO_CURL_MIME_H_
#define ZUKO_CURL_MIME_H_

#include "zapi.h"

#ifdef _WIN32
  #define EXPORT __declspec(dllexport)
#else
  #define EXPORT
#endif
extern zclass* mime_class;
extern "C"
{
    EXPORT zobject mime_construct(zobject*,int);
    EXPORT zobject mime_addpart(zobject*,int);
    EXPORT zobject mime__del__(zobject*,int);
}
#endif