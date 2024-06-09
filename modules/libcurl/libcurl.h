#ifdef _WIN32
  #define EXPORT __declspec(dllexport)
#else
  #define EXPORT
#endif

#include "zapi.h"

extern "C"
{

    EXPORT zobject init();
    //
    //Functions
    EXPORT zobject STRERROR(zobject*,int);
    //Methods of our Wrapper Curl Object(not real curl object)
    EXPORT zobject curlklass__construct__(zobject*,int);//
    EXPORT zobject setopt(zobject*,int);
    EXPORT zobject perform(zobject*,int);
    EXPORT zobject cleanup(zobject*,int);
    EXPORT zobject getinfo(zobject*,int);
    EXPORT zobject ESCAPE(zobject*,int);
    EXPORT zobject UNESCAPE(zobject*,int);
    EXPORT zobject curlklass__del__(zobject*,int);
    //Methods of MimeObject
    EXPORT zobject mime__construct__(zobject*,int);
    EXPORT zobject addpart(zobject*,int);
    EXPORT zobject MIME__del__(zobject*,int);
    //Methods MimePartObject
    EXPORT zobject MIME_NAME(zobject*,int);
    EXPORT zobject MIME_FILENAME(zobject*,int);
    EXPORT zobject MIME_CONTENTTYPE(zobject*,int);
    EXPORT zobject MIME_DATA(zobject*,int);
    EXPORT zobject MIMEPART__del__(zobject*,int);
    
}