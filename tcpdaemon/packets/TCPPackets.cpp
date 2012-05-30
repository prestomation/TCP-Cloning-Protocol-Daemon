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
        //Only copy the payload if we have a valid pointer and a 
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

//Constructed with a socket, we are receiving a packet currently on the wire
TCPPacket::TCPPacket(int sock): goodChecksum(true)
{
    mState = STATE_INCOMING;

    ::recvfrom(sock, &packet, sizeof(packet), 0, 0, 0);

    // recompute checksum
    uint32_t originalCRC = packet.checksum; // save incoming checksum
    packet.checksum = 0; // erase it
    sockaddr_in tempHeader = packet.header;
    memset( &packet.header, 0, sizeof (packet.header ));  
    packet.checksum =  crc((uint8_t *)&packet, sizeof(packet)); 
    packet.header = tempHeader;

    //std::cout << "Checksum: " << originalCRC << " Computed: " << packet.checksum << std::endl;
    // check to see if original and new crc match
    if (originalCRC != packet.checksum)
    {
        std::cout << "!!!Checksum "<< packet.checksum << " is bad, expected: " << originalCRC << std::endl;;
        goodChecksum = false;

    }

}

TCPPacket::~TCPPacket()
{
}

int TCPPacket::send(int sock)
{
    int bytesSent = 0;

    // compute the checksum
    packet.checksum = 0;

    sockaddr_in tempHeader = packet.header;
    memset( &packet.header, 0, sizeof (packet.header ));  
    packet.checksum =  crc((uint8_t *)&packet, sizeof(packet)); 
    packet.header = tempHeader;
    //std::cout << "Computed checksum: " << packet.checksum << std::endl;

    char * destinet;
    destinet = inet_ntoa(packet.header.sin_addr);
    std::cout << "Sending TCP Packet destined for "<< destinet << std::endl;

    bytesSent += ::sendto(sock, &packet, sizeof(packet), 0, (struct sockaddr*)&trollAddrInfo, sizeof(trollAddrInfo));
    return bytesSent;
}

void TCPPacket::init(sockaddr_in destination, uint32_t seqnum, uint32_t acknum)
{

    if(!initCRC)
    {
        initCRCTable();
        initCRC= true;
    }
    memset( packet.payload, 0, sizeof (packet.payload ));  

    packet.header.sin_family = htons(AF_INET);
    packet.header.sin_addr = destination.sin_addr;
    packet.header.sin_port  = destination.sin_port;

    packet.seqNum = seqnum;
    packet.ackNum = acknum;
    mState = STATE_OUTGOING;
}

struct sockaddr_in TCPPacket::trollAddrInfo;

// Create table for all possible remainders for ascii values
void TCPPacket::initCRCTable()
{
    uint32_t poly = 0xEDB88320L; // initial polynomial
    int i, j;
    for (i = 0; i < 256; i++) { // character by character
        uint32_t crc = i;
        for (j = 8; j > 0; j--) {
	    // least significant bit first this will be
	    // compensated for in the actuall crc 
            if (crc & 1) { 
                crc = (crc >> 1) ^ poly;
            } else {
                crc >>= 1;
            }
        }
        crc_table[i] = crc;
    }
}

// Formula for calculating the crc, done with inverted checksum 
uint32_t TCPPacket::crc(uint8_t *payload, unsigned int loadsize)
{
    uint32_t i, crc = 0xFFFFFFFF;
    for (i = 0; i < loadsize; i++)
        crc = ((crc >> 8) & 0x00FFFFFF) ^ crc_table[(crc ^ *payload++) & 0xFF];
    return (crc ^ 0xFFFFFFFF);

}

uint32_t TCPPacket::crc_table[256];
bool TCPPacket::initCRC = false;

