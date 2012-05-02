#include "TCPPackets.h"
#include <string.h> //memcpy
#include <iostream>
#include <arpa/inet.h>


TCPPacket::TCPPacket(sockaddr_in destination, uint32_t seqnum, uint32_t acknum, char * payload, int payloadsize)
{

	packet.header.sin_family = htons(AF_INET);
	packet.header.sin_addr = destination.sin_addr;
	packet.header.sin_port  = destination.sin_port;

    packet.seqNum = seqnum;
    packet.ackNum = acknum;
    packet.payloadsize = payloadsize;
    memcpy(packet.payload, payload, payloadsize); 
    mState = STATE_OUTGOING;
} 

TCPPacket::TCPPacket(int sock)
{
    mState = STATE_INCOMING;
    ::recvfrom(sock, &packet, sizeof(packet), 0, 0, 0);
}

int TCPPacket::send(int sock)
{
    int bytesSent = 0;

    char * destinet;
    destinet = inet_ntoa(packet.header.sin_addr);
    std::cout << "Sending TCP Packet destined for "<< destinet << std::endl;

    bytesSent += ::sendto(sock, &packet, sizeof(packet), 0, (struct sockaddr*)&trollAddrInfo, sizeof(trollAddrInfo));
    return bytesSent;
}

struct sockaddr_in TCPPacket::trollAddrInfo;
