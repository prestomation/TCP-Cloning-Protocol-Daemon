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
#include "Timer/TimerService.h"

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
                //Called by TCPConn() to remove a listening socket
		void removeListeningSocket(int sockfd, TCPConn* acceptingConn);

        //Double-dispatch for the TimerService. TCPConns' create a new Timer with this method
        //This allows TCPDaemon to be aware when a new timer is created
        void addTimer(uint32_t time, uint32_t seqNum, TCPConn& theConn);

        //Double-dispatch for the TimerService. TCPConns' remove Timers when a packet is ACK'd
        //This allows TCPDaemon to be aware when a timer is removed incase the poll() timeout must be changed
        void removeTimer(uint32_t seqNum, TCPConn& theConn);

        
        //Double-dispatch for the TimerService. TCPConns' remove all timers when a TCPConn is cleaning up
        void removeAllTimers(TCPConn& theConn);

        inline
            void setNextTimeout(DeltaTimer timer)
            {
                mCurrentTimer = timer;
            };

    private:

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

        //The current timer to timeout on
        DeltaTimer mCurrentTimer;
        //The TimerService instantiation
        TimerService theTimerService;


};

#endif //TCPDAEMON_H

