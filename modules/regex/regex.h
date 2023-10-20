#include "PltObject.h"
#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif
extern "C"
{
    EXPORT PltObject init();

    //
    //Functions
    EXPORT PltObject match(PltObject*,int);
    EXPORT PltObject search(PltObject*,int);
    EXPORT PltObject replace(PltObject*,int);
}
