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
  #define  SOCKTYPE int
#endif
using namespace std;



int validateArgTypes(string e,PltObject* args,int n)
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
PltObject nil;
EXPORT PltObject init()
{
    nil.type = 'n';
    #ifdef _WIN32
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        {
            string errMsg = "Error code: " + to_string(WSAGetLastError());
            return Plt_Err(Error, errMsg);
        }
    #endif
    Module* d = vm_allocModule();
    socketKlass = vm_allocKlass();
    socketKlass->name = "socket";
    socketKlass->members.emplace(("__construct__"), PObjFromMethod("",&socket__construct,socketKlass));
    socketKlass->members.emplace((".internalPTR"),nil );
    socketKlass->members.emplace(("bind"), PObjFromMethod("bind", &socket_Bind, socketKlass));
    socketKlass->members.emplace(("connect"), PObjFromMethod("connect", &socket_Connect, socketKlass));
    socketKlass->members.emplace(("send"), PObjFromMethod("send", &socket_Send, socketKlass));
    socketKlass->members.emplace(("recv"), PObjFromMethod("recv", & socket_Recv, socketKlass));
    socketKlass->members.emplace(("listen"), PObjFromMethod("listen", & socket_Listen, socketKlass));
    socketKlass->members.emplace(("accept"), PObjFromMethod("accept", &socket_Accept, socketKlass));
    socketKlass->members.emplace(("sendto"), PObjFromMethod("sendto", &socket_SendTo, socketKlass));
    socketKlass->members.emplace(("recvfrom"), PObjFromMethod("recvfrom", &socket_RecvFrom, socketKlass));
    socketKlass->members.emplace(("close"), PObjFromMethod("close", & socket_Close, socketKlass));
    socketKlass->members.emplace(("__del__"), PObjFromMethod("__del__", &socket_del__, socketKlass));
    
    udpResKlass = vm_allocKlass();
    udpResKlass->members.emplace(("addr"), nil);
    udpResKlass->members.emplace(("data"), nil);


    d->members.emplace(("socket"), PObjFromKlass(socketKlass));
    d->members.emplace(("socket"), PObjFromKlass(udpResKlass));
    d->members.emplace(("AF_INET"), PObjFromInt(AF_INET));
    d->members.emplace(("AF_INET6"), PObjFromInt(AF_INET6));
    d->members.emplace(("SOCK_STREAM"), PObjFromInt(SOCK_STREAM));
    d->members.emplace(("SOCK_DGRAM"), PObjFromInt(SOCK_DGRAM));

    return PObjFromModule(d);

}
EXPORT PltObject socket_del__( PltObject* args, int n)
{
    return nil;
}
EXPORT PltObject socket__construct(PltObject* args, int n)
{
  int e = validateArgTypes("oii", args, n);
  if (e == -1)
    return Plt_Err(ValueError, "3 arguments needed!");
  if (e!=-2)
    return Plt_Err(TypeError, "Argument " + to_string(e+1) + " is of invalid type.");
  if (args[0].type != PLT_OBJ || ((KlassObject*)args[0].ptr)->klass != socketKlass)
    return Plt_Err(TypeError, "Argument 1 must be an object of socket class!");  
  
  KlassObject& ko = *(KlassObject*)args[0].ptr;
  SOCKTYPE sock = socket(args[1].i, args[2].i, 0);  
  #ifdef _WIN32
    if (s->socket_desc == INVALID_SOCKET)
    {
      string errMsg = "Error code " + to_string(WSAGetLastError());
      return Plt_Err(Error, errMsg);      
    }
  #else
    if(sock < 0)
      return Plt_Err(Error,strerror(errno));
  #endif
  ko.members[".sock"] = PObjFromInt(sock);
  ko.members[".ipfamily"] = args[1];
  return nil;
}
EXPORT PltObject socket_Bind(PltObject* args, int n)
{
    int e = validateArgTypes("osi", args, n);
    if (e == -1)
      return Plt_Err(ValueError, "3 arguments needed!");
    if (e!=-2)
      return Plt_Err(TypeError, "Argument " + to_string(e+1) + " is of invalid type.");
        
    if (args[0].type != PLT_OBJ || ((KlassObject*)args[0].ptr)->klass != socketKlass)
      return Plt_Err(TypeError, "Argument 1 must be an object of socket class!");  
  
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
        return Plt_Err(Error, errMsg);    
      }
    #else
      if(ret < 0)
        return Plt_Err(Error,strerror(errno));
    #endif
    return nil;
}
EXPORT PltObject socket_Listen( PltObject* args, int n)
{
    int e = validateArgTypes("oi", args, n);
    if (e == -1)
      return Plt_Err(ValueError, "2 arguments needed!");
    if (e!=-2)
      return Plt_Err(TypeError, "Argument " + to_string(e) + " is of invalid type.");
    if (((KlassObject*)args[0].ptr)->klass != socketKlass)
      return Plt_Err(TypeError, "Error socket object needed!");
    KlassObject* p = (KlassObject*)args[0].ptr;
    
    SOCKTYPE s = p->members[".sock"].i;
    if (listen(s, args[0].i) < 0)
    {
        string errMsg = strerror(errno);
        return Plt_Err(Error, errMsg);
    }
    return nil;
}
EXPORT PltObject socket_Accept( PltObject* args, int n)
{
    if (n != 1)
      return Plt_Err(ValueError, "1 argument needed");
  
    if (((KlassObject*)args[0].ptr)->klass != socketKlass)
      return Plt_Err(TypeError, "Error socket object needed!");
    
    KlassObject* p = (KlassObject*)args[0].ptr;

    SOCKTYPE s = p->members[".sock"].i;
    struct sockaddr_in  client;
    int c = sizeof(client);
    SOCKTYPE new_socket = accept(s, (struct sockaddr*)&client, (socklen_t*)&c);
    #ifdef _WIN32
      if (new_socket == INVALID_SOCKET)
      {
        string errMsg = to_string(WSAGetLastError());
        return Plt_Err(Error, errMsg);
      }
    #else
      if(new_socket <0)
        return Plt_Err(Error,strerror(errno));
    #endif
    
    KlassObject* d = vm_allocKlassObject();
    d->klass = socketKlass;
    d->members = socketKlass->members;
    d->members[".sock"] = PObjFromInt(new_socket);
    return PObjFromKlassObj(d);
    
}
EXPORT PltObject socket_Connect( PltObject* args, int n)
{
    int e = validateArgTypes("osi", args, n);
    if (e == -1)
      return Plt_Err(ValueError, "3 arguments needed!");
    if (e!=-2)
      return Plt_Err(TypeError, "Argument " + to_string(e) + " is of invalid type.");
    
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
        return Plt_Err(Error, errMsg);        
      }
    #else
      if(ret < 0)
        return Plt_Err(Error,strerror(errno));
    #endif
    return nil;
}
EXPORT PltObject socket_Send(PltObject* args, int n)
{
    int e = validateArgTypes("oc", args, n);
    if (e == -1)
      return Plt_Err(ValueError, "2 arguments needed!");
    if (e != -2)
        return Plt_Err(TypeError, "Argument " + to_string(e+1) + " is of invalid type.");
    KlassObject* p = (KlassObject*)args[0].ptr;
    if (p->klass != socketKlass)
        return Plt_Err(TypeError, "Error socket object needed!");   

    SOCKTYPE s = p->members[".sock"].i;
    auto& l = *(vector<uint8_t>*)args[1].ptr;
    int ret = send(s,(const char*)&l[0],l.size(),0);
    #ifdef _WIN32
      if(ret == SOCKET_ERROR)
      {
        string errMsg = "Error code " + to_string(WSAGetLastError());
        return Plt_Err(Error, errMsg);
      }
    #else
      if(ret < 0)
        return Plt_Err(Error,strerror(errno));
    #endif
    return PObjFromInt(ret);
}
EXPORT PltObject socket_Recv( PltObject* args, int n)
{
    int e = validateArgTypes("oi", args, n);
    if (e == -1)
      return Plt_Err(ValueError, "1 argument needed!");
    if (e!=-2)
      return Plt_Err(TypeError, "Argument " + to_string(e) + " is of invalid type.");
    KlassObject* p = (KlassObject*)args[0].ptr;
    if (p->klass != socketKlass)
      return Plt_Err(TypeError, "Error socket object needed!");     


    SOCKTYPE s = p->members[".sock"].i;
    char* msg= new char[(size_t)args[1].i];
    int read = recv(s, msg, args[1].i, 0);
    #ifdef _WIN32
      if (read == SOCKET_ERROR)
      {
        string errMsg = "Error code: " + to_string(WSAGetLastError());
        return Plt_Err(Error, errMsg);
      }
    #else
      if(read < 0)
      return Plt_Err(Error,strerror(errno));
    #endif

    auto l = vm_allocByteArray();
    for (int i = 0; i < read; i++)
    {
        l->push_back(msg[i]);
    }
    delete[] msg;
    return PObjFromByteArr(l);
}
EXPORT PltObject socket_RecvFrom( PltObject* args, int n)
{
    int e = validateArgTypes("oi", args, n);
    if (e == -1)
      return Plt_Err(ArgumentError, "2 argument needed!");
    if (e!=-2)
      return Plt_Err(TypeError, "Argument " + to_string(e) + " is of invalid type.");
    KlassObject* p = (KlassObject*)args[0].ptr;
    if (p->klass != socketKlass)
        return Plt_Err(TypeError, "Error socket object needed!");
    

    SOCKTYPE s = p->members[".sock"].i;
    
    char* msg = new char[args[1].i];
    int len;
    struct sockaddr_in cliaddr;
    len = sizeof(cliaddr);  //
    int read = recvfrom(s,(char*)msg,args[1].i, 0, (struct sockaddr*)&cliaddr, (socklen_t*)&len); 
    #ifdef _WIN32
      if (read == SOCKET_ERROR)
      {
        string errMsg = to_string(WSAGetLastError());
        return Plt_Err(Error, errMsg);
      }
    #else
      if(read < 0)
        return Plt_Err(Error,strerror(errno));
    #endif

    auto l = vm_allocByteArray();
    for (int i = 0; i < read; i++)
    {
        l->push_back(msg[i]);
    }
    PltObject data;
    data.type = 'c';
    data.ptr = (void*)l;
    char* ip = inet_ntoa(cliaddr.sin_addr);
    string IP = ip;
    

    KlassObject* d = vm_allocKlassObject();
    d->klass = udpResKlass;

    d->members.emplace(("addr"), PObjFromStr(IP));
    d->members.emplace(("data"), data);
    delete[] msg;
    return PObjFromKlassObj(d);
}
EXPORT PltObject socket_SendTo( PltObject* args, int n)
{
    int e = validateArgTypes("ocsi", args, n);
    if (e == -1)
        return Plt_Err(ValueError, "4 arguments needed!");
    if (e!=-2)
        return Plt_Err(TypeError, "Argument " + to_string(e) + " is of invalid type.");
    KlassObject* p = (KlassObject*)args[0].ptr;
    if (p->klass != socketKlass)
        return Plt_Err(TypeError, "Error socket object needed!");
    
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
        return Plt_Err(Error, msg);
    }
    #else
      if(iResult < 0)
        return Plt_Err(Error,strerror(errno));
    #endif
    return nil;
}
EXPORT PltObject socket_Close( PltObject* args, int n)
{
    if (n != 1)
        return Plt_Err(ValueError, "1 arguments needed!.");
    if (args[0].type != 'o')
    {
        return Plt_Err(TypeError, "Error socket object needed!");
        
    }
    KlassObject* p = (KlassObject*)args[0].ptr;
    if (p->klass != socketKlass)
    {
        return Plt_Err(TypeError, "Error socket object needed!");
    }
    
    SOCKTYPE s = p->members[".sock"].i;
    #ifdef _WIN32
      closesocket(s);
    #else
      close(s);
    #endif
    return nil;
}