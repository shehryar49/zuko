#ifndef ZUKO_CURL_MIMEPART_H_
#define ZUKO_CURL_MIMEPART_H_
#include "zapi.h"

#ifdef _WIN32
  #define EXPORT __declspec(dllexport)
#else
  #define EXPORT
#endif

extern zclass* mimepart_class;

extern "C"
{
    EXPORT zobject mimepart_name(zobject*,int);
    EXPORT zobject mimepart_filename(zobject*,int);
    EXPORT zobject mimepart_filedata(zobject*,int);
    EXPORT zobject mimepart_content_type(zobject*,int);
    EXPORT zobject mimepart_data(zobject*,int);
    EXPORT zobject mimepart__del__(zobject*,int);
}
#endif
