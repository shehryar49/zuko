#include "PltObject.h"
#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif
extern "C"
{
    PltObject init();

    //
    //Functions
    PltObject match(PltObject*,int);
    PltObject search(PltObject*,int);
    PltObject replace(PltObject*,int);
}
