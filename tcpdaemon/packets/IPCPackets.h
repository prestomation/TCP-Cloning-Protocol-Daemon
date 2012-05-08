
#ifndef IPCPACKETS_H
#define IPCPACKETS_H


#include <sys/socket.h>
#include <netinet/in.h>
class IPCPacket
{

    public:
        virtual int send(int sockfd, struct sockaddr_un conninfo) = 0;
        virtual int receive(int sockfd) = 0;
        IPCPacket(char opcode): mOpcode(opcode){}
        virtual ~IPCPacket(){}
        
        
        char getOpcode();
    protected:
        char mOpcode;
};

class BindRequestPacket : public virtual IPCPacket
{
    public:
        int send(int sockfd, struct sockaddr_un conninfo);
        int receive(int sockfd);
        BindRequestPacket();

        //Address and port to bind to on other end
        in_addr_t addr;
        in_port_t port;
};

//The BindResponsePacket is identical to Request, but with a different opcode
class BindResponsePacket : public virtual BindRequestPacket{
    public:
        BindResponsePacket();
};

class ConnectRequestPacket : public virtual IPCPacket
{
    public:
        int send(int sockfd, struct sockaddr_un conninfo);
        int receive(int sockfd);
        ConnectRequestPacket();

        //Address and port to bind to on other end
        in_addr_t addr;
        in_port_t port;
};

//The ConnectResponsePacket is identical to Request, but with a different opcode
class ConnectResponsePacket : public virtual ConnectRequestPacket{
    public:
        ConnectResponsePacket();
};





class AcceptRequestPacket : public virtual IPCPacket
{
    public:
        AcceptRequestPacket();
        int send(int sockfd, struct sockaddr_un conninfo);
        int receive(int sockfd);

};
class AcceptResponsePacket : public virtual IPCPacket
{
    public: 
        int send(int sockfd, struct sockaddr_un conninfo);
        int receive(int sockfd);
        AcceptResponsePacket();

        //an error code
        int code;
        //Unique ID for connected sockets
        int connID;
        //Information about connected host
        in_addr_t addr;
        in_port_t port;

};

class RecvRequestPacket : public virtual IPCPacket
{
    public:
        RecvRequestPacket();
        int send(int sockfd, struct sockaddr_un conninfo);
        int receive(int sockfd);

        int sockid;
        int bufsize;

};
class RecvResponsePacket : public virtual IPCPacket
{
    public: 
        int send(int sockfd, struct sockaddr_un conninfo);
        int receive(int sockfd);
        RecvResponsePacket();

        int size;

        //TODO: We've just capped this max size..fix it
        char data[1024];

};

class SendRequestPacket : public virtual IPCPacket
{
    public:
        SendRequestPacket();
        int send(int sockfd, struct sockaddr_un conninfo);
        int receive(int sockfd);

        int sockid;
        int size;
        //TODO: We've just capped this max size..fix it
        char data[1024];

};
class SendResponsePacket : public virtual IPCPacket
{
    public: 
        int send(int sockfd, struct sockaddr_un conninfo);
        int receive(int sockfd);
        SendResponsePacket();
        int bytesSent;
};


#endif //IPCPACKETS_H
