#include "PltObject.h"

extern "C"
{
    PltObject init();
    //
    PltObject INIT(PltObject*,int32_t);//returns a connection object
    PltObject REAL_CONNECT(PltObject*,int32_t);//takes a conn. object and establishes connection
    PltObject STORE_RESULT(PltObject*,int32_t);//takes a conn.object and returns result object
    PltObject QUERY(PltObject*,int32_t);//takes a conn.obj,query and executes the query
    PltObject FETCH_ROW(PltObject*,int32_t);//fetch a row from result object and return it
    PltObject FETCH_ROW_AS_STR(PltObject*,int32_t);
    PltObject NUM_ROWS(PltObject*,int32_t);
    PltObject NUM_FIELDS(PltObject*,int32_t);
    
    PltObject CLOSE(PltObject*,int32_t);//close connection
    PltObject CONN__DEL__(PltObject*,int32_t);
    PltObject RES__DEL__(PltObject*,int32_t);
    void unload();//called when module is unloaded
    //
}