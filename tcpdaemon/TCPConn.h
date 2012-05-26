#ifndef TCPCONN_H
#define TCPCONN_H

#include <string>
#include <queue>
#include <map>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h> //sockaddr_un 

#include "packets/TCPPackets.h"

class BindRequestPacket;
class ConnectRequestPacket;
class BindResponsePacket;
class RecvRequestPacket;
class SendRequestPacket;
class AcceptRequestPacket;
class TCPDaemon;
class TimerService;


//A TCPConn encapsulates all information about a connection between two endpoints
//It contains an socket connected to the client API domain socket
//It contains a socket connected to the remote endpoint


class TCPConn
{

    public:

    //Constructor when a server has binded to a local port
    TCPConn(TCPDaemon& daemon, BindRequestPacket* incomingpacket, sockaddr_un IPCInfo, int ipcSock);

    //Constructor when a client is making an outbound connection
    TCPConn(TCPDaemon& daemon, ConnectRequestPacket* incomingpacket, sockaddr_un IPCInfo, int ipcSock);

    //Get the UDP socket associated with this connection
    int getSocket();

    //Called when the Daemon has received a AcceptRequest
    void Accept(AcceptRequestPacket* packet);

    //Called when the Daemon has received a RecvRequest
    void RecvRequest(RecvRequestPacket* packet);
    //Called when the Daemon has received a SendRequest
    void SendRequest(SendRequestPacket* packet);

    //This is called by TCPDaemon when data arrives
    void ReceiveData();

    //This is called by the TimerService when a timer owned by this TCPConn expires
    //This causes the given packet to be retransmitted and a new timer created
    void ExpireTimer();

    private:

    //ACKs the just received packet as it is the expected one
    void sendACK(TCPPacket::Flags flags = TCPPacket::FLAG_ACK);
    
    enum State{
        STANDBY,
        ACCEPTING,
        SYN_SENT,
        
        RECV,
        SEND,
    };

    //Class static var for creating stream sockets on the server end
    static int socketpool;

    //Reference back to the daemon
    TCPDaemon& theDaemon;

    //Contains info to connect back to the domain socket of the client
    struct sockaddr_un mIPCInfo;

    //The fd to use to communicate back to the client process via ResponsePackets
    int mIPCSock;

    //Used to store the UDP conn info to connect to the remote end
    struct sockaddr_in mRemoteInfo;

    //The actual fd associated with the remote end
    int mUDPSocket;

    //Not currently used fully. Used to create multiple stream sockets from one listening socket
    int mClientSocket;

    //The current sequence number
    uint32_t mSeqNum;

    //The other ends sequence number
    uint32_t mAckNum;

    //The current State of the client(basically which Response packet the client is waiting for )
    State mState;

    //The buffer between IPC data and send/recv'ing over the wire
    std::map<uint32_t, TCPPacket> mRecvBuffer;
    std::queue<TCPPacket*> mSendBuffer;

    

    //The number of bytes the client is requesting
    int mRecvSize;

    std::string anID;
    int mPacketsInFlight;

    int mRTO;
    int mSRTT;
};


#endif //TCPCONN_H

