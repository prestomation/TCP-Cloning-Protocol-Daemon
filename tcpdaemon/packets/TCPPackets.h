#ifndef TCPPACKETS_H
#define TCPPACKETS_H
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>

class TCPPacket
{


    public:
        static struct sockaddr_in trollAddrInfo;
        TCPPacket(int sock);
        TCPPacket(sockaddr_in destination, uint32_t  seqnum, uint32_t acknum,  char * payload, int payloadsize);
        virtual int send(int sockfd);


        struct trollmessage{
            struct sockaddr_in header;
            uint32_t seqNum;
            uint32_t ackNum;
            short checksum;
            int payloadsize;
            char *payload;

        } packet;

    private:
        enum Direction
        {
            STATE_INCOMING,
            STATE_OUTGOING
        };
        Direction mState;



};
#endif //TCPPACKETS_H
