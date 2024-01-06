#include "socket.h"
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



int validateArgTypes(string e,ZObject* args,int n)
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
Klass* socketKlass;
Klass* udpResKlass;
ZObject nil;
EXPORT ZObject init()
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
    Module* d = vm_allocModule();
    socketKlass = vm_allocKlass();
    socketKlass->name = "socket";
    Klass_addNativeMethod(socketKlass,"__construct__",&socket__construct);
    Klass_addNativeMethod(socketKlass,"bind",&socket_Bind);
    Klass_addNativeMethod(socketKlass,"connect",  &socket_Connect);
    Klass_addNativeMethod(socketKlass,"send",  &socket_Send);
    Klass_addNativeMethod(socketKlass,"recv",  & socket_Recv);
    Klass_addNativeMethod(socketKlass,"listen",  & socket_Listen);
    Klass_addNativeMethod(socketKlass,"accept",  &socket_Accept);
    Klass_addNativeMethod(socketKlass,"sendto",  &socket_SendTo);
    Klass_addNativeMethod(socketKlass,"recvfrom", &socket_RecvFrom);
    Klass_addNativeMethod(socketKlass,"close",  & socket_Close);
    Klass_addNativeMethod(socketKlass,"__del__", &socket_del__);
    Klass_addMember(socketKlass,".internalPTR",nil );
    
    udpResKlass = vm_allocKlass();
    Klass_addMember(udpResKlass,("addr"), nil);
    Klass_addMember(udpResKlass,("data"), nil);


    Module_addKlass(d,"socket", socketKlass);
    Module_addKlass(d,"udpres", udpResKlass);
    Module_addMember(d,"AF_INET", ZObjFromInt(AF_INET));
    Module_addMember(d,"AF_INET6", ZObjFromInt(AF_INET6));
    Module_addMember(d,"SOCK_STREAM", ZObjFromInt(SOCK_STREAM));
    Module_addMember(d,"SOCK_DGRAM", ZObjFromInt(SOCK_DGRAM));
    Module_addMember(d,"SOCK_RAW", ZObjFromInt(SOCK_RAW));
    

    return ZObjFromModule(d);

}
EXPORT ZObject socket_del__( ZObject* args, int n)
{
    return nil;
}
inline ZObject quickErr(Klass* k,string msg) // uses std::string instead of cstring
{
  return Z_Err(k,msg.c_str());
}
EXPORT ZObject socket__construct(ZObject* args, int n)
{
  int e = validateArgTypes("oii", args, n);
  if (e == -1)
    return quickErr(ValueError, "3 arguments needed!");
  if (e!=-2)
    return quickErr(TypeError, "Argument " + to_string(e+1) + " is of invalid type.");
  if (args[0].type != Z_OBJ || ((KlassObject*)args[0].ptr)->klass != socketKlass)
    return quickErr(TypeError, "Argument 1 must be an object of socket class!");  
  
  KlassObject* ko = (KlassObject*)args[0].ptr;
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
  KlassObj_setMember(ko,".sock",ZObjFromInt(sock));
  KlassObj_setMember(ko,".ipfamily",args[1]);
  return nil;
}
EXPORT ZObject socket_Bind(ZObject* args, int n)
{
    int e = validateArgTypes("osi", args, n);
    if (e == -1)
      return quickErr(ValueError, "3 arguments needed!");
    if (e!=-2)
      return quickErr(TypeError, "Argument " + to_string(e+1) + " is of invalid type.");
        
    if (args[0].type != Z_OBJ || ((KlassObject*)args[0].ptr)->klass != socketKlass)
      return quickErr(TypeError, "Argument 1 must be an object of socket class!");  
  
    sockaddr_in server;
    
    KlassObject* p = (KlassObject*)args[0].ptr;
    ZStr* addr = AS_STR(args[1]);
    SOCKTYPE s = AS_INT(KlassObj_getMember(p,".sock"));

    server.sin_family = AS_INT(KlassObj_getMember(p,".ipfamily"));
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
EXPORT ZObject socket_Listen( ZObject* args, int n)
{
    int e = validateArgTypes("oi", args, n);
    if (e == -1)
      return quickErr(ValueError, "2 arguments needed!");
    if (e!=-2)
      return quickErr(TypeError, "Argument " + to_string(e) + " is of invalid type.");
    if (((KlassObject*)args[0].ptr)->klass != socketKlass)
      return quickErr(TypeError, "Error socket object needed!");
    KlassObject* p = (KlassObject*)args[0].ptr;
    
    SOCKTYPE s = AS_INT(KlassObj_getMember(p,".sock"));
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
EXPORT ZObject socket_Accept( ZObject* args, int n)
{
    if (n != 1)
      return quickErr(ValueError, "1 argument needed");
  
    if (((KlassObject*)args[0].ptr)->klass != socketKlass)
      return quickErr(TypeError, "Error socket object needed!");
    
    KlassObject* p = (KlassObject*)args[0].ptr;

    SOCKTYPE s = AS_INT(KlassObj_getMember(p,".sock"));
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
    
    KlassObject* d = vm_allocKlassObject(socketKlass);
    KlassObj_setMember(d,".sock",ZObjFromInt(new_socket));
    return ZObjFromKlassObj(d);
    
}
EXPORT ZObject socket_Connect( ZObject* args, int n)
{
    int e = validateArgTypes("osi", args, n);
    if (e == -1)
      return quickErr(ValueError, "3 arguments needed!");
    if (e!=-2)
      return quickErr(TypeError, "Argument " + to_string(e) + " is of invalid type.");
    
    KlassObject* p = (KlassObject*)args[0].ptr;
    ZStr* addr = AS_STR(args[1]);

    SOCKTYPE s = AS_INT(KlassObj_getMember(p,".sock"));

    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(addr->val);
    server.sin_family = AS_INT(KlassObj_getMember(p,".ipfamily"));
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
EXPORT ZObject socket_Send(ZObject* args, int n)
{
    int e = validateArgTypes("oc", args, n);
    if (e == -1)
      return quickErr(ValueError, "2 arguments needed!");
    if (e != -2)
        return quickErr(TypeError, "Argument " + to_string(e+1) + " is of invalid type.");
    KlassObject* p = (KlassObject*)args[0].ptr;
    if (p->klass != socketKlass)
        return quickErr(TypeError, "Error socket object needed!");   

    SOCKTYPE s = AS_INT(KlassObj_getMember(p,".sock"));
    ZByteArr* l = AS_BYTEARRAY(args[1]);
    int ret = send(s,l->arr,l->size,0);
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
    return ZObjFromInt(ret);
}
EXPORT ZObject socket_Recv( ZObject* args, int n)
{
    int e = validateArgTypes("oi", args, n);
    if (e == -1)
      return quickErr(ValueError, "1 argument needed!");
    if (e!=-2)
      return quickErr(TypeError, "Argument " + to_string(e) + " is of invalid type.");
    KlassObject* p = (KlassObject*)args[0].ptr;
    if (p->klass != socketKlass)
      return quickErr(TypeError, "Error socket object needed!");     


    SOCKTYPE s = AS_INT(KlassObj_getMember(p,".sock"));
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

    auto l = vm_allocByteArray();
    for (int i = 0; i < read; i++)
    {
        ZByteArr_push(l,msg[i]);
    }
    delete[] msg;
    return ZObjFromByteArr(l);
}
EXPORT ZObject socket_RecvFrom( ZObject* args, int n)
{
    int e = validateArgTypes("oi", args, n);
    if (e == -1)
      return quickErr(ArgumentError, "2 argument needed!");
    if (e!=-2)
      return quickErr(TypeError, "Argument " + to_string(e) + " is of invalid type.");
    KlassObject* p = (KlassObject*)args[0].ptr;
    if (p->klass != socketKlass)
        return quickErr(TypeError, "Error socket object needed!");
    

    SOCKTYPE s = AS_INT(KlassObj_getMember(p,".sock"));
    
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

    auto l = vm_allocByteArray();
    for (int i = 0; i < read; i++)
    {
        ZByteArr_push(l,msg[i]);
    }
    ZObject data;
    data.type = 'c';
    data.ptr = (void*)l;
    char* ip = inet_ntoa(cliaddr.sin_addr);

    KlassObject* d = vm_allocKlassObject(udpResKlass);
   
    KlassObj_setMember(d,"addr", ZObjFromStr(ip));
    KlassObj_setMember(d,"data", data);
    delete[] msg;
    return ZObjFromKlassObj(d);
}
EXPORT ZObject socket_SendTo( ZObject* args, int n)
{
    int e = validateArgTypes("ocsi", args, n);
    if (e == -1)
        return quickErr(ValueError, "4 arguments needed!");
    if (e!=-2)
        return quickErr(TypeError, "Argument " + to_string(e) + " is of invalid type.");
    KlassObject* p = (KlassObject*)args[0].ptr;
    if (p->klass != socketKlass)
        return quickErr(TypeError, "Error socket object needed!");
    
    SOCKTYPE s = AS_INT(KlassObj_getMember(p,".sock"));
    auto l = AS_BYTEARRAY(args[1]);
    
    int read;
    unsigned int len;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    // Filling server information
    addr.sin_family = AS_INT(KlassObj_getMember(p,".ipfamily"));
    addr.sin_port = htons(args[3].i);
    ZStr* saddr = AS_STR(args[2]);
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
EXPORT ZObject socket_Close( ZObject* args, int n)
{
    if (n != 1)
        return quickErr(ValueError, "1 arguments needed!.");
    if (args[0].type != 'o')
    {
        return quickErr(TypeError, "Error socket object needed!");
        
    }
    KlassObject* p = (KlassObject*)args[0].ptr;
    if (p->klass != socketKlass)
    {
        return quickErr(TypeError, "Error socket object needed!");
    }
    
    SOCKTYPE s = AS_INT(KlassObj_getMember(p,".sock"));
    #ifdef _WIN32
      closesocket(s);
    #else
      close(s);
    #endif
    return nil;
}