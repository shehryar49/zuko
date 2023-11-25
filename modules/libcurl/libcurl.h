#ifdef _WIN32
  #define EXPORT __declspec(dllexport)
#else
  #define EXPORT
#endif

#include "ZObject.h"

extern "C"
{

    EXPORT ZObject init();
    //
    //Functions
    EXPORT ZObject STRERROR(ZObject*,int);
    //Methods of our Wrapper Curl Object(not real curl object)
    EXPORT ZObject curlklass__construct__(ZObject*,int);//
    EXPORT ZObject setopt(ZObject*,int);
    EXPORT ZObject perform(ZObject*,int);
    EXPORT ZObject cleanup(ZObject*,int);
    EXPORT ZObject getinfo(ZObject*,int);
    EXPORT ZObject ESCAPE(ZObject*,int);
    EXPORT ZObject UNESCAPE(ZObject*,int);
    EXPORT ZObject curlklass__del__(ZObject*,int);
    //Methods of MimeObject
    EXPORT ZObject mime__construct__(ZObject*,int);
    EXPORT ZObject addpart(ZObject*,int);
    EXPORT ZObject MIME__del__(ZObject*,int);
    //Methods MimePartObject
    EXPORT ZObject MIME_NAME(ZObject*,int);
    EXPORT ZObject MIME_FILENAME(ZObject*,int);
    EXPORT ZObject MIME_CONTENTTYPE(ZObject*,int);
    EXPORT ZObject MIME_DATA(ZObject*,int);
    EXPORT ZObject MIMEPART__del__(ZObject*,int);
    
}