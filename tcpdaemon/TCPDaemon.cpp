
//System Defines
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> //inet_ntop
#include <unistd.h> //sock
#include <stdio.h> //perror
#include <sys/un.h> //sockaddr_un, 
#include <sys/poll.h> //poll
#include <iostream> //cout
#include <string> //cout
using namespace std; 


//Application Defines
#include "common.h"
#include "api.h"
#include "TCPDaemon.h"
#include "packets/IPCPackets.h"
#include "packets/TCPPackets.h"


TCPDaemon::TCPDaemon(): mNumListeningSockets(1),mRunning(true)
{
}

void TCPDaemon::Start(sockaddr_in trollInfo)
{

	//Inform our TCPPackets where to send all outgoing packets
	TCPPacket::trollAddrInfo = trollInfo;

	//Setup our sockets
	mLocalSock = this->setupLocalSocket();

	//Setup our polling
	int rv;

	mListeningSockets[0].fd = mLocalSock;
	mListeningSockets[0].events = POLLIN;

	while(1)
	{
		//cout << "Press a key" << endl;
		//string test;
		//cin >> test;
		//Poll with 5 sec timeout
		rv = poll(mListeningSockets, mNumListeningSockets, 5000);

		if (rv == -1)
		{
			cout << "Error with poll... " << endl;
			return;
		}
		else if (rv == 0)
		{
			cout << "Poll timeout!" << endl;
		}
		else
		{
			if (mListeningSockets[0].revents & POLLIN)
			{
				//mLocalSock traffic

				struct sockaddr_un incomingIPCInfo;
				IPCPacket* incomingpacket = ReceivePacket(mLocalSock, &incomingIPCInfo);
				//cout << "IPCaddr: " << incomingIPCInfo.sun_path << endl;
				if(!incomingpacket)
				{
					cout <<  "!!!!!!!!Unrecognized opcode!!!!!!!!" << endl;
				}
				else if (incomingpacket->getOpcode() == OPCODE_BIND_REQUEST)
				{
					//bind request

					TCPConn* newconn = new TCPConn(*this, dynamic_cast<BindRequestPacket*>(incomingpacket), incomingIPCInfo, mLocalSock);

					mIPCConnMap[incomingIPCInfo.sun_path] = newconn;
				}
				else if (incomingpacket->getOpcode() == OPCODE_CONNECT_REQUEST)
				{
					//bind request

					TCPConn* newconn = new TCPConn(*this, dynamic_cast<ConnectRequestPacket*>(incomingpacket), incomingIPCInfo, mLocalSock);

					mIPCConnMap[incomingIPCInfo.sun_path] = newconn;
				}

				else if (incomingpacket->getOpcode() == OPCODE_ACCEPT_REQUEST)
				{
					cout << "Got accept request packet" << endl;

					AcceptRequestPacket* request = dynamic_cast<AcceptRequestPacket*>(incomingpacket);

					if(mIPCConnMap.count(incomingIPCInfo.sun_path) == 0)
					{
						//TODO
						cout <<"Not implemented!" << endl;
						//This socket is not binded yet. make a new tcpconn that binds implicitly
						//TCPConn* newconn = new TCPConn(request, incomingIPCInfo, mLocalSock);

						//mIPCConnMap[incomingIPCInfo.sun_path] = newconn;
					}
					else
					{
						mIPCConnMap[incomingIPCInfo.sun_path]->Accept(request);
						//This sock is already binded
					}
				}
				else if (incomingpacket->getOpcode() == OPCODE_RECV_REQUEST)
				{
					RecvRequestPacket* request = dynamic_cast<RecvRequestPacket*>(incomingpacket);
					mIPCConnMap[incomingIPCInfo.sun_path]->RecvRequest(request);

				}
				else if (incomingpacket->getOpcode() == OPCODE_SEND_REQUEST)
				{
					SendRequestPacket* request = dynamic_cast<SendRequestPacket*>(incomingpacket);
					mIPCConnMap[incomingIPCInfo.sun_path]->SendRequest(request);
				}
				else{
					cout << "Factory returned a packet we're apparently not handling...." << endl;
				}
			}
		}

		for (int i = 1; i < mNumListeningSockets; i++)
		{
			//Iterate through client listening sockets.
			if(mListeningSockets[i].revents & POLLIN)
			{
				//This socket has stuff!
				cout << "Socket " << mListeningSockets[i].fd << " has stuff!" << endl;
				mListeningMap[mListeningSockets[i].fd]->ReceiveData();
			}
		}
	}
	//clean up
	cout << "Cleaning up.." << endl;
	close(mLocalSock);
	unlink(DAEMON_SOCKET);
}




//When a API server bind()s to a local socket, this method passes that socket to
//the Daemon to add to the list of poll()ing sockets. 
void TCPDaemon::addListeningSocket(int sockfd, TCPConn* acceptingConn)
{

	cout << "Adding " << sockfd << " to mListeningMap" << endl;
	mListeningMap[sockfd] = acceptingConn;

	mListeningSockets[mNumListeningSockets].fd = sockfd;
	mListeningSockets[mNumListeningSockets].events = POLLIN;
	mNumListeningSockets++;

}

int TCPDaemon::setupLocalSocket()
{
	int asocket;
	struct sockaddr_un ipcInfo;


	ipcInfo.sun_family = AF_UNIX;
	strcpy(ipcInfo.sun_path, DAEMON_SOCKET);

	unlink(ipcInfo.sun_path);

	/*initialize socket connection in unix domain*/
	if((asocket = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0){
		perror("error opening AF_UNIX datagram socket");
		return -1;
	}


	if(bind(asocket, (struct sockaddr *)&ipcInfo, sizeof(struct sockaddr_un)) < 0) {
		perror("error binding AF_UNIX socket");
		return -1;
	}

	return asocket;

}


void TCPDaemon::Stop()
{
	//notify the daemon to stop.
	mRunning = false;

}


ssize_t TCPDaemon::sendto(int sockfd, const void *buf, size_t len, int flags,
		const struct sockaddr *dest_addr, socklen_t addrlen)
{
/*	int bytesSent;

	TrollMessage message;
	//Copy values into the header

	cout << "Sending troll message" << endl;
	message.header.sin_family = htons(AF_INET);
	message.header.sin_addr = ((sockaddr_in*)dest_addr)->sin_addr;
	message.header.sin_port  = ((sockaddr_in*)dest_addr)->sin_port;
	char ipAddr[INET_ADDRSTRLEN] ;
	inet_ntop(AF_INET, &message.header.sin_addr.s_addr, ipAddr, INET_ADDRSTRLEN);
	cout << "Message destined for " <<  ipAddr;
	message.bodysize = len;
	memcpy(&message.body, buf, len);



	bytesSent = ::sendto(sockfd, &message, sizeof(message), flags, (struct sockaddr*)&mTrollInfo, sizeof(mTrollInfo));
	return bytesSent - sizeof(message.header);*/


}


IPCPacket* TCPDaemon::ReceivePacket(int sockfd, sockaddr_un* incomingIPCInfo)
{

	int sockaddr_unLen = sizeof(*incomingIPCInfo);

	char opcode;

	if ( recvfrom(sockfd, &opcode, sizeof(opcode), 0, (struct sockaddr *)incomingIPCInfo, (socklen_t*) &sockaddr_unLen)< 0)
	{
		perror("Error reading from domain socket");
	}
	cout << "IPCaddr: " << incomingIPCInfo->sun_path << endl;

	//We should only ever get REQUEST packets, as we generate all RESPONSE packets

	if (opcode == OPCODE_BIND_REQUEST)
	{
		cout << "Received Bind Request" << endl;
		BindRequestPacket* packet = new BindRequestPacket;
		packet->receive(sockfd); 
		return packet;
	}

	if (opcode == OPCODE_CONNECT_REQUEST)
	{
		cout << "Received Connect Request" << endl;
		ConnectRequestPacket* packet = new ConnectRequestPacket;
		packet->receive(sockfd); 
		return packet;
	}

	if (opcode == OPCODE_ACCEPT_REQUEST)
	{ 
		cout << "Received Accept Request" << endl;
		AcceptRequestPacket* packet = new AcceptRequestPacket;
		packet->receive(sockfd); 
		return packet;
	}
	if (opcode == OPCODE_RECV_REQUEST)
	{
		cout << "Received Recv Request" << endl;
		RecvRequestPacket* packet = new RecvRequestPacket;
		packet->receive(sockfd); 
		return packet;
	}
	if (opcode == OPCODE_SEND_REQUEST)
	{
		cout << "Received Send Request" << endl;
		SendRequestPacket* packet = new SendRequestPacket;
		packet->receive(sockfd); 
		return packet;
	}
	return NULL;

}
