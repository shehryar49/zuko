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
    EXPORT PltObject INIT(PltObject*,int32_t);//returns a connection object
    EXPORT PltObject REAL_CONNECT(PltObject*,int32_t);//takes a conn. object and establishes connection
    EXPORT PltObject STORE_RESULT(PltObject*,int32_t);//takes a conn.object and returns result object
    EXPORT PltObject QUERY(PltObject*,int32_t);//takes a conn.obj,query and executes the query
    EXPORT PltObject FETCH_ROW(PltObject*,int32_t);//fetch a row from result object and return it
    EXPORT PltObject FETCH_ROW_AS_STR(PltObject*,int32_t);
    EXPORT PltObject NUM_ROWS(PltObject*,int32_t);
    EXPORT PltObject NUM_FIELDS(PltObject*,int32_t);
    
    EXPORT PltObject CLOSE(PltObject*,int32_t);//close connection
    EXPORT PltObject CONN__DEL__(PltObject*,int32_t);
    EXPORT PltObject RES__DEL__(PltObject*,int32_t);
    EXPORT void unload();//called when module is unloaded
    //
}