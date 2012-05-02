#ifndef TCPDAEMON_H
#define TCPDAEMON_H

#define BUF_SIZE 1024
#include <map>
#include <string>
#include <sys/poll.h> //poll
#include <netinet/in.h>
#include <sys/socket.h>

#include "TCPConn.h"
#include "packets/IPCPackets.h"

class sockaddr_un;
class sockaddr_in;

typedef std::map<std::string, TCPConn*> IPCConnMap;
typedef std::map<int, TCPConn*> ListeningMap;


class TCPDaemon {

	public:
		TCPDaemon();

		//Start the daemon, mostly a infinite poll(), first on the local domain socket
		//for API communication and then also for UDP ports for server accept()s
		void Start(sockaddr_in trollInfo);
		//Stop polling() and cleanup
		void Stop();
		//Called by a TCPConn() when a local socket is listening for incoming connections
		//acceptingConn is a pointer back to the associated TCPConn object.
		void addListeningSocket(int sockfd, TCPConn* acceptingConn);

	private:

		//TODO:Do this better, we shouldn't have a max
		//We are setting a max of 50 here
		pollfd mListeningSockets[50];
		int mNumListeningSockets;

		bool mRunning;
		int mLocalSock;


		//A map of incoming IPC endpoints to the owned TCPConn
		IPCConnMap mIPCConnMap;
		//A map of listening UDP sockets to the TCPConn who cares about data
		//on that socket
		ListeningMap mListeningMap;

		//Contains info of where to send packets bound for the troll
		struct sockaddr_in mTrollInfo;

		//Setup the local IPC listening socket
		int setupLocalSocket();
		//Receive an incoming IPCPacket 
		IPCPacket* ReceivePacket(int sockfd, sockaddr_un* incoming);

};

#endif //TCPDAEMON_H

