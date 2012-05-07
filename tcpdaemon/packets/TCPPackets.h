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

class Flags;

class TCPPacket
{

    public:
        enum Flags
        {
            FLAG_SYN,
            FLAG_SYNACK,
            FLAG_ACK,
            FLAG_FIN,
            FLAG_FINACK
        };


        static struct sockaddr_in trollAddrInfo;
        TCPPacket(int sock);
        TCPPacket(sockaddr_in destination, uint32_t  seqnum, uint32_t acknum,  char * payload, int payloadsize, Flags flags);
        TCPPacket(sockaddr_in destination, uint32_t  seqnum, uint32_t acknum);
        ~TCPPacket();
        virtual int send(int sockfd);


        struct trollmessage{
            struct sockaddr_in header;
            Flags flags;
            uint32_t seqNum;
            uint32_t ackNum;
            short checksum;
            int payloadsize;
            char payload[1024];

        } packet;

    uint32_t crc_table[256];
    uint32_t polynomial;
    void initCRCTable();
    uint16_t crc(uint8_t *payload, unsigned int loadsize);

    private:
        enum Direction
        {
            STATE_INCOMING,
            STATE_OUTGOING
        };

        Direction mState;
        void init(sockaddr_in destination, uint32_t seqnum, uint32_t acknum);



};
#endif //TCPPACKETS_H
