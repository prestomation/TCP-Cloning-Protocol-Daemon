#include "TCPPackets.h"
#include <string.h> //memcpy
#include <iostream>
#include <arpa/inet.h>


TCPPacket::TCPPacket(sockaddr_in destination, uint32_t seqnum, uint32_t acknum, char * payload, int payloadsize, Flags flags)
{
    init(destination, seqnum, acknum);

    packet.flags = flags;

    packet.payloadsize = payloadsize;
    if(payload != NULL && payloadsize > 0)
    {
        //Only copy the payload if we have a valid pointer or a 
        //positive number of bytes 
        memcpy(packet.payload, payload, payloadsize); 
    } 


}

TCPPacket::TCPPacket(sockaddr_in destination, uint32_t seqnum, uint32_t acknum)
{
    init(destination, seqnum, acknum);
    //This is an ACK only packet
    packet.flags=FLAG_ACK;
}

//Construcuted with a socket, we are receiving a packet currently on the wire
TCPPacket::TCPPacket(int sock)
{
    mState = STATE_INCOMING;
    int recvBytes = ::recvfrom(sock, &packet, sizeof(packet), 0, 0, 0);
    if (recvBytes !=sizeof(packet))
    {
        std::cout << "Unknown packet received. Dropped" << std::endl;
    }

}

TCPPacket::~TCPPacket()
{
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

void TCPPacket::init(sockaddr_in destination, uint32_t seqnum, uint32_t acknum)
{

    memset( packet.payload, 0, sizeof (packet.payload ));  

    packet.header.sin_family = htons(AF_INET);
    packet.header.sin_addr = destination.sin_addr;
    packet.header.sin_port  = destination.sin_port;

    packet.seqNum = seqnum;
    packet.ackNum = acknum;
    mState = STATE_OUTGOING;
}

struct sockaddr_in TCPPacket::trollAddrInfo;
