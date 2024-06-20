#ifdef _WIN32
  #define EXPORT __declspec(dllexport)
#else
  #define EXPORT
#endif

#include "zapi.h"

extern "C"
{

    EXPORT zobject init(); //initialize the module
    EXPORT zobject wrapped_strerror(zobject*,int);
    EXPORT void unload(); // module cleanup   
}