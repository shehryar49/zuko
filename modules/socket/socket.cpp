#include "socket.h"
#include <string.h>
#include <stdlib.h>
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
          return Z_Err(Error, errMsg);
      }
    #endif
    Module* d = vm_allocModule();
    socketKlass = vm_allocKlass();
    socketKlass->name = "socket";
    socketKlass->members.emplace(("__construct__"), ZObjFromMethod("",&socket__construct,socketKlass));
    socketKlass->members.emplace((".internalPTR"),nil );
    socketKlass->members.emplace(("bind"), ZObjFromMethod("bind", &socket_Bind, socketKlass));
    socketKlass->members.emplace(("connect"), ZObjFromMethod("connect", &socket_Connect, socketKlass));
    socketKlass->members.emplace(("send"), ZObjFromMethod("send", &socket_Send, socketKlass));
    socketKlass->members.emplace(("recv"), ZObjFromMethod("recv", & socket_Recv, socketKlass));
    socketKlass->members.emplace(("listen"), ZObjFromMethod("listen", & socket_Listen, socketKlass));
    socketKlass->members.emplace(("accept"), ZObjFromMethod("accept", &socket_Accept, socketKlass));
    socketKlass->members.emplace(("sendto"), ZObjFromMethod("sendto", &socket_SendTo, socketKlass));
    socketKlass->members.emplace(("recvfrom"), ZObjFromMethod("recvfrom", &socket_RecvFrom, socketKlass));
    socketKlass->members.emplace(("close"), ZObjFromMethod("close", & socket_Close, socketKlass));
    socketKlass->members.emplace(("__del__"), ZObjFromMethod("__del__", &socket_del__, socketKlass));
    
    
    udpResKlass = vm_allocKlass();
    udpResKlass->members.emplace(("addr"), nil);
    udpResKlass->members.emplace(("data"), nil);


    d->members.emplace(("socket"), ZObjFromKlass(socketKlass));
    d->members.emplace(("socket"), ZObjFromKlass(udpResKlass));
    d->members.emplace(("AF_INET"), ZObjFromInt(AF_INET));
    d->members.emplace(("AF_INET6"), ZObjFromInt(AF_INET6));
    d->members.emplace(("SOCK_STREAM"), ZObjFromInt(SOCK_STREAM));
    d->members.emplace(("SOCK_DGRAM"), ZObjFromInt(SOCK_DGRAM));
    d->members.emplace(("SOCK_RAW"), ZObjFromInt(SOCK_RAW));
    

    return ZObjFromModule(d);

}
EXPORT ZObject socket_del__( ZObject* args, int n)
{
    return nil;
}
EXPORT ZObject socket__construct(ZObject* args, int n)
{
  int e = validateArgTypes("oii", args, n);
  if (e == -1)
    return Z_Err(ValueError, "3 arguments needed!");
  if (e!=-2)
    return Z_Err(TypeError, "Argument " + to_string(e+1) + " is of invalid type.");
  if (args[0].type != Z_OBJ || ((KlassObject*)args[0].ptr)->klass != socketKlass)
    return Z_Err(TypeError, "Argument 1 must be an object of socket class!");  
  
  KlassObject& ko = *(KlassObject*)args[0].ptr;
  SOCKTYPE sock = socket(args[1].i, args[2].i, 0);  
  #ifdef _WIN32
    if (sock == INVALID_SOCKET)
    {
      string errMsg = "Error code " + to_string(WSAGetLastError());
      return Z_Err(Error, errMsg);      
    }
  #else
    if(sock < 0)
      return Z_Err(Error,strerror(errno));
  #endif
  ko.members[".sock"] = ZObjFromInt(sock);
  ko.members[".ipfamily"] = args[1];
  return nil;
}
EXPORT ZObject socket_Bind(ZObject* args, int n)
{
    int e = validateArgTypes("osi", args, n);
    if (e == -1)
      return Z_Err(ValueError, "3 arguments needed!");
    if (e!=-2)
      return Z_Err(TypeError, "Argument " + to_string(e+1) + " is of invalid type.");
        
    if (args[0].type != Z_OBJ || ((KlassObject*)args[0].ptr)->klass != socketKlass)
      return Z_Err(TypeError, "Argument 1 must be an object of socket class!");  
  
    sockaddr_in server;
    
    KlassObject* p = (KlassObject*)args[0].ptr;
    string& addr = *(string*)args[1].ptr;
    SOCKTYPE s = p->members[".sock"].i;

    server.sin_family = p->members[".ipfamily"].i;
    server.sin_addr.s_addr = inet_addr(addr.c_str());
    server.sin_port = htons(args[2].i);
    
    int ret = bind(s,(sockaddr*)&server,sizeof(server));
    #ifdef _WIN32
      if (ret == SOCKET_ERROR)
      {
        string errMsg = to_string(WSAGetLastError());
        return Z_Err(Error, errMsg);    
      }
    #else
      if(ret < 0)
        return Z_Err(Error,strerror(errno));
    #endif
    return nil;
}
EXPORT ZObject socket_Listen( ZObject* args, int n)
{
    int e = validateArgTypes("oi", args, n);
    if (e == -1)
      return Z_Err(ValueError, "2 arguments needed!");
    if (e!=-2)
      return Z_Err(TypeError, "Argument " + to_string(e) + " is of invalid type.");
    if (((KlassObject*)args[0].ptr)->klass != socketKlass)
      return Z_Err(TypeError, "Error socket object needed!");
    KlassObject* p = (KlassObject*)args[0].ptr;
    
    SOCKTYPE s = p->members[".sock"].i;
    int i = listen(s, args[0].i);
    #ifdef _WIN32
      if (i == SOCKET_ERROR)
      {
        string errMsg = to_string(WSAGetLastError());
        return Z_Err(Error, errMsg);
      }
    #else
      if (i < 0)
      {
          string errMsg = strerror(errno);
          return Z_Err(Error, errMsg);
      }
    #endif
    
    return nil;
}
EXPORT ZObject socket_Accept( ZObject* args, int n)
{
    if (n != 1)
      return Z_Err(ValueError, "1 argument needed");
  
    if (((KlassObject*)args[0].ptr)->klass != socketKlass)
      return Z_Err(TypeError, "Error socket object needed!");
    
    KlassObject* p = (KlassObject*)args[0].ptr;

    SOCKTYPE s = p->members[".sock"].i;
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
        return Z_Err(Error, errMsg);
      }
    #else
      if(new_socket <0)
        return Z_Err(Error,strerror(errno));
    #endif
    
    KlassObject* d = vm_allocKlassObject();
    d->klass = socketKlass;
    d->members = socketKlass->members;
    d->members[".sock"] = ZObjFromInt(new_socket);
    return ZObjFromKlassObj(d);
    
}
EXPORT ZObject socket_Connect( ZObject* args, int n)
{
    int e = validateArgTypes("osi", args, n);
    if (e == -1)
      return Z_Err(ValueError, "3 arguments needed!");
    if (e!=-2)
      return Z_Err(TypeError, "Argument " + to_string(e) + " is of invalid type.");
    
    KlassObject* p = (KlassObject*)args[0].ptr;
    string& addr = *(string*)args[1].ptr;

    SOCKTYPE s = p->members[".sock"].i;

    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(addr.c_str());
    server.sin_family = p->members[".ipfamily"].i;
    server.sin_port = htons(args[2].i);
    int ret = connect(s, (struct sockaddr*)&server, sizeof(server));
    #ifdef _WIN32
      if (ret == SOCKET_ERROR)
      {
        string errMsg = to_string(WSAGetLastError());
        return Z_Err(Error, errMsg);        
      }
    #else
      if(ret < 0)
        return Z_Err(Error,strerror(errno));
    #endif
    return nil;
}
EXPORT ZObject socket_Send(ZObject* args, int n)
{
    int e = validateArgTypes("oc", args, n);
    if (e == -1)
      return Z_Err(ValueError, "2 arguments needed!");
    if (e != -2)
        return Z_Err(TypeError, "Argument " + to_string(e+1) + " is of invalid type.");
    KlassObject* p = (KlassObject*)args[0].ptr;
    if (p->klass != socketKlass)
        return Z_Err(TypeError, "Error socket object needed!");   

    SOCKTYPE s = p->members[".sock"].i;
    auto& l = *(vector<uint8_t>*)args[1].ptr;
    int ret = send(s,(const char*)&l[0],l.size(),0);
    #ifdef _WIN32
      if(ret == SOCKET_ERROR)
      {
        string errMsg = "Error code " + to_string(WSAGetLastError());
        return Z_Err(Error, errMsg);
      }
    #else
      if(ret < 0)
        return Z_Err(Error,strerror(errno));
    #endif
    return ZObjFromInt(ret);
}
EXPORT ZObject socket_Recv( ZObject* args, int n)
{
    int e = validateArgTypes("oi", args, n);
    if (e == -1)
      return Z_Err(ValueError, "1 argument needed!");
    if (e!=-2)
      return Z_Err(TypeError, "Argument " + to_string(e) + " is of invalid type.");
    KlassObject* p = (KlassObject*)args[0].ptr;
    if (p->klass != socketKlass)
      return Z_Err(TypeError, "Error socket object needed!");     


    SOCKTYPE s = p->members[".sock"].i;
    char* msg= new char[(size_t)args[1].i];
    int read = recv(s, msg, args[1].i, 0);
    #ifdef _WIN32
      if (read == SOCKET_ERROR)
      {
        string errMsg = "Error code: " + to_string(WSAGetLastError());
        return Z_Err(Error, errMsg);
      }
    #else
      if(read < 0)
      return Z_Err(Error,strerror(errno));
    #endif

    auto l = vm_allocByteArray();
    for (int i = 0; i < read; i++)
    {
        l->push_back(msg[i]);
    }
    delete[] msg;
    return ZObjFromByteArr(l);
}
EXPORT ZObject socket_RecvFrom( ZObject* args, int n)
{
    int e = validateArgTypes("oi", args, n);
    if (e == -1)
      return Z_Err(ArgumentError, "2 argument needed!");
    if (e!=-2)
      return Z_Err(TypeError, "Argument " + to_string(e) + " is of invalid type.");
    KlassObject* p = (KlassObject*)args[0].ptr;
    if (p->klass != socketKlass)
        return Z_Err(TypeError, "Error socket object needed!");
    

    SOCKTYPE s = p->members[".sock"].i;
    
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
        return Z_Err(Error, errMsg);
      }
    #else
      if(read < 0)
        return Z_Err(Error,strerror(errno));
    #endif

    auto l = vm_allocByteArray();
    for (int i = 0; i < read; i++)
    {
        l->push_back(msg[i]);
    }
    ZObject data;
    data.type = 'c';
    data.ptr = (void*)l;
    char* ip = inet_ntoa(cliaddr.sin_addr);
    string IP = ip;
    

    KlassObject* d = vm_allocKlassObject();
    d->klass = udpResKlass;

    d->members.emplace(("addr"), ZObjFromStr(IP));
    d->members.emplace(("data"), data);
    delete[] msg;
    return ZObjFromKlassObj(d);
}
EXPORT ZObject socket_SendTo( ZObject* args, int n)
{
    int e = validateArgTypes("ocsi", args, n);
    if (e == -1)
        return Z_Err(ValueError, "4 arguments needed!");
    if (e!=-2)
        return Z_Err(TypeError, "Argument " + to_string(e) + " is of invalid type.");
    KlassObject* p = (KlassObject*)args[0].ptr;
    if (p->klass != socketKlass)
        return Z_Err(TypeError, "Error socket object needed!");
    
    SOCKTYPE s = p->members[".sock"].i;
    auto& l = *(vector<uint8_t>*)args[1].ptr;
    
    int read;
    unsigned int len;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    // Filling server information
    addr.sin_family = p->members[".ipfamily"].i;
    addr.sin_port = htons(args[3].i);
    string& saddr = *(string*)args[2].ptr;
    addr.sin_addr.s_addr = inet_addr(saddr.c_str());
    //windows: (SOCKADDR*)
    int iResult = sendto(s,(const char*)&l[0],l.size(), 0, (sockaddr*)&addr, sizeof(addr));
    #ifdef _WIN32
    if (iResult == SOCKET_ERROR)
    {
        string msg = to_string(GetLastError());
        return Z_Err(Error, msg);
    }
    #else
      if(iResult < 0)
        return Z_Err(Error,strerror(errno));
    #endif
    return nil;
}
EXPORT ZObject socket_Close( ZObject* args, int n)
{
    if (n != 1)
        return Z_Err(ValueError, "1 arguments needed!.");
    if (args[0].type != 'o')
    {
        return Z_Err(TypeError, "Error socket object needed!");
        
    }
    KlassObject* p = (KlassObject*)args[0].ptr;
    if (p->klass != socketKlass)
    {
        return Z_Err(TypeError, "Error socket object needed!");
    }
    
    SOCKTYPE s = p->members[".sock"].i;
    #ifdef _WIN32
      closesocket(s);
    #else
      close(s);
    #endif
    return nil;
}