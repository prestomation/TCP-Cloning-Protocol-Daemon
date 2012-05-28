#include "api.h"
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <iostream>

#include "packets/IPCPackets.h"
#include <map>

//TODO: we are duplicating memory with the values
//A map of binded sockets to an info object to communicate with 
std::map<int, struct sockaddr_un> ipcConnMap;

//accepted socket to ipcSocket map. Lookup value in ipcConnMap to get back to the daemon
std::map<int, int> acceptedConnMap;

char srcPath[25];




//Given the IPC socket where a packet is anticipated to arrive, receive the opcode and return it
char recvOpcode(int sockfd)
{
    char opcode;

    cout << "recving opcode.." << endl;
    //recv opcode
    if(recv(sockfd, &opcode, sizeof(opcode), 0) < 0 )
    {
        cout << "failed to recv opcode" << endl;
        return -1;
    }
    std::cout <<  "Received opcode: " << opcode << endl;
    return opcode;
}


//Create new socket
int SOCKET(int socket_family, int socket_type, int protocol)
{
    cout << "SOCKET" << endl;

    //We are actually creating a socket to connect with the local daemon over Unix domain sockets
    int asocket = socket(AF_UNIX, SOCK_DGRAM, 0);
    sprintf(srcPath, "%s%d-%d", DAEMON_SOCKET, getpid(), asocket);
    return asocket;
}

int BIND(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    //Bind to an address. This can be used to bind to a local port or a remote port, just like the native call

    cout << "BIND" << endl;
    struct sockaddr_un ipcSrcInfo;
    ipcSrcInfo.sun_family = AF_UNIX;
    strcpy(ipcSrcInfo.sun_path, srcPath);


    struct sockaddr_un ipcInfo;
    ipcInfo.sun_family = AF_UNIX;
    strcpy(ipcInfo.sun_path, DAEMON_SOCKET);

    ipcConnMap[sockfd] = ipcInfo;


    //bind to the src domain socket, this is required to recv return messages from the daemon
    if(bind(sockfd, (struct sockaddr *) &ipcSrcInfo, sizeof(ipcSrcInfo)) < 0 )
    {
        cout << "bind failed to domain socket" << endl;
        return -1;
    }

    //Construct the bind packet and send it 
    BindRequestPacket packet;
    packet.addr = ((struct sockaddr_in *)addr)->sin_addr.s_addr;
    packet.port = ntohs(((struct sockaddr_in *)addr)->sin_port);
    packet.send(sockfd, ipcConnMap[sockfd]);
    char opcode;
    //Block, waiting for a response...
    if((opcode =recvOpcode(sockfd) )!= OPCODE_BIND_RESPONSE)
    {
        cout << "Unexcepted Opcode:" << (char)(opcode + 49) << endl;

        return -1;
    }

    BindResponsePacket responsePacket;
    responsePacket.receive(sockfd);
    if(packet.addr != responsePacket.addr || packet.port != responsePacket.port)
    {
        //Response packet is not equal to request. An error has occured
        return -1;
    }

    return 0;
}

int LISTEN(int socketfd, int backlog)
{
    //no-op due to stream emulation at the application layer
    cout << "LISTEN" << endl;
    return 0;
}

int ACCEPT(int sockfd, const struct sockaddr *addr, socklen_t* addrlen){

    cout << "ACCEPT" << endl;
    if(ipcConnMap.count(sockfd) == 0)
    {
        struct sockaddr_un ipcInfo;
        ipcInfo.sun_family = AF_UNIX;
        strcpy(ipcInfo.sun_path, DAEMON_SOCKET);

        ipcConnMap[sockfd] = ipcInfo;

        struct sockaddr_un ipcSrcInfo;
        ipcSrcInfo.sun_family = AF_UNIX;
        strcpy(ipcSrcInfo.sun_path, srcPath);


        //bind to the src domain socket
        if(bind(sockfd, (struct sockaddr *) &ipcSrcInfo, sizeof(ipcSrcInfo)) < 0 )
        {
            cout << "bind failed to domain socket" << endl;
            return -1;
        }


    }



    AcceptRequestPacket request;
    request.send(sockfd, ipcConnMap[sockfd]);
    AcceptResponsePacket response;
    char opcode;
    //block, waiting for a response..
    if((opcode =recvOpcode(sockfd) )!= OPCODE_ACCEPT_RESPONSE)
    {
        cout << "Unexcepted Opcode:" << (char)(opcode + 49) << endl;

        return -1;
    }


    response.receive(sockfd);
    int newsock = response.connID;
    if(addr != NULL)
    {
        ((struct sockaddr_in*)addr)->sin_family = AF_INET;
        ((struct sockaddr_in*)addr)->sin_addr.s_addr = response.addr;
        ((struct sockaddr_in*)addr)->sin_port= htons(response.port);

    }
    acceptedConnMap[newsock] = sockfd;

    //TODO: fill out addr struct
    return newsock;

}
int CONNECT(int sockfd, const struct sockaddr *addr, socklen_t addrlen){
    //TODO this is almost identical to bind, refactor to minimize duplication
    cout << "CONNECT" << endl;

    struct sockaddr_un ipcSrcInfo;
    ipcSrcInfo.sun_family = AF_UNIX;
    strcpy(ipcSrcInfo.sun_path, srcPath);


    struct sockaddr_un ipcInfo;
    ipcInfo.sun_family = AF_UNIX;
    strcpy(ipcInfo.sun_path, DAEMON_SOCKET);

    ipcConnMap[sockfd] = ipcInfo;


    //bind to the src domain socket
    if(bind(sockfd, (struct sockaddr *) &ipcSrcInfo, sizeof(ipcSrcInfo)) < 0 )
    {
        cout << "bind failed to domain socket" << endl;
        return -1;
    }



    //connect to the daemon over Unix sockets 
    /*if(connect(sockfd, (struct sockaddr *)&ipcConnMap[sockfd], sizeof(ipcConnMap[sockfd])) < 0)
    {
        cout << "Connection failed to domain socket" << endl;
        return -1;
    }*/

    //Construct the Connect packet and send it 
    ConnectRequestPacket packet;
    packet.addr = ((struct sockaddr_in *)addr)->sin_addr.s_addr;
    packet.port = ntohs(((struct sockaddr_in *)addr)->sin_port);
    packet.send(sockfd, ipcConnMap[sockfd]);
    char opcode;
    //block, waiting for a response..
    if((opcode =recvOpcode(sockfd) )!= OPCODE_CONNECT_RESPONSE)
    {
        cout << "Unexcepted Opcode:" << (char)(opcode + 49) << endl;

        return -1;
    }

    ConnectResponsePacket responsePacket;
    responsePacket.receive(sockfd);
    if(packet.addr != responsePacket.addr || packet.port != responsePacket.port)
    {
        //Response packet is not equal to request. An error has occured
        return -1;
    }

    return 0;
	
}
ssize_t SEND(int sockfd, const void *buf, size_t len, int flags){

    cout << "SEND" << endl;
    SendRequestPacket request;
    int ipcsock;

    //If this is an accepted socket, find what the actual socket is
    if (acceptedConnMap.count(sockfd) == 1)
    {
        //This is an accepted connection
        ipcsock = acceptedConnMap[sockfd] ;
    }
    else
    {
        //This is a direct connection
        ipcsock = sockfd; 
    }

    request.sockid = sockfd;
    request.size = len;
    memcpy(&request.data, buf, len); 
    request.send(ipcsock, ipcConnMap[ipcsock]);
    
    SendResponsePacket response;
    char opcode;
    //block, waiting for a response..
    if((opcode =recvOpcode(ipcsock) )!= OPCODE_SEND_RESPONSE)
    {
        cout << "Unexcepted Opcode:" << (char)(opcode + 49) << endl;

        return -1;
    }
    response.receive(ipcsock);


    cout << "Got data!" << endl;
    return response.bytesSent;


}
int RECV(int sockfd, void *buf, size_t len, int flags){
    cout << "RECV" << endl;
    RecvRequestPacket request;

    request.sockid = sockfd;
    request.bufsize = len;
    int ipcsock;

    //If this is an accepted socket, find what the actual socket is
    if (acceptedConnMap.count(sockfd) == 1)
    {
        //This is an accepted connection
        ipcsock = acceptedConnMap[sockfd] ;
    }
    else
    {
        //This is a direct connection
        ipcsock = sockfd; 
    }

    request.send(ipcsock, ipcConnMap[ipcsock]);

    RecvResponsePacket response;

    char opcode;
    //block, waiting for a response..
    if((opcode =recvOpcode(ipcsock) )!= OPCODE_RECV_RESPONSE)
    {
        cout << "Unexcepted Opcode:" << (char)(opcode + 49) << endl;

        return -1;
    }
    response.receive(ipcsock);

    memcpy(buf, &response.data, response.size);

    cout << "Got data!" << endl;
    return response.size;

}

int CLOSE(int sockfd){

    cout << "CLOSE" << endl;
    CloseRequestPacket request;

    request.sockid = sockfd;
    int ipcsock;

    //If this is an accepted socket, find what the actual socket is
    if (acceptedConnMap.count(sockfd) == 1)
    {
        //This is an accepted connection
        ipcsock = acceptedConnMap[sockfd] ;
    }
    else
    {
        //This is a direct connection
        ipcsock = sockfd; 
    }

    request.send(ipcsock, ipcConnMap[ipcsock]);

    CloseResponsePacket response;

    char opcode;
    //block, waiting for a response..
    if((opcode =recvOpcode(ipcsock) )!= OPCODE_CLOSE_RESPONSE)
    {
        cout << "Unexcepted Opcode:" << (char)(opcode + 49) << endl;

        return -1;
    }
    response.receive(ipcsock);

    return response.status;
}






