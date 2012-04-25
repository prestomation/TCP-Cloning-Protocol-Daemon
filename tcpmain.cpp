#include <signal.h>
#include <iostream>


#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>

#include "tcpdaemon/TCPDaemon.h"



//Global instantiation of our Daemon
TCPDaemon tcpdaemon;

void sigintHandler(int dummy)
{
    //Signal the daemon to stop. We will still have to wait for poll to timeout
    std::cout << "Caught SIGINT. Stopping.. \n" << std::endl;
    tcpdaemon.Stop();

}


//From the shell, the daemon is called with the host and port where troll is listening
int main(int argc, char *argv[])
{

    //Add a CTRL-C handler
    signal(SIGINT, sigintHandler);

    struct hostent *trollHostInfo;
    struct sockaddr_in trollSockInfo;

    if (argc != 3)
    {
        std::cout << "Please enter a valid address and port for the troll " << std::endl;
        return 1;
    }


    if((trollHostInfo = gethostbyname(argv[1])) == NULL)
    {
        std::cout << "Please enter a valid address and port for the troll " << std::endl;
        return 1;
    }

    bcopy(trollHostInfo->h_addr, &trollSockInfo.sin_addr, trollHostInfo->h_length);
    trollSockInfo.sin_family = AF_INET;
    trollSockInfo.sin_port = htons(atoi(argv[2]));

    tcpdaemon.Start(trollSockInfo);
    return 0;
}
