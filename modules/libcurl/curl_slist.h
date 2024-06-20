#ifndef ZUKO_CURL_SLIST_H_
#define ZUKO_CURL_SLIST_H_

#include "zapi.h"
#include "zobject.h"

#ifdef _WIN32
  #define EXPORT __declspec(dllexport)
#else
  #define EXPORT
#endif

extern zclass* slist_class;

extern "C"
{
    EXPORT zobject slist_construct(zobject*,int32_t);
    EXPORT zobject slist_append(zobject*,int32_t);
    EXPORT zobject slist__del__(zobject*,int32_t);
}
#endif