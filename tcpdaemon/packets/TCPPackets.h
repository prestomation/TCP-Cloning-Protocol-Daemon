#ifndef TCPPACKETS_H
#define TCPPACKETS_H

#ifdef linux
#include <stdint.h>
#endif
#ifdef sun
#include <inttypes.h>
#endif

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
            char payload[1024];

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
