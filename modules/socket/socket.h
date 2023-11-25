#ifndef SOCKET_PLT_H_
#define SOCKET_PLT_H_
#ifdef _WIN32
  #include "ZObject.h"
  #define EXPORT __declspec(dllexport)
#else
  #define EXPORT
  #include "ZObject.h"
#endif
extern "C"
{
	//Functions
	EXPORT ZObject init();
	//Methods
	EXPORT ZObject socket__construct(ZObject*, int);
	EXPORT ZObject socket_Setopt(ZObject*,int);
	EXPORT ZObject socket_Bind( ZObject*,int);
	EXPORT ZObject socket_Connect( ZObject*,int);
	EXPORT ZObject socket_Send( ZObject*,int);
	EXPORT ZObject socket_Recv( ZObject*,int);
	EXPORT ZObject socket_Listen( ZObject*,int);
	EXPORT ZObject socket_Accept( ZObject*,int);
	EXPORT ZObject socket_Close( ZObject*,int);
	EXPORT ZObject socket_SendTo( ZObject*, int);
	EXPORT ZObject socket_RecvFrom( ZObject*, int);
	EXPORT ZObject socket_del__( ZObject*, int);
}
#endif