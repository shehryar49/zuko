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
    EXPORT ZObject REAL_CONNECT(ZObject*,int32_t);//returns a connection object

    //Connection object methods
    EXPORT ZObject CONN__STORE_RESULT(ZObject*,int32_t);//takes a conn.object and returns result object
    EXPORT ZObject CONN__QUERY(ZObject*,int32_t);//takes a conn.obj,query and executes the query
    EXPORT ZObject CONN__CLOSE(ZObject*,int32_t);//close connection
    EXPORT ZObject CONN__DEL__(ZObject*,int32_t);
    
    //Result object methods
    EXPORT ZObject RES__FETCH_ROW(ZObject*,int32_t);//fetch a row from result object and return it
    EXPORT ZObject RES__NUM_ROWS(ZObject*,int32_t);
    EXPORT ZObject RES__NUM_FIELDS(ZObject*,int32_t);
    EXPORT ZObject RES__DEL__(ZObject*,int32_t);
    
    EXPORT void unload();//called when module is unloaded
    //
}
