#ifndef SOCKET_ZUKO_H_
#define SOCKET_ZUKO_H_
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
	EXPORT zobject socket_construct(zobject*, int);
	EXPORT zobject socket_setopt(zobject*,int);
	EXPORT zobject socket_bind( zobject*,int);
	EXPORT zobject socket_connect( zobject*,int);
	EXPORT zobject socket_send( zobject*,int);
	EXPORT zobject socket_recv( zobject*,int);
	EXPORT zobject socket_listen( zobject*,int);
	EXPORT zobject socket_accept( zobject*,int);
	EXPORT zobject socket_close( zobject*,int);
	EXPORT zobject socket_sendto( zobject*, int);
	EXPORT zobject socket_recvfrom( zobject*, int);
	EXPORT zobject socket_del( zobject*, int);
}
#endif
