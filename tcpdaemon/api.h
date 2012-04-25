
#ifndef TCPDAEMONAPI_H
#define TCPDAEMONAPI_H
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h> //sock
#include <stdio.h> //perror
#include <sys/un.h> //sockaddr_un, strcpy
#include <sys/poll.h> //poll
#include <iostream> //cout


//Application includes
#include "common.h"

using namespace std;

//The following method have identical and attempt to behave identically to 
//their native, TCP counterpart


int SOCKET(int socket_family, int socket_type, int protocol);
int BIND(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int LISTEN(int socketfd, int backlog);
int ACCEPT(int sockfd, const struct sockaddr *addr, socklen_t* addrlen);
int CONNECT(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
ssize_t SEND(int sockfd, const void *buf, size_t len, int flags);
int RECV(int sockfd, void *buf, size_t len, int flags);
int CLOSE(int sockfd);




#endif //TCPDAEMONAPI_H
