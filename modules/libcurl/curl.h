#ifndef ZUKO_CURL_H_
#define ZUKO_CURL_H_

#include "zapi.h"

#ifdef _WIN32
  #define EXPORT __declspec(dllexport)
#else
  #define EXPORT
#endif

extern zclass* curl_class;

extern "C"
{
    // These are wrapper functions!
    EXPORT zobject zcurl_construct(zobject*,int32_t);
    EXPORT zobject zcurl_setopt(zobject*,int);
    EXPORT zobject zcurl_perform(zobject*,int);
    EXPORT zobject zcurl_getinfo(zobject*,int);
    EXPORT zobject zcurl_escape(zobject*,int);
    EXPORT zobject zcurl_unescape(zobject*,int);
    EXPORT zobject zcurl__del__(zobject*,int); // curl object deletor, this also eliminates the need for curl_easy_cleanup()
}

#endif