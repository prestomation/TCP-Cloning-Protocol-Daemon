#ifndef TCPCONN_H
#define TCPCONN_H

#include <string>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h> //sockaddr_un 

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

    private:

    
    enum State{
        STANDBY,
        ACCEPTING,
        RECV,
        SEND
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

    //The current State of the client(basically which Response packet the client is waiting for )
    State mState;

    //TODO: This 1024 is hardcoded
    //TODO: Circular buffer
    //TODO: We will probably need a buffer for each substream(multple accept() on a server)
    //The buffer used to transfer data between data from the UDP socket and the IPC RecvResponse
    char mRecvBuf[1024];
    //The number of bytes the client is requesting
    int mRecvSize;
};


#endif //TCPCONN_H

