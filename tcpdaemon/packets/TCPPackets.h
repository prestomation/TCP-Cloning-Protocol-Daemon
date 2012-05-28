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


//TCPPacket encapsulates all information related to a TCPPacket
class TCPPacket
{

    public:

        //The TCP flag ENUM 
        enum Flags
        {
            FLAG_SYN,
            FLAG_SYNACK,
            FLAG_ACK,
            FLAG_FIN,
            FLAG_FINACK
        };


        //A globally static structure of where the Troll can be found
        static struct sockaddr_in trollAddrInfo;

        //Receive contructor
        //This creates a TCPPacket right off data on the wire. After calling this constuctor, the object is 
        //fully instantiatied. The goodChecksum bool is set according to the checksum check
        TCPPacket(int sock);
        
        //Constructor for creating a new outgoing TCPPacket
        TCPPacket(sockaddr_in destination, uint32_t  seqnum, uint32_t acknum,  char * payload, int payloadsize, Flags flags);

        //Constructor for creating a new outgoing ACK TCPPacket that has no payload
        TCPPacket(sockaddr_in destination, uint32_t  seqnum, uint32_t acknum);

        virtual ~TCPPacket();

        //Send this TCPPacket over the wire on the socket sockfd. It is sent to the troll with destination written as the header
        virtual int send(int sockfd);


        //packet is the actual message 
        struct trollmessage{
            struct sockaddr_in header;
            Flags flags;
            uint32_t seqNum;
            uint32_t ackNum;
            uint32_t checksum;
            int payloadsize;
            char payload[1024];

        } packet;

        //Set after a TCPPacket is received according to the checksum
        bool goodChecksum;
        uint32_t packetNumber;

    private:

        enum Direction
        {
            STATE_INCOMING,
            STATE_OUTGOING
        };

        //This currently isn't used
        Direction mState;

        //Contains initialization code shared between the outgoing constructors
        void init(sockaddr_in destination, uint32_t seqnum, uint32_t acknum);

        //Initialize the CRC table. This only needs to be called once
        void initCRCTable();

        //Calculate the CRC checksum for a packet.
        uint32_t crc(uint8_t *payload, unsigned int loadsize);
        static uint32_t crc_table[256];
        //Set to true once the CRC table has been initialized
        static bool initCRC;



};
#endif //TCPPACKETS_H
