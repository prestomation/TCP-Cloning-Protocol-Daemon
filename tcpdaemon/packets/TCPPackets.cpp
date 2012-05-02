#include "TCPPackets.h"
#include <string.h> //memcpy

TCPPacket::TCPPacket(sockaddr_in destination, uint32_t seqnum, uint32_t acknum, char * payload, int payloadsize)
{
packet.header = destination;
	packet.seqNum = seqnum;
	packet.ackNum = acknum;
	packet.payloadsize = payloadsize;
    memcpy(payload, &packet.payload, payloadsize); 
	mState = STATE_OUTGOING;
} 

TCPPacket::TCPPacket(int sock)
{
    mState = STATE_INCOMING;
    ::recvfrom(sock, &packet, sizeof(packet), 0, 0, 0);

/*
    ::recvfrom(sock, &header, sizeof(header), 0, 0, 0);
    ::recvfrom(sock, &seqNum, sizeof(seqNum), 0, 0, 0);
    ::recvfrom(sock, &ackNum, sizeof(ackNum), 0, 0, 0);
    ::recvfrom(sock, &checksum, sizeof(checksum), 0, 0, 0);
    ::recvfrom(sock, &payloadsize, sizeof(payloadsize), 0, 0, 0);
    ::recvfrom(sock, &payload, payloadsize, 0, 0, 0);*/
}

int TCPPacket::send(int sock)
{
    int bytesSent = 0;
/*
    bytesSent += ::sendto(sock, &header, sizeof(header), 0, (struct sockaddr*)&trollAddrInfo, sizeof(trollAddrInfo));
    bytesSent += ::sendto(sock, &seqNum, sizeof(seqNum), 0, (struct sockaddr*)&trollAddrInfo, sizeof(trollAddrInfo));
    bytesSent += ::sendto(sock, &ackNum, sizeof(ackNum), 0, (struct sockaddr*)&trollAddrInfo, sizeof(trollAddrInfo));
    bytesSent += ::sendto(sock, &checksum, sizeof(checksum), 0, (struct sockaddr*)&trollAddrInfo, sizeof(trollAddrInfo));
    bytesSent += ::sendto(sock, &payloadsize, sizeof(payloadsize), 0, (struct sockaddr*)&trollAddrInfo, sizeof(trollAddrInfo));
    bytesSent += ::sendto(sock, &payload, payloadsize, 0, (struct sockaddr*)&trollAddrInfo, sizeof(trollAddrInfo));
*/
    bytesSent += ::sendto(sock, &packet, sizeof(packet), 0, (struct sockaddr*)&trollAddrInfo, sizeof(trollAddrInfo));
    return bytesSent;
}

struct sockaddr_in TCPPacket::trollAddrInfo;
