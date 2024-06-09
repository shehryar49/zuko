#ifndef SOCKET_PLT_H_
#define SOCKET_PLT_H_
#ifdef _WIN32
  #include "zapi.h"
  #define EXPORT __declspec(dllexport)
#else
  #define EXPORT
  #include "zapi.h"
#endif
extern "C"
{
	//Functions
	EXPORT zobject init();
	//Methods
	EXPORT zobject socket__construct(zobject*, int);
	EXPORT zobject socket_Setopt(zobject*,int);
	EXPORT zobject socket_Bind( zobject*,int);
	EXPORT zobject socket_Connect( zobject*,int);
	EXPORT zobject socket_Send( zobject*,int);
	EXPORT zobject socket_Recv( zobject*,int);
	EXPORT zobject socket_Listen( zobject*,int);
	EXPORT zobject socket_Accept( zobject*,int);
	EXPORT zobject socket_Close( zobject*,int);
	EXPORT zobject socket_SendTo( zobject*, int);
	EXPORT zobject socket_RecvFrom( zobject*, int);
	EXPORT zobject socket_del__( zobject*, int);
}
#endif