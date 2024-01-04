#include "zapi.h"
#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif
extern "C"
{
    EXPORT ZObject init();

    //
    //Functions
    EXPORT ZObject match(ZObject*,int);
    EXPORT ZObject search(ZObject*,int);
    EXPORT ZObject replace(ZObject*,int);
}
