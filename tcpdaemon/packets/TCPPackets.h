#ifndef TCPPACKETS_H
#define TCPPACKETS_H

class TCPPacket
{


    public:
        static struct sockaddr_in trollAddrInfo;
        virtual int send(int sockfd, struct sockaddr_un conninfo) = 0;
        virtual int receive(int sockfd) = 0;

    private:
        IPCPacket(char opcode): mOpcode(opcode){}
        struct sockaddr_in header;
        uint32_t mSequenceNum;
        uint32_t mAckNum;
        short mChecksum;
        int payloadsize;
        char payload[1024];


};
#endif //TCPPACKETS_H
