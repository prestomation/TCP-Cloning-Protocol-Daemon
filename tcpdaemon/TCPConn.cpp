#include <iostream>
#include <sys/time.h>
#include <stack>
#include <sys/un.h> //sockaddr_un, 
#include <arpa/inet.h> //inet_ntop

#include "TCPConn.h"
#include "packets/IPCPackets.h"
#include "packets/TCPPackets.h"
#include "TCPDaemon.h"

#define WIN_SIZE 20
#define RTO_U 4
#define SMOOTHING_FACTOR 0.9

using namespace std;

//BindRequest constructor
TCPConn::TCPConn(TCPDaemon& daemon, BindRequestPacket* bindrequest, struct sockaddr_un IPCInfo, int ipcSock):
    theDaemon(daemon), mIPCInfo(IPCInfo), mIPCSock(ipcSock), mRecvBuffer(), mSendBuffer(), mSRTT(1000)
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
    theDaemon(daemon), mIPCInfo(IPCInfo), mIPCSock(ipcSock), mSRTT(1000)
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
    mSendBuffer.push_back(outgoingPacket);
    theDaemon.addTimer(mSRTT/1000.0, mSeqNum, *this);

    //Listen for packets coming back
    theDaemon.addListeningSocket(mUDPSocket, this);

    mState = SYN_SENT;
    response.send(mIPCSock, mIPCInfo);
    delete bindrequest;
}



int TCPConn::getSocket()
{
    return mUDPSocket;
}


void TCPConn::Accept(AcceptRequestPacket* packet)
{
    //The client has indicated that he wantes to accept a new connection
    //Tell the Daemon to start listening to this socket and change state    
    mState = ACCEPTING;
    theDaemon.addListeningSocket(mUDPSocket, this);
    delete packet;

}

void TCPConn::ReceiveData()
{

    //Pull the packet off the wire
    TCPPacket incomingPacket(mUDPSocket);
    if(incomingPacket.goodChecksum == false)
    {
        cout << anID << ": dropping packet due to bad checksum" << endl;
        return;
    }


    //TODO: don't differentiate like this. client and server should act the same
    if (anID == "SERVER")
    {

        if(mState == ACCEPTING)
        {

            if(incomingPacket.packet.flags != TCPPacket::FLAG_SYN)
            {
                //We are waiting for a new connection(SYN) but we seemed to have missed it. Drop it and wait for a SYN
                cout << anID << ": is ACCEPTING, but this is not a SYN. Drop it.." << endl;
                return;
            }
            mClientSocket = socketpool++;

            AcceptResponsePacket response;
            response.code = 0;
            response.connID = mClientSocket;

            char ipAddr[INET_ADDRSTRLEN] ;
            inet_ntop(AF_INET, &incomingPacket.packet.header.sin_addr.s_addr, ipAddr, INET_ADDRSTRLEN);


            response.addr = incomingPacket.packet.header.sin_addr.s_addr;
            response.port = ntohs(incomingPacket.packet.header.sin_port);

            mRemoteInfo = incomingPacket.packet.header;
            cout << anID << " Accepted connection from " << ipAddr << ":" << response.port << endl;

            //Unblock the accepting call at the client. 
            response.send(mIPCSock, mIPCInfo);

            mAckNum =incomingPacket.packet.seqNum; 
            sendACK(TCPPacket::FLAG_SYNACK);
            cout << anID << ": Going into STANDBY" << endl;
            mState = STANDBY;
        }
        else 
        {
            if(incomingPacket.packet.flags == TCPPacket::FLAG_SYN)
            {
                //This is a duplication of the SYN packet, drop it
                cout << anID << ": ReACKing SYN duplicate" << endl;
                sendACK(TCPPacket::FLAG_SYNACK);
                return;
            }   


            cout<< anID <<" Received SEQ: " << incomingPacket.packet.seqNum << " we might want: " << mAckNum+incomingPacket.packet.payloadsize << endl;
            if(incomingPacket.packet.seqNum < (mAckNum+incomingPacket.packet.payloadsize))
            {
                //This is a duplicate, just drop it about it, but send an ACK
                cout << anID << ": we've had this packet before, resending ACK" <<endl;
                sendACK();
                if(mRecvBuffer.size() > 0)
                    return;
            }



            if(incomingPacket.packet.seqNum == (mAckNum + incomingPacket.packet.payloadsize))
            {
                //This is the same packet, increase our ACK
                cout << anID << ": increasing ACK by " << incomingPacket.packet.payloadsize << endl;
                mAckNum += incomingPacket.packet.payloadsize;

                if(mState == RECV)
                {
                    //If this is the next packet we want, and the client is blocked, then send him some data!

                    cout << anID << ":We shouldn't have anything in our recv buffer, but it has this many: " << mRecvBuffer.size() << endl;

                    cout << anID << ": Our server is currently blocked, send him data now" << endl;
                    RecvResponsePacket response;
                    memcpy(&response.data, &incomingPacket.packet.payload, incomingPacket.packet.payloadsize);
                    response.size = incomingPacket.packet.payloadsize;
                    response.send(mIPCSock, mIPCInfo);
                    mState = STANDBY;
                }
                else
                {
                    //Client isn't waiting for data, so store it
                    mRecvBuffer.insert(make_pair(incomingPacket.packet.seqNum,  incomingPacket));
                }
            }
            sendACK();

        }
    }
    else{
        //Else this is an ACK
        if(anID == "SERVER")
        {
            cout << "THIS SHOULDNT HAPPEN. The server received an unknown packet" << endl;
            sendACK();
            return;
        }

        cout << anID <<" Received ACK: " << incomingPacket.packet.ackNum << " our seqnum is: " << mSeqNum << endl;

        if (incomingPacket.packet.flags == TCPPacket::FLAG_SYNACK && mState == SYN_SENT)
        {
            //Special case of our SYNACK
            cout << anID << ": SYN Handshake complete!" << endl;
            mState = STANDBY;
        }

        if(incomingPacket.packet.ackNum > mSeqNum)
        {

            cout << anID << " dropping packet" << endl;
            return;
        }
        else
        {
            cout << anID << ": ACK" << endl;
            theDaemon.removeTimer(incomingPacket.packet.ackNum, *this);
            cout << "BUFFER HAS " << mSendBuffer.size() << endl;

            //SRTT calculation. 
            //SRTT is only updated if it's the first time an ack is encountered
            
            timeval now;
            gettimeofday(&now, 0);
            uint32_t timeDiff = (now.tv_usec - mLastTickTimeUSec);
            cout << anID << ": We received at at " << now.tv_usec/1000<< " for a RTT of: " << timeDiff << endl;
            mSRTT = SMOOTHING_FACTOR * mSRTT  + (1.0-SMOOTHING_FACTOR) * timeDiff;
            cout << anID << ": SRTT calculated as " << mSRTT << endl;

            while(!mSendBuffer.empty())
            {
                TCPPacket *old = mSendBuffer.front();
                if (old->packet.seqNum <= incomingPacket.packet.ackNum)
                {
                    //Remove everything in the buffer that has been ack'ed 
                    cout << anID << " Deleting Buffer element with SEQ "<< old->packet.seqNum << " as it has been ACK'd" << endl;
                    mSendBuffer.pop_front();
                    delete old;
                    mCurrentPacketNum++;
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
    if(mRecvBuffer.size() > 0)
    {
        //There's data in the buffer, send it back immediately if it's the next packet
        cout << anID << " data is available" << endl;


        map<uint32_t, TCPPacket>::iterator iter = mRecvBuffer.begin();
        TCPPacket nextPacket = iter->second;
        if(nextPacket.packet.seqNum <= mAckNum)
        {

            cout << anID << "..and that data is next. send RecvResponse " << endl;
            cout << anID << " next seqNum: " << nextPacket.packet.seqNum << " our ack: " << mAckNum << endl;
            //This is the next packet, send data to the client
            RecvResponsePacket response;
            memcpy(&response.data, &nextPacket.packet.payload, nextPacket.packet.payloadsize);
            response.size = nextPacket.packet.payloadsize;
            response.send(mIPCSock, mIPCInfo);
            mRecvBuffer.erase(iter);
            mState=STANDBY;
        }
        else
        {

            cout << anID << "..and that data is NOT next, block the client.. " << endl;
            cout << anID << " next seqNum: " << nextPacket.packet.seqNum << " our ack: " << mAckNum << endl;
        }

    }
    else
    {
        cout << anID << ": no data available, block" << endl;
    }
    delete packet;
}

void TCPConn::SendRequest(SendRequestPacket* packet)
{
    cout << anID << " Going into SEND: " << endl;

    //mState=SEND;

    mSeqNum += packet->size;
    TCPPacket *outgoingPacket = new TCPPacket(mRemoteInfo, mSeqNum, mAckNum, packet->data, packet->size, TCPPacket::FLAG_ACK);
    outgoingPacket->packetNumber = ++mLastPacketNum;
    mSendBuffer.push_back(outgoingPacket);
    //int bytesSent = outgoingPacket->send(mUDPSocket);
    int bytesSent = packet->size;
    SendResponsePacket response; 
    response.bytesSent = bytesSent;
    response.send(mIPCSock, mIPCInfo);

    theDaemon.addTimer(1.0, mSeqNum, *this);

    delete packet;
}


//This is where we actually put data packets on the wire
void TCPConn::ExpireTimer()
{



    TCPPacket* outgoingPacket = mSendBuffer.front();
    int RTO = mSRTT * 2;
    cout << anID << " RTO: " << RTO / 1000.0 << endl;

    //We always reset the timer for our SRTT
    theDaemon.addTimer(RTO/1000.0, outgoingPacket->packet.seqNum, *this);

    stack<TCPPacket*> tmpStack;

    cout << anID << ": timer expired, send our window" << endl;
    while(!mSendBuffer.empty() && mSendBuffer.front()->packetNumber < mCurrentPacketNum + 20)
    {

        //Room is available in the window, send some packets
        //Pop our next packet and put it on our temp stack
        tmpStack.push(mSendBuffer.front());
        mSendBuffer.pop_front();

        if (mState == SYN_SENT){
            //Special case for while we're wating for SYNACK, don't send data packets

            cout << anID << ": still waiting on SYNACK" << endl;
            if (tmpStack.top()->packet.seqNum != 0)
            {
                cout << anID << ": Expired packet is not our SYN, so we're not going to send it until the conn is established" << endl;
                break;
            }
        }

        cout << anID << ": sending a packet" << endl;
        tmpStack.top()->send(mUDPSocket);
    }
    //Ok, now our whole window is in flight, put these packets back in our buffer so they can be ACK'd or resent
    while(!tmpStack.empty())
    {
        mSendBuffer.push_front(tmpStack.top());
        tmpStack.pop();
    }
    timeval now;
    gettimeofday(&now, 0);
    mLastTickTimeUSec =  now.tv_usec;
    cout << anID << ": We sent window at " << mLastTickTimeUSec / 1000 << endl;

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
