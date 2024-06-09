#include "zapi.h"
#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif
extern "C"
{
    EXPORT zobject init();

    //
    //Functions
    EXPORT zobject match(zobject*,int);
    EXPORT zobject search(zobject*,int);
    EXPORT zobject replace(zobject*,int);
}
