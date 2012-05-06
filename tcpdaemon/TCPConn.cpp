#include <iostream>
#include <sys/un.h> //sockaddr_un, 
#include <arpa/inet.h> //inet_ntop

#include "TCPConn.h"
#include "packets/IPCPackets.h"
#include "packets/TCPPackets.h"
#include "TCPDaemon.h"

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

    //TODO: Generate random sequence number

    //send garbage to get the other end to accept
    TCPPacket outgoingPacket(mRemoteInfo, mSeqNum, 0, mRecvBuf, 0, TCPPacket::FLAG_SYN);
    mSeqNum += outgoingPacket.send(mUDPSocket);

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

    if(mState == ACCEPTING)
    {

        TCPPacket incomingPacket(mUDPSocket);

        mClientSocket = socketpool++;
        cout << "We are accepting a new connection" << endl;
        //Unblock the accepting call at the client. 
        AcceptResponsePacket response;
        response.code = 0;
        response.connID = mClientSocket;
        response.addr = incomingPacket.packet.header.sin_addr.s_addr;
        response.port = ntohs(incomingPacket.packet.header.sin_port);
        cout << "Sending AcceptResponse" << endl;
        response.send(mIPCSock, mIPCInfo);

        //This is the initial sequence number as defined by the initiating remote end  + received data(which is 0 in this case)
        mSeqNum = incomingPacket.packet.seqNum + incomingPacket.packet.payloadsize;

        cout << "Going into STANDBY" << endl;
        mState = STANDBY;
    }
    else if (mState == RECV)
    {
        TCPPacket incomingPacket(mUDPSocket);

        if(incomingPacket.packet.seqNum != mSeqNum)
        {
            //This is not the seqnum we were expecting...
            //Drop it.
            //cout << "WE DROPPED A PACKET" << endl;
            //return;
        }
        //We've received the next packet, increase our sequence number
        //and ACK the packet
        mSeqNum += incomingPacket.packet.payloadsize; 
        sendACK();
        RecvResponsePacket response;

        memcpy(&response.data, &incomingPacket.packet.payload, incomingPacket.packet.payloadsize);
        response.size = incomingPacket.packet.payloadsize;
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
    TCPPacket outgoingPacket(mRemoteInfo, 0, 0, packet->data, packet->size, TCPPacket::FLAG_ACK);
    int bytesSent = outgoingPacket.send(mUDPSocket);
    SendResponsePacket response;
    response.bytesSent = bytesSent;
    response.send(mIPCSock, mIPCInfo);
    delete packet;
}

void TCPConn::sendACK()
{

    TCPPacket outgoingPacket(mRemoteInfo, 0, mSeqNum, 0, 0, TCPPacket::FLAG_ACK);
    return;
}

int TCPConn::socketpool = 100;
