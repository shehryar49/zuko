#include "socket.h"
#include "zapi.h"
#include "zobject.h"
#include <string.h>
#include <stdlib.h>
#include <string>
#ifdef _WIN32
  #include <winsock2.h>
  #pragma comment(lib,"ws2_32")
  #define SOCKTYPE SOCKET
#else
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  #define EXPORT
  #define SOCKTYPE int
#endif
using namespace std;



int validateArgTypes(string e,zobject* args,int n)
{
    if (e.length() != n)
        return -1;//missing arguments
    int f = 0;
    for (int k = 0;k<n;k+=1)
    {
        if (args[k].type != e[f])
            return f;//type mismatch for argument number f
        f += 1;
    }
    return -2;//no error
}

zclass* socketKlass;
zclass* udpResKlass;
zobject nil;

inline zobject quickErr(zclass* k,string msg) // uses std::string instead of cstring
{
  return z_err(k,msg.c_str());
}
EXPORT zobject init()
{
    nil.type = Z_NIL;
    #ifdef _WIN32
      WSADATA wsa;
      if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
      {
          string errMsg = "Error code: " + to_string(WSAGetLastError());
          return quickErr(Error, errMsg);
      }
    #endif
    zmodule* d = vm_alloc_zmodule();
    socketKlass = vm_alloc_zclass();
    socketKlass->name = "socket";
    
    zclass_add_method(socketKlass,"__construct__",&socket_construct);
    zclass_add_method(socketKlass,"bind",&socket_bind);
    zclass_add_method(socketKlass,"connect",  &socket_connect);
    zclass_add_method(socketKlass,"send",  &socket_send);
    zclass_add_method(socketKlass,"recv",  & socket_recv);
    zclass_add_method(socketKlass,"listen",  & socket_listen);
    zclass_add_method(socketKlass,"accept",  &socket_accept);
    zclass_add_method(socketKlass,"sendto",  &socket_sendto);
    zclass_add_method(socketKlass,"recvfrom", &socket_recvfrom);
    zclass_add_method(socketKlass,"close",  & socket_close);
    zclass_add_method(socketKlass,"__del__", &socket_del);
    zclass_addmember(socketKlass,".internalPTR",nil );
    
    udpResKlass = vm_alloc_zclass();
    zclass_addmember(udpResKlass,("addr"), nil);
    zclass_addmember(udpResKlass,("data"), nil);

    zmodule_add_class(d,"socket", socketKlass);
    zmodule_add_class(d,"udpres", udpResKlass);
    zmodule_add_member(d,"AF_INET", zobj_from_int(AF_INET));
    zmodule_add_member(d,"AF_INET6", zobj_from_int(AF_INET6));
    zmodule_add_member(d,"SOCK_STREAM", zobj_from_int(SOCK_STREAM));
    zmodule_add_member(d,"SOCK_DGRAM", zobj_from_int(SOCK_DGRAM));
    zmodule_add_member(d,"SOCK_RAW", zobj_from_int(SOCK_RAW));
    

    return zobj_from_module(d);

}
EXPORT zobject socket_del( zobject* args, int n)
{
    return nil;
}

zobject socket_construct(zobject* args, int n)
{
  int e = validateArgTypes("oii", args, n);
  if (e == -1)
    return quickErr(ValueError, "3 arguments needed!");
  if (e!=-2)
    return quickErr(TypeError, "Argument " + to_string(e+1) + " is of invalid type.");
  if (args[0].type != Z_OBJ || ((zclass_object*)args[0].ptr)->_klass != socketKlass)
    return quickErr(TypeError, "Argument 1 must be an object of socket class!");  
  
  zclass_object* ko = (zclass_object*)args[0].ptr;
  SOCKTYPE sock = socket(args[1].i, args[2].i, 0);  
  #ifdef _WIN32
    if (sock == INVALID_SOCKET)
    {
      string errMsg = "Error code " + to_string(WSAGetLastError());
      return quickErr(Error, errMsg.c_str());      
    }
  #else
    if(sock < 0)
      return quickErr(Error,strerror(errno));
  #endif
  zclassobj_set(ko,".sock",zobj_from_int(sock));
  zclassobj_set(ko,".ipfamily",args[1]);
  return nil;
}
zobject socket_bind(zobject* args, int n)
{
    int e = validateArgTypes("osi", args, n);
    if (e == -1)
      return quickErr(ValueError, "3 arguments needed!");
    if (e!=-2)
      return quickErr(TypeError, "Argument " + to_string(e+1) + " is of invalid type.");
        
    if (args[0].type != Z_OBJ || ((zclass_object*)args[0].ptr)->_klass != socketKlass)
      return quickErr(TypeError, "Argument 1 must be an object of socket class!");  
  
    sockaddr_in server;
    
    zclass_object* p = (zclass_object*)args[0].ptr;
    zstr* addr = AS_STR(args[1]);
    SOCKTYPE s = AS_INT(zclassobj_get(p,".sock"));

    server.sin_family = AS_INT(zclassobj_get(p,".ipfamily"));
    server.sin_addr.s_addr = inet_addr(addr->val);
    server.sin_port = htons(args[2].i);
    
    int ret = ::bind(s,(sockaddr*)&server,sizeof(server));
    #ifdef _WIN32
      if (ret == SOCKET_ERROR)
      {
        string errMsg = to_string(WSAGetLastError());
        return quickErr(Error, errMsg);    
      }
    #else
      if(ret < 0)
        return quickErr(Error,strerror(errno));
    #endif
    return nil;
}
zobject socket_listen( zobject* args, int n)
{
    int e = validateArgTypes("oi", args, n);
    if (e == -1)
      return quickErr(ValueError, "2 arguments needed!");
    if (e!=-2)
      return quickErr(TypeError, "Argument " + to_string(e) + " is of invalid type.");
    if (((zclass_object*)args[0].ptr)->_klass != socketKlass)
      return quickErr(TypeError, "Error socket object needed!");
    zclass_object* p = (zclass_object*)args[0].ptr;
    
    SOCKTYPE s = AS_INT(zclassobj_get(p,".sock"));
    int i = listen(s, args[0].i);
    #ifdef _WIN32
      if (i == SOCKET_ERROR)
      {
        string errMsg = to_string(WSAGetLastError());
        return quickErr(Error, errMsg);
      }
    #else
      if (i < 0)
      {
          string errMsg = strerror(errno);
          return quickErr(Error, errMsg);
      }
    #endif
    
    return nil;
}
zobject socket_accept( zobject* args, int n)
{
    if (n != 1)
      return quickErr(ValueError, "1 argument needed");
  
    if (((zclass_object*)args[0].ptr)->_klass != socketKlass)
      return quickErr(TypeError, "Error socket object needed!");
    
    zclass_object* p = (zclass_object*)args[0].ptr;

    SOCKTYPE s = AS_INT(zclassobj_get(p,".sock"));
    struct sockaddr_in  client;
    int c = sizeof(client);
    #ifdef _WIN32
        SOCKTYPE new_socket = accept(s, (struct sockaddr*)&client, &c);
    #else
        SOCKTYPE new_socket = accept(s, (struct sockaddr*)&client, (socklen_t*)&c);
    #endif
    #ifdef _WIN32
      if (new_socket == INVALID_SOCKET)
      {
        string errMsg = to_string(WSAGetLastError());
        return quickErr(Error, errMsg);
      }
    #else
      if(new_socket <0)
        return quickErr(Error,strerror(errno));
    #endif
    
    zclass_object* d = vm_alloc_zclassobj(socketKlass);
    zclassobj_set(d,".sock",zobj_from_int(new_socket));
    return zobj_from_classobj(d);
    
}
zobject socket_connect( zobject* args, int n)
{
    int e = validateArgTypes("osi", args, n);
    if (e == -1)
      return quickErr(ValueError, "3 arguments needed!");
    if (e!=-2)
      return quickErr(TypeError, "Argument " + to_string(e) + " is of invalid type.");
    
    zclass_object* p = (zclass_object*)args[0].ptr;
    zstr* addr = AS_STR(args[1]);

    SOCKTYPE s = AS_INT(zclassobj_get(p,".sock"));

    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(addr->val);
    server.sin_family = AS_INT(zclassobj_get(p,".ipfamily"));
    server.sin_port = htons(args[2].i);
    int ret = connect(s, (struct sockaddr*)&server, sizeof(server));
    #ifdef _WIN32
      if (ret == SOCKET_ERROR)
      {
        string errMsg = to_string(WSAGetLastError());
        return quickErr(Error, errMsg);        
      }
    #else
      if(ret < 0)
        return quickErr(Error,strerror(errno));
    #endif
    return nil;
}
zobject socket_send(zobject* args, int n)
{
    int e = validateArgTypes("oc", args, n);
    if (e == -1)
      return quickErr(ValueError, "2 arguments needed!");
    if (e != -2)
        return quickErr(TypeError, "Argument " + to_string(e+1) + " is of invalid type.");
    zclass_object* p = (zclass_object*)args[0].ptr;
    if (p->_klass != socketKlass)
        return quickErr(TypeError, "Error socket object needed!");   

    SOCKTYPE s = AS_INT(zclassobj_get(p,".sock"));
    zbytearr* l = AS_BYTEARRAY(args[1]);
    int ret = send(s,(const char*)l->arr,l->size,0);
    #ifdef _WIN32
      if(ret == SOCKET_ERROR)
      {
        string errMsg = "Error code " + to_string(WSAGetLastError());
        return quickErr(Error, errMsg);
      }
    #else
      if(ret < 0)
        return quickErr(Error,strerror(errno));
    #endif
    return zobj_from_int(ret);
}
zobject socket_recv( zobject* args, int n)
{
    int e = validateArgTypes("oi", args, n);
    if (e == -1)
      return quickErr(ValueError, "1 argument needed!");
    if (e!=-2)
      return quickErr(TypeError, "Argument " + to_string(e) + " is of invalid type.");
    zclass_object* p = (zclass_object*)args[0].ptr;
    if (p->_klass != socketKlass)
      return quickErr(TypeError, "Error socket object needed!");     


    SOCKTYPE s = AS_INT(zclassobj_get(p,".sock"));
    char* msg= new char[(size_t)args[1].i];
    int read = recv(s, msg, args[1].i, 0);
    #ifdef _WIN32
      if (read == SOCKET_ERROR)
      {
        string errMsg = "Error code: " + to_string(WSAGetLastError());
        return quickErr(Error, errMsg);
      }
    #else
      if(read < 0)
      return quickErr(Error,strerror(errno));
    #endif

    auto l = vm_alloc_zbytearr();
    for (int i = 0; i < read; i++)
    {
        zbytearr_push(l,msg[i]);
    }
    delete[] msg;
    return zobj_from_bytearr(l);
}
zobject socket_recvfrom( zobject* args, int n)
{
    int e = validateArgTypes("oi", args, n);
    if (e == -1)
      return quickErr(ArgumentError, "2 argument needed!");
    if (e!=-2)
      return quickErr(TypeError, "Argument " + to_string(e) + " is of invalid type.");
    zclass_object* p = (zclass_object*)args[0].ptr;
    if (p->_klass != socketKlass)
        return quickErr(TypeError, "Error socket object needed!");
    

    SOCKTYPE s = AS_INT(zclassobj_get(p,".sock"));
    
    char* msg = new char[args[1].i];
    int len;
    struct sockaddr_in cliaddr;
    len = sizeof(cliaddr);  //
    #ifdef _WIN32
        int read = recvfrom(s, (char*)msg, args[1].i, 0, (struct sockaddr*)&cliaddr, &len);
    #else
        int read = recvfrom(s,(char*)msg,args[1].i, 0, (struct sockaddr*)&cliaddr, (socklen_t*)&len); 
    #endif
    #ifdef _WIN32
      if (read == SOCKET_ERROR)
      {
        string errMsg = to_string(WSAGetLastError());
        return quickErr(Error, errMsg);
      }
    #else
      if(read < 0)
        return quickErr(Error,strerror(errno));
    #endif

    auto l = vm_alloc_zbytearr();
    for (int i = 0; i < read; i++)
    {
        zbytearr_push(l,msg[i]);
    }
    zobject data;
    data.type = 'c';
    data.ptr = (void*)l;
    char* ip = inet_ntoa(cliaddr.sin_addr);

    zclass_object* d = vm_alloc_zclassobj(udpResKlass);
   
    zclassobj_set(d,"addr", zobj_from_str(ip));
    zclassobj_set(d,"data", data);
    delete[] msg;
    return zobj_from_classobj(d);
}
zobject socket_sendto( zobject* args, int n)
{
    int e = validateArgTypes("ocsi", args, n);
    if (e == -1)
        return quickErr(ValueError, "4 arguments needed!");
    if (e!=-2)
        return quickErr(TypeError, "Argument " + to_string(e) + " is of invalid type.");
    zclass_object* p = (zclass_object*)args[0].ptr;
    if (p->_klass != socketKlass)
        return quickErr(TypeError, "Error socket object needed!");
    
    SOCKTYPE s = AS_INT(zclassobj_get(p,".sock"));
    auto l = AS_BYTEARRAY(args[1]);
    
    int read;
    unsigned int len;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    // Filling server information
    addr.sin_family = AS_INT(zclassobj_get(p,".ipfamily"));
    addr.sin_port = htons(args[3].i);
    zstr* saddr = AS_STR(args[2]);
    addr.sin_addr.s_addr = inet_addr(saddr->val);
    //windows: (SOCKADDR*)
    int iResult = sendto(s,(const char*)l->arr,l->size, 0, (sockaddr*)&addr, sizeof(addr));
    #ifdef _WIN32
    if (iResult == SOCKET_ERROR)
    {
        string msg = to_string(GetLastError());
        return quickErr(Error, msg);
    }
    #else
      if(iResult < 0)
        return quickErr(Error,strerror(errno));
    #endif
    return nil;
}
zobject socket_close( zobject* args, int n)
{
    if (n != 1)
        return quickErr(ValueError, "1 arguments needed!.");
    if (args[0].type != 'o')
    {
        return quickErr(TypeError, "Error socket object needed!");
        
    }
    zclass_object* p = (zclass_object*)args[0].ptr;
    if (p->_klass != socketKlass)
    {
        return quickErr(TypeError, "Error socket object needed!");
    }
    
    SOCKTYPE s = AS_INT(zclassobj_get(p,".sock"));
    #ifdef _WIN32
      closesocket(s);
    #else
      close(s);
    #endif
    return nil;
}
