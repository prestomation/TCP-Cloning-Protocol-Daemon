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
<<<<<<< HEAD
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
=======
    int recvBytes = ::recvfrom(sock, &packet, sizeof(packet), 0, 0, 0);
    if (recvBytes !=sizeof(packet))
    {
        std::cout << "Unknown packet received. Dropped" << std::endl;
    }

}

TCPPacket::~TCPPacket()
{
>>>>>>> eac578e98d5e1190efd5d70a2cf847a288f9fdb6
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
