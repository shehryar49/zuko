#include "PltObject.h"

extern "C"
{

    void init(PltObject*);
    //
    //Functions
    void STRERROR(PltObject*,int,PltObject*);
    //Methods of our Wrapper Curl Object(not real curl object)
    void curlklass__construct__(PltObject*,int,PltObject*);//
    void setopt(PltObject*,int,PltObject*);
    void perform(PltObject*,int,PltObject*);
    void cleanup(PltObject*,int,PltObject*);
    void getinfo(PltObject*,int,PltObject*);
    void data(PltObject*,int,PltObject*);
    void curlklass__destroy(PltObject*,int,PltObject*);
    //Methods of MimeObject
    void mime__construct__(PltObject*,int,PltObject*);
    void addpart(PltObject*,int,PltObject*);
    void MIME_FREE(PltObject*,int,PltObject*);
    //Methods MimePartObject
    void MIME_NAME(PltObject*,int,PltObject*);
    void MIME_DATA(PltObject*,int,PltObject*);
    void destroyMIMEPART(PltObject*,int,PltObject*);
}