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

    // recompute checksum
    uint16_t originalCRC = packet.checksum; // save incoming checksum
    packet.checksum = 0; // erase it
    packet.checksum =  htonl(crc((uint8_t *)&packet, sizeof(packet))); 

    // check to see if original and new crc match
    if (originalCRC != packet.checksum)
    {
	printf("checksum %d is bad, expected %d\n", packet.checksum, originalCRC);
    }
}

int TCPPacket::send(int sock)
{
    initCRCTable(); // make the crc table before use, this should
		    // probably be moved to the main function to save time
    int bytesSent = 0;

    // compute the checksum
    packet.checksum = htonl(crc((uint8_t *)&packet, sizeof(packet)));

    char * destinet;
    destinet = inet_ntoa(packet.header.sin_addr);
    std::cout << "Sending TCP Packet destined for "<< destinet << std::endl;

    bytesSent += ::sendto(sock, &packet, sizeof(packet), 0, (struct sockaddr*)&trollAddrInfo, sizeof(trollAddrInfo));
    return bytesSent;
}

struct sockaddr_in TCPPacket::trollAddrInfo;

// Create table for all possible remainders for ascii values
void TCPPacket::initCRCTable()
{
    polynomial = 0x18005; // Polynomial divisor 17 bit 
    int i, j;
    for (i = 0; i < 256; i++) { // 256 upper extended ascii value
        uint16_t crc = i;
        for (j = 8; j > 0; j--) { // go through the 8 bits of each value
            if (crc & 1) { // bitwise AND
                crc = (crc >> 1) ^ polynomial; // bitwise shift right, XOR
            } else {
                crc >>= 1;
            }
        }
        crc_table[i] = crc; // store
    }
}

// Formula for calculating the crc, done with inverted checksum 
uint16_t TCPPacket::crc(uint8_t *payload, unsigned int loadsize)
{
    uint16_t crc = 0xFFFF; // invert checksum to comply with table
    uint32_t i;
    for (i = 0; i < loadsize; i++)
        crc = ((crc >> 8) & 0x00FF) ^ crc_table[(crc ^ *payload++) & 0xFF];
    return (crc ^ 0xFFFF); // XOR again
}
