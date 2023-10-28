#ifdef _WIN32
  #define EXPORT __declspec(dllexport)
#else
  #define EXPORT
#endif

#include "PltObject.h"

extern "C"
{

    EXPORT PltObject init();
    //
    //Functions
    EXPORT PltObject STRERROR(PltObject*,int);
    //Methods of our Wrapper Curl Object(not real curl object)
    EXPORT PltObject curlklass__construct__(PltObject*,int);//
    EXPORT PltObject setopt(PltObject*,int);
    EXPORT PltObject perform(PltObject*,int);
    EXPORT PltObject cleanup(PltObject*,int);
    EXPORT PltObject getinfo(PltObject*,int);
    EXPORT PltObject ESCAPE(PltObject*,int);
    EXPORT PltObject UNESCAPE(PltObject*,int);
    EXPORT PltObject curlklass__del__(PltObject*,int);
    //Methods of MimeObject
    EXPORT PltObject mime__construct__(PltObject*,int);
    EXPORT PltObject addpart(PltObject*,int);
    EXPORT PltObject MIME__del__(PltObject*,int);
    //Methods MimePartObject
    EXPORT PltObject MIME_NAME(PltObject*,int);
    EXPORT PltObject MIME_FILENAME(PltObject*,int);
    EXPORT PltObject MIME_CONTENTTYPE(PltObject*,int);
    EXPORT PltObject MIME_DATA(PltObject*,int);
    EXPORT PltObject MIMEPART__del__(PltObject*,int);
    
}