#include <iostream>
#include <sys/un.h> //sockaddr_un, 
#include <arpa/inet.h> //inet_ntop

#include "TCPConn.h"
#include "packets/IPCPackets.h"
#include "packets/TCPPackets.h"
#include "TCPDaemon.h"

using namespace std;

//BindRequest constructor
TCPConn::TCPConn(TCPDaemon& daemon, BindRequestPacket* bindrequest, struct sockaddr_un IPCInfo, int ipcSock):
    theDaemon(daemon), mIPCInfo(IPCInfo), mIPCSock(ipcSock), mRecvBuffer(), mSendBuffer()
{

    anID = "SERVER";
    mAckNum = 0;
    //TODO: Randomize
    mSeqNum = 0;

    BindResponsePacket response;

    struct sockaddr_in bindInfo;
    bindInfo.sin_family = AF_INET;
    bindInfo.sin_addr.s_addr = bindrequest->addr;
    bindInfo.sin_port = ntohs(bindrequest->port);

    response.addr = bindrequest->addr;
    response.port = bindrequest->port;

    /*initialize socket connection for UDP server*/
    if((mUDPSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        response.addr = 0;
        response.port = 0;
    }


    if(bind(mUDPSocket, (struct sockaddr *)&bindInfo, sizeof(struct sockaddr_in)) < 0) {
        response.addr = 0;
        response.port = 0;
    }

    mState = STANDBY;


    response.send(mIPCSock, mIPCInfo);
    delete bindrequest;
}

//ConnectRequest constructor
TCPConn::TCPConn(TCPDaemon& daemon, ConnectRequestPacket* bindrequest, struct sockaddr_un IPCInfo, int ipcSock):
    theDaemon(daemon), mIPCInfo(IPCInfo), mIPCSock(ipcSock)
{

    anID = "CLIENT";

    mAckNum = 0;
    //TODO: Randomize
    mSeqNum = 0;

    ConnectResponsePacket response;

    mRemoteInfo.sin_family = AF_INET;
    mRemoteInfo.sin_addr.s_addr = bindrequest->addr;
    mRemoteInfo.sin_port = ntohs(bindrequest->port);

    char ipAddr[INET_ADDRSTRLEN] ;
    inet_ntop(AF_INET, &mRemoteInfo.sin_addr.s_addr, ipAddr, INET_ADDRSTRLEN);
    //cout << anID << ": Message destined for " <<  ipAddr;

    response.addr = bindrequest->addr;
    response.port = bindrequest->port;

    /*initialize socket connection for UDP server*/
    if((mUDPSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        response.addr = 0;
        response.port = 0;
    }


    //send a SYN packet, no data
    TCPPacket *outgoingPacket = new TCPPacket(mRemoteInfo, mSeqNum, 0, 0, 0, TCPPacket::FLAG_SYN);
    outgoingPacket->send(mUDPSocket);
    mSendBuffer.push(outgoingPacket);
    theDaemon.addTimer(100, mSeqNum, *this);

    //Listen for packets coming back
    theDaemon.addListeningSocket(mUDPSocket, this);

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

    TCPPacket incomingPacket(mUDPSocket);
    if(incomingPacket.goodChecksum == false)
    {
        cout << anID << ": dropping packet due to bad checksum" << endl;
        return;
    }


    if(mState == ACCEPTING)
    {

        if(incomingPacket.packet.flags != TCPPacket::FLAG_SYN)
        {
            cout << "You haven't even syn'd yet, that's rude" << endl;
            return;
        }
        mClientSocket = socketpool++;
        //cout << anID << ": We are accepting a new connection" << endl;
        //Unblock the accepting call at the client. 
        AcceptResponsePacket response;
        response.code = 0;
        response.connID = mClientSocket;

        char ipAddr[INET_ADDRSTRLEN] ;
        inet_ntop(AF_INET, &incomingPacket.packet.header.sin_addr.s_addr, ipAddr, INET_ADDRSTRLEN);


        response.addr = incomingPacket.packet.header.sin_addr.s_addr;
        response.port = ntohs(incomingPacket.packet.header.sin_port);

        mRemoteInfo = incomingPacket.packet.header;
        cout << anID << " Accepted connection from " << ipAddr << ":" << response.port << endl;

        //cout << "Sending AcceptResponse" << endl;
        response.send(mIPCSock, mIPCInfo);


        mAckNum =incomingPacket.packet.seqNum; 
        sendACK(TCPPacket::FLAG_SYNACK);
        cout << anID << ": Going into STANDBY" << endl;
        mState = STANDBY;
    }
    else if (mState == RECV)
    {


        cout<< anID <<" Received SEQ: " << incomingPacket.packet.seqNum << " we want: " << mAckNum+incomingPacket.packet.payloadsize << endl;
        if(incomingPacket.packet.seqNum != (mAckNum+incomingPacket.packet.payloadsize))
        {

            if(incomingPacket.packet.seqNum < (mAckNum+incomingPacket.packet.payloadsize))
            {
                cout << anID << ": we've had this packet before, resending ACK" <<endl;
                sendACK();

            }
            //This is NOT the expected packet. Did we miss one? ignore it
            cout << anID << " dropping packet!" << endl;
            return;
        }

        RecvResponsePacket response;
        memcpy(&response.data, &incomingPacket.packet.payload, incomingPacket.packet.payloadsize);
        response.size = incomingPacket.packet.payloadsize;
        response.send(mIPCSock, mIPCInfo);

        //Increase the mAckNum according to the incoming payload
        cout << anID << ": increasing ACK by " << incomingPacket.packet.payloadsize << endl;
        mAckNum += incomingPacket.packet.payloadsize;
        sendACK();

        cout << "Going into STANDBY" << endl;
        mState = STANDBY;
    }
    else{
        //Else this is an ACK
        if(anID == "SERVER")
        {
            cout << "THIS SHOULDNT HAPPEN. The server received an unknown packet" << endl;
            sendACK();
            return;
        }

        cout << anID <<" Received ACK: " << incomingPacket.packet.ackNum << " Sequence num is: " << mSeqNum << endl;
        if(incomingPacket.packet.ackNum > mSeqNum)
        {

            cout << anID << " dropping packet" << endl;
            return;
        }
        else{
            cout << anID << ": ACK" << endl;
            theDaemon.removeTimer(incomingPacket.packet.ackNum, *this);
            cout << "BUFFER HAS " << mSendBuffer.size() << endl;

            while(!mSendBuffer.empty())
            {
                TCPPacket *old = mSendBuffer.front();
                if (old->packet.seqNum <= incomingPacket.packet.ackNum)
                {
                    //Remove everything in the buffer that has been ack'ed 
                    cout << anID << " Deleting Buffer element with SEQ "<< old->packet.seqNum << " as it has been ACK'd" << endl;
                    mSendBuffer.pop();
                    delete old;
                }
                else
                {
                    cout << anID <<" Keeping next with SEQ: "<< old->packet.seqNum << endl;
                    break;
                }
            }
        }
    }
}

void TCPConn::RecvRequest(RecvRequestPacket* packet)
{

    cout << anID << " Going into RECV" << endl;
    mState=RECV;
    //TODO: we're currently not caring how much data the client is asking for 
    //packet->sockid //TODO: We're not differentiating between different accepted sockets on the same listen socket
    delete packet;
}

void TCPConn::SendRequest(SendRequestPacket* packet)
{
    cout << anID << " Going into SEND: " << endl;

    mState=SEND;

    mSeqNum += packet->size;
    BufferPair outgoingBuf; //TODO :remove?
    memset( &outgoingBuf.second, 0, sizeof (outgoingBuf.second ));  
    outgoingBuf.first = packet->size;
    memcpy(outgoingBuf.second, packet->data, packet->size);
    TCPPacket *outgoingPacket = new TCPPacket(mRemoteInfo, mSeqNum, mAckNum, packet->data, packet->size, TCPPacket::FLAG_ACK);
    mSendBuffer.push(outgoingPacket);
    int bytesSent = outgoingPacket->send(mUDPSocket);
    SendResponsePacket response;
    response.bytesSent = bytesSent;
    response.send(mIPCSock, mIPCInfo);

    theDaemon.addTimer(500, mSeqNum, *this);

    //TODO: for some reason this is seg faulting ? something to do with introducing the buffer queue
    //delete packet;
}


void TCPConn::ExpireTimer()
{
    cout << anID << ": Expired packet " << ", resending front of the buffer.." << endl;
    if(mSendBuffer.empty())
    {

        return;
    }
    TCPPacket *outgoingPacket  = mSendBuffer.front();
    if(outgoingPacket == NULL)
    {
        cout << "Buffer is empty" << endl;
        return;
    }
    outgoingPacket->send(mUDPSocket);
    cout << "EXPIRED PACKET HAS SEQ: " << outgoingPacket->packet.seqNum << " AND ACK: " << outgoingPacket->packet.ackNum << endl;

    theDaemon.addTimer(100, outgoingPacket->packet.seqNum, *this);

}




void TCPConn::sendACK(TCPPacket::Flags flags )
{

    TCPPacket outgoingPacket(mRemoteInfo, mSeqNum , mAckNum, 0, 0, flags);
    cout << anID << " Sending ACK " << mAckNum << " for SEQ " << mSeqNum << endl;
    outgoingPacket.send(mUDPSocket);
    //TODO: put this on a timer
    return;
}

int TCPConn::socketpool = 100;
