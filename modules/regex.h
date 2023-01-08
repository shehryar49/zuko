#include "PltObject.h"

extern "C"
{
    void init(PltObject* rr);

    //
    //Functions
    void match(PltObject*,int,PltObject*);
    void search(PltObject*,int,PltObject*);
    void replace(PltObject*,int,PltObject*);
}
