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
        virtual int receive(int sockfd);


    private:
        struct sockaddr_in mHeader;
        uint32_t mSequenceNum;
        uint32_t mAckNum;
        short mChecksum;
        int mPayloadsize;
	char *mPayload;
	enum Direction
	{
		STATE_INCOMING,
		STATE_OUTGOING
	};
	Direction mState;


};
#endif //TCPPACKETS_H
