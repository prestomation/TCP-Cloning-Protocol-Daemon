/* server.c using UDP */

/* Server for accepting an Internet stream connection on port 1040 */
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include "tcpdaemon/api.h"
using namespace std;

#define BUF_SIZE 1024
#define FILESIZE_BYTES 4
#define FILENAME_BYTES 20

/* server program called with no argument */
int main(int argc, char *argv[])
{
	int sock, msgsock;                     /* initial socket descriptor */
    int filesize;
    char filename[21];

    char listenport[6];            /* port to listen on as specified on the cli */  

    struct sockaddr_in sin_addr; /* structure for socket name setup */
    char buf[BUF_SIZE];               /* buffer for holding read data */
    ofstream outputFile ;
    int totalbytes = 0;
    int bytesread ;

    if(argc < 2) {
        cout << "usage: ftps local-port" << endl;
        exit(1);
    }

    strcpy(listenport,argv[1]);

    cout << "UDP server waiting for remote connection from clients ..." << endl;

    /*initialize socket connection in unix domain*/
    if((sock = SOCKET(AF_INET, SOCK_DGRAM, 0)) < 0){
        perror("error opening datagram socket");
        exit(1);
    }

    /* construct name of socket to send to */
    sin_addr.sin_family = AF_INET;
    sin_addr.sin_addr.s_addr = INADDR_ANY;
    sin_addr.sin_port = htons(atoi(listenport));

    /* bind socket name to socket */
    if(BIND(sock, (struct sockaddr *)&sin_addr, sizeof(struct sockaddr_in)) < 0) {
        perror("error binding stream socket");
        exit(1);
    }

    /* listen for socket connection and set max opened socket connetions to 5 */
    LISTEN(sock, 5);


    /* put all zeros in buffer (clear) */
    bzero(buf,BUF_SIZE);

    /* accept a (1) connection in socket msgsocket */ 
    if((msgsock = ACCEPT(sock, (struct sockaddr *)NULL, NULL)) == -1) { 
        perror("error connecting stream socket");
        exit(1);
    } 

    /*The first 4 bytes is the number of data bytes to read */
    cout << "Receiving filesize" << endl;
    if(RECV(msgsock, &filesize, FILESIZE_BYTES, 0) < 0) {
        perror("error reading on stream socket");
        exit(1);
    } 

    printf("This file will be %d bytes\n", filesize);

    /*The next 20 bytes contain the filename */
    if(RECV(msgsock, &filename, FILENAME_BYTES, 0) < 0) {
        perror("error reading on stream socket");
        exit(1);
    } 
    cout << "This file will be called " << filename << endl;



    /* open the file for  binary output*/
    outputFile.open(filename, ios::out|ios::binary);
    if (!outputFile.is_open())
    {
        cout << "Error opening " << filename << " for writing! Exiting.." << endl;
    }




    /* read from msgsock and write to file */
    while(1)
    {
        bytesread = RECV(sock, buf, BUF_SIZE, 0);

        outputFile.write(buf,bytesread);


        totalbytes += bytesread;

        if (bytesread < 0)
        {
            perror("Error! ");
            exit(1);
        }



        if (bytesread < BUF_SIZE)
        {
            cout << totalbytes << " bytes written" << endl ;
            cout << "File written. We're done" << endl;
            break;
        }



    }

    /* close all connections and remove socket file */
    close(sock);
    outputFile.close();
}
