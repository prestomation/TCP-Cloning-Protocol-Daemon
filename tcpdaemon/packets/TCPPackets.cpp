#include "TCPPackets.h"

TCPPacket::TCPPacket(sockaddr_in destination, uint32_t seqnum, uint32_t acknum, char * payload, int payloadsize)
{
	mHeader = destination;
	mSequenceNum = seqnum;
	mAckNum = acknum;
	mPayloadsize = payloadsize;
	mPayload = payload;
	mState = STATE_OUTGOING;
} 

TCPPacket::TCPPacket(int sock)
{
	mState = STATE_INCOMING;

	//return ::recvfrom(sock, this, sizeof(*this), 0, (struct sockaddr*)&trollAddrInfo, sizeof(trollAddrInfo));
        int recvBytes = ::recvfrom(sock, this, sizeof(*this), 0, 0, 0);
}

int TCPPacket::send(int sock)
{
	return ::sendto(sock, this, sizeof(*this), 0, (struct sockaddr*)&trollAddrInfo, sizeof(trollAddrInfo));
}

int TCPPacket::receive(int sock)
{
	return -1;
}

struct sockaddr_in TCPPacket::trollAddrInfo;
