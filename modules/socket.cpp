#include "pch.h"
#include "socket.h"
#include "C:\plutonium\PltObject.h"
#include <winsock2.h>
using namespace std;

#pragma comment(lib,"ws2_32")

struct Socket //wrapper around winsock object
{
    SOCKET socket_desc;
    string ip;
    int ipfamily;
    int connMethod;
    int port;
    bool open;

};
   
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
		WSADATA wsa;
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		{
			string errMsg = "Error code: " + to_string(WSAGetLastError());
			return Plt_Err(UNKNOWN_ERROR, errMsg);
		}
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
        socketKlass->members.emplace((".destroy"), PObjFromMethod(".destroy", &socket_Destroy, socketKlass));
        
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
     EXPORT PltObject socket_Destroy( PltObject* args, int n)
     {
         KlassInstance* d = (KlassInstance*)args[0].ptr;
         
         if (d->members[".internalPTR"].type == 'n')
             return nil;
         Socket* s = (Socket*)d->members[".internalPTR"].ptr;
         delete s;
         return nil;
     }
     EXPORT PltObject socket__construct(PltObject* args, int n)
     {
         int e = validateArgTypes("oii", args, n);
         if (e == -1)
         {
             return Plt_Err(VALUE_ERROR, "3 arguments needed!");
             
         }
         if (e!=-2)
         {
             return Plt_Err(TYPE_ERROR, "Argument " + to_string(e+1) + " is of invalid type.");
             
         }
         if (((KlassInstance*)args[0].ptr)->klass != socketKlass)
         {
             return Plt_Err(TYPE_ERROR, "Argument 1 must be a socket object!");
             
         }
         Socket* s = new Socket;
         s->socket_desc = socket(args[1].i, args[2].i, 0);
         
         if (s->socket_desc == INVALID_SOCKET) {
             string errMsg = "Error code " + to_string(WSAGetLastError());
             return Plt_Err(UNKNOWN_ERROR, errMsg);
             
         }
         s->ipfamily = args[1].i;
         s->connMethod = args[2].i;
         s->open = true;
         KlassInstance* k = ((KlassInstance*)args[0].ptr);
         k->members[".internalPTR"].ptr = (void*)s;
         return nil;
     }
     EXPORT PltObject socket_Bind(PltObject* args, int n)
     {
         int e = validateArgTypes("osi", args, n);
         if (e == -1)
         {
             return Plt_Err(VALUE_ERROR, "3 arguments needed!");
             
         }
         if (e!=-2)
         {
             return Plt_Err(TYPE_ERROR, "Argument " + to_string(e+1) + " is of invalid type.");
             
         }
         if (((KlassInstance*)args[0].ptr)->klass != socketKlass)
         {
             return Plt_Err(TYPE_ERROR, "Error socket object needed!");
             
         }

         struct sockaddr_in server;
         KlassInstance* p = (KlassInstance*)args[0].ptr;
         string& addr = *(string*)args[1].ptr;
         
         Socket* s = (Socket*)p->members[".internalPTR"].ptr;
         server.sin_family = s->ipfamily;
         server.sin_addr.s_addr = inet_addr(addr.c_str());
         server.sin_port = htons(args[2].i);
         s->port = args[2].i;
         s->ip = addr;
         if (bind(s->socket_desc, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
         {
             string errMsg = to_string(WSAGetLastError());
             return Plt_Err(UNKNOWN_ERROR, errMsg);
             
         }
         return nil;
     }
     EXPORT PltObject socket_Listen( PltObject* args, int n)
     {
         int e = validateArgTypes("oi", args, n);
         if (e == -1)
         {
             return Plt_Err(VALUE_ERROR, "2 arguments needed!");
             
         }
         if (e!=-2)
         {
             return Plt_Err(TYPE_ERROR, "Argument " + to_string(e) + " is of invalid type.");
             
         }
         if (((KlassInstance*)args[0].ptr)->klass != socketKlass)
         {
             return Plt_Err(TYPE_ERROR, "Error socket object needed!");
             
         }
         KlassInstance* p = (KlassInstance*)args[0].ptr;
         
         Socket* s = (Socket*)p->members[".internalPTR"].ptr;
         if (listen(s->socket_desc, args[0].i) < 0)
         {
             string errMsg = strerror(errno);
             return Plt_Err(UNKNOWN_ERROR, errMsg);
         }
         return nil;
     }
     EXPORT PltObject socket_Accept( PltObject* args, int n)
     {
         if (n != 1)
         {
             return Plt_Err(VALUE_ERROR, "1 argument needed");
             
         }
         if (((KlassInstance*)args[0].ptr)->klass != socketKlass)
         {
             return Plt_Err(TYPE_ERROR, "Error socket object needed!");
             
         }
         KlassInstance* p = (KlassInstance*)args[0].ptr;
         

         Socket* s = (Socket*)p->members[".internalPTR"].ptr;
         struct sockaddr_in  client;
         int c = sizeof(client);
         int new_socket = accept(s->socket_desc, (struct sockaddr*)&client, &c);
         if (new_socket == INVALID_SOCKET)
         {
             string errMsg = to_string(WSAGetLastError());
             return Plt_Err(UNKNOWN_ERROR, errMsg);
             
         }
         Socket* A = new Socket;
         A->socket_desc = new_socket;
         A->port = s->port;
         A->ipfamily = s->ipfamily;
         A->connMethod = s->connMethod;
         A->open = true;
         KlassInstance* d = vm_allocKlassInstance();
         d->klass = socketKlass;
         d->members.emplace(("__construct__"), PObjFromMethod("", &socket__construct, socketKlass));
         d->members.emplace((".internalPTR"), PObjFromPtr((void*)A));
         d->members.emplace(("bind"), PObjFromMethod("bind", &socket_Bind, socketKlass));
         d->members.emplace(("connect"), PObjFromMethod("connect", &socket_Connect, socketKlass));
         d->members.emplace(("send"), PObjFromMethod("send", &socket_Send, socketKlass));
         d->members.emplace(("recv"), PObjFromMethod("recv", &socket_Recv, socketKlass));
         d->members.emplace(("listen"), PObjFromMethod("listen", &socket_Listen, socketKlass));
         d->members.emplace(("accept"), PObjFromMethod("accept", &socket_Accept, socketKlass));
         d->members.emplace(("sendto"), PObjFromMethod("sendto", &socket_SendTo, socketKlass));
         d->members.emplace(("recvfrom"), PObjFromMethod("recvfrom", &socket_RecvFrom, socketKlass));
         d->members.emplace(("close"), PObjFromMethod("close", &socket_Close, socketKlass));
         d->members.emplace((".destroy"), PObjFromMethod(".destroy", &socket_Destroy, socketKlass));
         return PObjFromKlassInst(d);
         
     }
     EXPORT PltObject socket_Connect( PltObject* args, int n)
     {
         int e = validateArgTypes("osi", args, n);
         if (e == -1)
         {
             return Plt_Err(VALUE_ERROR, "3 arguments needed!");
             
         }
         if (e!=-2)
         {
             return Plt_Err(TYPE_ERROR, "Argument " + to_string(e) + " is of invalid type.");
             
         }
         KlassInstance* p = (KlassInstance*)args[0].ptr;
         
         string& addr = *(string*)args[1].ptr;
         Socket* s = (Socket*)p->members[".internalPTR"].ptr;
         struct sockaddr_in server;
         server.sin_addr.s_addr = inet_addr(addr.c_str());
         server.sin_family = s->ipfamily;
         server.sin_port = htons(args[2].i);
         if (connect(s->socket_desc, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
         {
             string errMsg = to_string(WSAGetLastError());
             return Plt_Err(UNKNOWN_ERROR, errMsg);
             
         }
         s->ip = addr;
         s->port = args[2].i;
         s->open = true;
         return nil;
     }
     EXPORT PltObject socket_Send(PltObject* args, int n)
     {
         int e = validateArgTypes("oc", args, n);
         if (e == -1)
         {
             return Plt_Err(VALUE_ERROR, "2 arguments needed!");
             
         }
         if (e != -2)
         {
             return Plt_Err(TYPE_ERROR, "Argument " + to_string(e+1) + " is of invalid type.");
             
         }
         KlassInstance* p = (KlassInstance*)args[0].ptr;
         if (p->klass != socketKlass)
         {
             return Plt_Err(TYPE_ERROR, "Error socket object needed!");
             
         }
         
         Socket* s = (Socket*)p->members[".internalPTR"].ptr;
         auto& l = *(vector<uint8_t>*)args[1].ptr;
         if(send(s->socket_desc,(const char*)&l[0], l.size(), 0)==SOCKET_ERROR)
         {
           string errMsg = "Error code " + to_string(WSAGetLastError());
           return Plt_Err(UNKNOWN_ERROR, errMsg);
           
         }
         return nil;
     }
     EXPORT PltObject socket_Recv( PltObject* args, int n)
     {
         int e = validateArgTypes("oi", args, n);
         if (e == -1)
         {
             return Plt_Err(VALUE_ERROR, "1 argument needed!");
             
         }
         if (e!=-2)
         {
             return Plt_Err(TYPE_ERROR, "Argument " + to_string(e) + " is of invalid type.");
             
         }
         KlassInstance* p = (KlassInstance*)args[0].ptr;
         if (p->klass != socketKlass)
         {
             return Plt_Err(TYPE_ERROR, "Error socket object needed!");
             
         }
         
         Socket* s = (Socket*)p->members[".internalPTR"].ptr;
         char* msg= new char[(size_t)args[1].i];
         size_t read = recv(s->socket_desc, msg, args[1].i, 0);
         if (read == SOCKET_ERROR)
         {
             string errMsg = "Error code: " + to_string(WSAGetLastError());
             return Plt_Err(UNKNOWN_ERROR, errMsg);
             
         }
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
         {
             return Plt_Err(ARGUMENT_ERROR, "2 argument needed!");
             
         }
         if (e!=-2)
         {
             return Plt_Err(TYPE_ERROR, "Argument " + to_string(e) + " is of invalid type.");
             
         }
         KlassInstance* p = (KlassInstance*)args[0].ptr;
         if (p->klass != socketKlass)
         {
             return Plt_Err(TYPE_ERROR, "Error socket object needed!");
             
         }
         
         Socket* s = (Socket*)p->members[".internalPTR"].ptr;
         if (s->connMethod != SOCK_DGRAM)
         {
             return Plt_Err(VALUE_ERROR, "Error recvfrom() is valid for udp sockets only!");
             
         }
         char* msg = new char[args[1].i];
         int len;
         int read;
         struct sockaddr_in cliaddr;
         len = sizeof(cliaddr);  //len is value/resuslt
         if ((read = recvfrom(s->socket_desc,(char*)msg,args[1].i, 0, (struct sockaddr*)&cliaddr, &len)) == SOCKET_ERROR)
         {
             string errMsg = to_string(WSAGetLastError());
             return Plt_Err(UNKNOWN_ERROR, errMsg);
             
         }
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
         
        
         KlassInstance* d = vm_allocKlassInstance();
         d->klass = udpResKlass;
        
         d->members.emplace(("addr"), PObjFromStr(IP));
         d->members.emplace(("data"), data);
         delete[] msg;
         return PObjFromKlassInst(d);
     }
     EXPORT PltObject socket_SendTo( PltObject* args, int n)
     {
         int e = validateArgTypes("ocsi", args, n);
         if (e == -1)
             return Plt_Err(VALUE_ERROR, "4 arguments needed!");
         if (e!=-2)
             return Plt_Err(TYPE_ERROR, "Argument " + to_string(e) + " is of invalid type.");
         KlassInstance* p = (KlassInstance*)args[0].ptr;
         if (p->klass != socketKlass)
             return Plt_Err(TYPE_ERROR, "Error socket object needed!");
         
         Socket* s = (Socket*)p->members[".internalPTR"].ptr;
         if (s->connMethod != SOCK_DGRAM)
             return Plt_Err(VALUE_ERROR, "Error sendto() is valid for udp sockets only!");
         auto& l = *(vector<uint8_t>*)args[1].ptr;
         
         int read;
         unsigned int len;
         struct sockaddr_in addr;
         memset(&addr, 0, sizeof(addr));
         // Filling server information
         addr.sin_family = s->connMethod;
         addr.sin_port = htons(args[3].i);
         string& saddr = *(string*)args[2].ptr;
         addr.sin_addr.s_addr = inet_addr(saddr.c_str());
         int iResult = sendto(s->socket_desc,(const char*)&l[0],l.size(), 0, (SOCKADDR*)&addr, sizeof(addr));
         if (iResult == SOCKET_ERROR)
         {
             string msg = to_string(GetLastError());
             return Plt_Err(UNKNOWN_ERROR, msg);
         }
         return nil;
     }
     EXPORT PltObject socket_Close( PltObject* args, int n)
     {
         if (n != 1)
         {
             return Plt_Err(VALUE_ERROR, "1 arguments needed!.");
             
         }
         if (args[0].type != 'o')
         {
             return Plt_Err(TYPE_ERROR, "Error socket object needed!");
             
         }
         KlassInstance* p = (KlassInstance*)args[0].ptr;
         if (p->klass != socketKlass)
         {
             return Plt_Err(TYPE_ERROR, "Error socket object needed!");
             
         }
         
         Socket* s = (Socket*)p->members[".internalPTR"].ptr;
         if (!(s->open))
         {
             return Plt_Err(VALUE_ERROR, "Socket already closed!"); 
             
         }
         closesocket(s->socket_desc);
         s->open = false;
         return nil;
     }