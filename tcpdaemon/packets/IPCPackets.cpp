#include <sys/socket.h>
#include <sys/un.h> //sockaddr_un, strcpy
#include <iostream>
#include <stdio.h>

#include "IPCPackets.h"
#include "../common.h"
using namespace std;






char IPCPacket::getOpcode()
{
    return mOpcode;
}

int BindRequestPacket::send(int sockfd, struct sockaddr_un conninfo)
{
    cout << "Sending BindRequest: " << "mOpcode: " << this->getOpcode() << " addr: " << this->addr << " port: " << this->port << endl; 

    if(sendto(sockfd, &mOpcode, sizeof(mOpcode), 0,(struct sockaddr *)&conninfo, sizeof(conninfo) ) < 0) {

        cout << "failed to send opcode" << endl;
        return -1;
    }   
    //send addr
    if(sendto(sockfd, &addr, sizeof(addr), 0,(struct sockaddr *)&conninfo, sizeof(conninfo) ) < 0) {
        cout << "failed to send addr" << endl;
        return -1;
    }   

    //send port
    if(sendto(sockfd, &port, sizeof(port), 0,(struct sockaddr *)&conninfo, sizeof(conninfo) ) < 0) {

        cout << "failed to send port" << endl;
        return -1;
    } 

    return 0;
}

int BindRequestPacket::receive(int sockfd)
{
    cout << "Receiving packet.."  << endl;

    //recv addr
    if(recv(sockfd, &addr, sizeof(addr), 0) < 0 )
    {
        return -1;
    }   

    //recv port
    if(recv(sockfd, &port, sizeof(port), 0) < 0 )
    {
        return -1;
    }   

    cout << "Received addr: " << addr << " port: " << port << endl;

    return 0;
}
int ConnectRequestPacket::send(int sockfd, struct sockaddr_un conninfo)
{
    cout << "Sending ConnectRequest: " << "mOpcode: " << this->getOpcode() << " addr: " << this->addr << " port: " << this->port << endl; 

    if(sendto(sockfd, &mOpcode, sizeof(mOpcode), 0,(struct sockaddr *)&conninfo, sizeof(conninfo) ) < 0) {

        cout << "failed to send opcode" << endl;
        return -1;
    }   
    //send addr
    if(sendto(sockfd, &addr, sizeof(addr), 0,(struct sockaddr *)&conninfo, sizeof(conninfo) ) < 0) {
        cout << "failed to send addr" << endl;
        return -1;
    }   

    //send port
    if(sendto(sockfd, &port, sizeof(port), 0,(struct sockaddr *)&conninfo, sizeof(conninfo) ) < 0) {

        cout << "failed to send port" << endl;
        return -1;
    } 

    return 0;
}

int ConnectRequestPacket::receive(int sockfd)
{
    cout << "Receiving packet.."  << endl;

    //recv addr
    if(recv(sockfd, &addr, sizeof(addr), 0) < 0 )
    {
        return -1;
    }   

    //recv port
    if(recv(sockfd, &port, sizeof(port), 0) < 0 )
    {
        return -1;
    }   

    cout << "Received addr: " << addr << " port: " << port << endl;

    return 0;
}


int AcceptRequestPacket::send(int sockfd, struct sockaddr_un conninfo)
{
    cout << "Sending AcceptRequest" << endl;

    if(sendto(sockfd, &mOpcode, sizeof(mOpcode), 0,(struct sockaddr *)&conninfo, sizeof(conninfo) ) < 0) {

        cout << "failed to send opcode" << endl;
        return -1;
    }   
    return 0;
}

int AcceptRequestPacket::receive(int sockfd)
{
//No payload for this packet type
    return 0;
}

int AcceptResponsePacket::send(int sockfd, struct sockaddr_un conninfo)
{
    //cout << "Sending AcceptResponse" << endl;

    if(sendto(sockfd, &mOpcode, sizeof(mOpcode), 0,(struct sockaddr *)&conninfo, sizeof(conninfo) ) < 0) {

        cout << "failed to send opcode" << endl;
        return -1;
    }   
    if(sendto(sockfd, &code, sizeof(code), 0,(struct sockaddr *)&conninfo, sizeof(conninfo) ) < 0) {
        return -1;
    }   

    if(sendto(sockfd, &connID, sizeof(connID), 0,(struct sockaddr *)&conninfo, sizeof(conninfo) ) < 0) {
        return -1;
    }   

    if(sendto(sockfd, &addr, sizeof(addr), 0,(struct sockaddr *)&conninfo, sizeof(conninfo) ) < 0) {
        return -1;
    }   
    if(sendto(sockfd, &port, sizeof(port), 0,(struct sockaddr *)&conninfo, sizeof(conninfo) ) < 0) {
        return -1;
    }   
    return 0;
}

int AcceptResponsePacket::receive(int sockfd)
{

    if(recv(sockfd, &code, sizeof(code), 0) < 0 )
    {
        return -1;
    }   

    if(recv(sockfd, &connID, sizeof(connID), 0) < 0 )
    {
        return -1;
    }   

    if(recv(sockfd, &addr, sizeof(addr), 0) < 0 )
    {
        return -1;
    }   

    if(recv(sockfd, &port, sizeof(port), 0) < 0 )
    {
        return -1;
    }   
    return 0;
}


int RecvRequestPacket::send(int sockfd, struct sockaddr_un conninfo)
{
    cout << "Sending RecvRequest" << endl;

    if(sendto(sockfd, &mOpcode, sizeof(mOpcode), 0,(struct sockaddr *)&conninfo, sizeof(conninfo) ) < 0) {
        return -1;
    }   

    if(sendto(sockfd, &sockid, sizeof(sockid), 0,(struct sockaddr *)&conninfo, sizeof(conninfo) ) < 0) {
        return -1;
    }   

    if(sendto(sockfd, &bufsize, sizeof(bufsize), 0,(struct sockaddr *)&conninfo, sizeof(conninfo) ) < 0) {
        return -1;
    }   
    return 0;
}

int RecvRequestPacket::receive(int sockfd)
{
    if(recv(sockfd, &sockid, sizeof(sockid), 0) < 0 )
    {
        return -1;
    }   

    if(recv(sockfd, &bufsize, sizeof(bufsize), 0) < 0 )
    {
        return -1;
    }   
    return 0;
}

int RecvResponsePacket::send(int sockfd, struct sockaddr_un conninfo)
{
    cout << "Sending RecvResponse" << endl;

    if(sendto(sockfd, &mOpcode, sizeof(mOpcode), 0,(struct sockaddr *)&conninfo, sizeof(conninfo) ) < 0) {

        return -1;
    }   
    if(sendto(sockfd, &size, sizeof(size), 0,(struct sockaddr *)&conninfo, sizeof(conninfo) ) < 0) {
        return -1;
    }   

    if(sendto(sockfd, &data, size, 0,(struct sockaddr *)&conninfo, sizeof(conninfo) ) < 0) {
        return -1;
    }   

    return 0;
}

int RecvResponsePacket::receive(int sockfd)
{

    if(recv(sockfd, &size, sizeof(size), 0) < 0 )
    {
        return -1;
    }   
    if(recv(sockfd, &data, size, 0) < 0 )
    {
        return -1;
    }   

    return 0;
}
int SendRequestPacket::send(int sockfd, struct sockaddr_un conninfo)
{
    cout << "Sending SendRequest" << endl;

    if(sendto(sockfd, &mOpcode, sizeof(mOpcode), 0,(struct sockaddr *)&conninfo, sizeof(conninfo) ) < 0) {
        return -1;
    }   

    if(sendto(sockfd, &sockid, sizeof(sockid), 0,(struct sockaddr *)&conninfo, sizeof(conninfo) ) < 0) {
        return -1;
    }   

    if(sendto(sockfd, &size, sizeof(size), 0,(struct sockaddr *)&conninfo, sizeof(conninfo) ) < 0) {
        return -1;
    }   
    if(sendto(sockfd, &data, size, 0,(struct sockaddr *)&conninfo, sizeof(conninfo) ) < 0) {
        return -1;
    }   
    return 0;
}

int SendRequestPacket::receive(int sockfd)
{
    if(recv(sockfd, &sockid, sizeof(sockid), 0) < 0 )
    {
        return -1;
    }   

    if(recv(sockfd, &size, sizeof(size), 0) < 0 )
    {
        return -1;
    }   
    if(recv(sockfd, &data, size, 0) < 0 )
    {
        return -1;
    }   
    return 0;
}

int SendResponsePacket::send(int sockfd, struct sockaddr_un conninfo)
{
    //cout << "Sending SendResponse" << endl;

    if(sendto(sockfd, &mOpcode, sizeof(mOpcode), 0,(struct sockaddr *)&conninfo, sizeof(conninfo) ) < 0) {

        return -1;
    }   

    if(sendto(sockfd, &bytesSent, sizeof(bytesSent), 0,(struct sockaddr *)&conninfo, sizeof(conninfo) ) < 0) {
        return -1;
    }   

    return 0;
}

int SendResponsePacket::receive(int sockfd)
{

    if(recv(sockfd, &bytesSent, bytesSent, 0) < 0 )
    {
        return -1;
    }   

    return 0;
}

int CloseRequestPacket::send(int sockfd, struct sockaddr_un conninfo)
{
    cout << "Sending CloseRequest" << endl;

    if(sendto(sockfd, &mOpcode, sizeof(mOpcode), 0,(struct sockaddr *)&conninfo, sizeof(conninfo) ) < 0) {
        return -1;
    }   

    if(sendto(sockfd, &sockid, sizeof(sockid), 0,(struct sockaddr *)&conninfo, sizeof(conninfo) ) < 0) {
        return -1;
    }   
 
    return 0;
}

int CloseRequestPacket::receive(int sockfd)
{
    if(recv(sockfd, &sockid, sizeof(sockid), 0) < 0 )
    {
        return -1;
    }   

    return 0;
}

int CloseResponsePacket::send(int sockfd, sockaddr_un conninfo)
{
 cout << "Sending CloseResponse" << endl;

    if(sendto(sockfd, &mOpcode, sizeof(mOpcode), 0,(struct sockaddr *)&conninfo, sizeof(conninfo) ) < 0) {
        return -1;
    }   

    if(sendto(sockfd, &status, sizeof(status), 0,(struct sockaddr *)&conninfo, sizeof(conninfo) ) < 0) {
        return -1;
    }   
 
    return 0;
}

int CloseResponsePacket::receive(int sockfd)
{
    if(recv(sockfd, &status, sizeof(status), 0) < 0)
    {
        return -1;
    }

    return 0;
}

//The following constructors set opcodes
BindRequestPacket::BindRequestPacket(): IPCPacket(OPCODE_BIND_REQUEST){}
BindResponsePacket::BindResponsePacket(): IPCPacket(OPCODE_BIND_RESPONSE){}
ConnectRequestPacket::ConnectRequestPacket(): IPCPacket(OPCODE_CONNECT_REQUEST){}
ConnectResponsePacket::ConnectResponsePacket(): IPCPacket(OPCODE_CONNECT_RESPONSE){}
AcceptRequestPacket::AcceptRequestPacket() : IPCPacket(OPCODE_ACCEPT_REQUEST){}
AcceptResponsePacket::AcceptResponsePacket() : IPCPacket(OPCODE_ACCEPT_RESPONSE){}
RecvRequestPacket::RecvRequestPacket() : IPCPacket(OPCODE_RECV_REQUEST){}
RecvResponsePacket::RecvResponsePacket() : IPCPacket(OPCODE_RECV_RESPONSE){}
SendRequestPacket::SendRequestPacket() : IPCPacket(OPCODE_SEND_REQUEST){}
SendResponsePacket::SendResponsePacket() : IPCPacket(OPCODE_SEND_RESPONSE){}
CloseResponsePacket::CloseResponsePacket() : IPCPacket(OPCODE_CLOSE_RESPONSE){}
CloseRequestPacket::CloseRequestPacket() : IPCPacket(OPCODE_CLOSE_REQUEST){}
 


