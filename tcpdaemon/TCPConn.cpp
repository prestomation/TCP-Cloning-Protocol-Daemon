#include <iostream>
#include <sys/un.h> //sockaddr_un, 
#include <arpa/inet.h> //inet_ntop

#include "TCPConn.h"
#include "packets/IPCPackets.h"
#include "TCPDaemon.h"
#include "TimerService.h"

using namespace std;

TCPConn::TCPConn(TCPDaemon& daemon, BindRequestPacket* bindrequest, struct sockaddr_un IPCInfo, int ipcSock):
    theDaemon(daemon), mIPCInfo(IPCInfo), mIPCSock(ipcSock)
{


    BindResponsePacket response;

    mRemoteInfo.sin_family = AF_INET;
    mRemoteInfo.sin_addr.s_addr = bindrequest->addr;
    mRemoteInfo.sin_port = ntohs(bindrequest->port);

    response.addr = bindrequest->addr;
    response.port = bindrequest->port;

    /*initialize socket connection for UDP server*/
    if((mUDPSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        response.addr = 0;
        response.port = 0;
    }


    if(bind(mUDPSocket, (struct sockaddr *)&mRemoteInfo, sizeof(struct sockaddr_in)) < 0) {
        response.addr = 0;
        response.port = 0;
    }

    mState = STANDBY;


    response.send(mIPCSock, mIPCInfo);
    delete bindrequest;
}

TCPConn::TCPConn(TCPDaemon& daemon, ConnectRequestPacket* bindrequest, struct sockaddr_un IPCInfo, int ipcSock):
    theDaemon(daemon), mIPCInfo(IPCInfo), mIPCSock(ipcSock)
{

    ConnectResponsePacket response;

    mRemoteInfo.sin_family = AF_INET;
    mRemoteInfo.sin_addr.s_addr = bindrequest->addr;
    mRemoteInfo.sin_port = ntohs(bindrequest->port);

    char ipAddr[INET_ADDRSTRLEN] ;
    inet_ntop(AF_INET, &mRemoteInfo.sin_addr.s_addr, ipAddr, INET_ADDRSTRLEN);
    cout << "Message destined for " <<  ipAddr;

    response.addr = bindrequest->addr;
    response.port = bindrequest->port;

    /*initialize socket connection for UDP server*/
    if((mUDPSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        response.addr = 0;
        response.port = 0;
    }


    //send garbage to get the other end to accept
    theDaemon.sendto(mUDPSocket, mRecvBuf, 0, 0, (struct sockaddr *)&mRemoteInfo, sizeof(mRemoteInfo));

    mState = STANDBY;


    response.send(mIPCSock, mIPCInfo);
    delete bindrequest;
}



int TCPConn::getSocket()
{
    return mUDPSocket;
}


void TCPConn::Accept(AcceptRequestPacket* packet)
{

    mState = ACCEPTING;
    theDaemon.addListeningSocket(mUDPSocket, this);
    delete packet;

}

void TCPConn::ReceiveData()
{
    cout << "ReceivedData" << endl;
    int recvBytes;
    struct sockaddr_in incomingInfo;
    int sockaddr_inLen = sizeof(incomingInfo);

    if(mState == ACCEPTING)
    {
        TCPDaemon::TrollMessage newMessage;

        recvBytes = recvfrom(mUDPSocket, &newMessage, sizeof(newMessage), 0, (struct sockaddr *)&incomingInfo, (socklen_t*) &sockaddr_inLen);

        if (recvBytes < 0 )
        {
            cout << "No bytes received. This really shouldn't happen.." << endl;
        }
        cout << "Received " << recvBytes << " bytes!" << endl;
        mClientSocket = socketpool++;
        cout << "We are accepting a new connection" << endl;
        //Unblock the accepting call at the client. 
        AcceptResponsePacket response;
        response.code = 0;
        response.connID = mClientSocket;
        //response.addr = incomingInfo.sin_addr.s_addr;
        //response.port = ntohs(incomingInfo.sin_port);
        response.addr = newMessage.header.sin_addr.s_addr;
        response.port = ntohs(newMessage.header.sin_port);
        cout << "Sending AcceptResponse" << endl;
        response.send(mIPCSock, mIPCInfo);
        cout << "Going into STANDBY" << endl;
        mState = STANDBY;
    }
    else if (mState == RECV)
    {
        TCPDaemon::TrollMessage newMessage;

        recvBytes = recvfrom(mUDPSocket, &newMessage, sizeof(newMessage), 0, (struct sockaddr *)&incomingInfo, (socklen_t*) &sockaddr_inLen);

        if (recvBytes < 0 )
        {
            cout << "No bytes received. This really shouldn't happen.." << endl;
        }
        RecvResponsePacket response;
        cout << "WE SHOULD HAVE THIS MANY: " << newMessage.bodysize;
        memcpy(&response.data, &newMessage.body, newMessage.bodysize);
        response.size = newMessage.bodysize;
        response.send(mIPCSock, mIPCInfo);
        cout << "Going into STANDBY" << endl;
        mState = STANDBY;
    }
}

void TCPConn::RecvRequest(RecvRequestPacket* packet)
{

    cout << "Going into RECV" << endl;
    mState=RECV;
    mRecvSize = packet->bufsize;
    //packet->sockid //TODO: We're not differentiating between different accepted sockets on the same listen socket
    delete packet;
}

void TCPConn::SendRequest(SendRequestPacket* packet)
{
    cout << "Going into SEND" << endl;

    mState=SEND;
    int bytesSent = theDaemon.sendto(mUDPSocket, &packet->data, packet->size, 0, (struct sockaddr *)&mRemoteInfo, sizeof(mRemoteInfo));
    SendResponsePacket response;
    response.bytesSent = bytesSent;
    response.send(mIPCSock, mIPCInfo);
    delete packet;
}




int TCPConn::socketpool = 100;
