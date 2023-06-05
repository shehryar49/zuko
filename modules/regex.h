#include "PltObject.h"

extern "C"
{
    PltObject init();

    //
    //Functions
    PltObject match(PltObject*,int);
    PltObject search(PltObject*,int);
    PltObject replace(PltObject*,int);
}
