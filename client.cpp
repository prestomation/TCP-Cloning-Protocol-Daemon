/* client.c using UDP */

/* Client for connecting to Internet datagram server waiting on port 1040 */
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <strings.h> //for bcopy on Solaris
#include <stdlib.h>
#include <cmath>
#include <iostream>
#include <fstream>

#include "tcpdaemon/api.h"
using namespace std;

#define BUF_SIZE 1024


/* client program called with host name where server is run */
int main(int argc, char *argv[])
{
    int sock;                     /* initial socket descriptor */
    struct sockaddr_in  sin_addr ; /* structure for socket name 
                                   setup */
    char filename[21];
    int filesize;

    char buf[BUF_SIZE];     /* message to set to server */
    struct hostent *hp;
    char serverport[6];
    ifstream  inputFile;
    int byteswritten ;
    int totalbytes = 0;


    if(argc < 2) {
        cout << "usage: ftpc remote_host port filename " << endl;
        exit(1);
    }


    strcpy(filename ,argv[3]);

    inputFile.open(filename, ios::in|ios::binary|ios::ate); //open the file for reading, binary, and start at the end to more easily get the size
    if (!inputFile.is_open()) {
        cerr << "Can't open input file " << filename << endl;
        exit(1);
    }

    /* We are already at the end, so get the position(filesize) and seek back*/
    filesize = inputFile.tellg();
    inputFile.seekg(0, std::ios::beg);

    /* initialize socket connection in unix domain */
    if((sock = SOCKET(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
        perror("error opening datagram socket");
        exit(1);
    }

    hp = gethostbyname(argv[1]);
    if(hp == 0) {
        cerr << argv[1] << ": unknown host" << endl;
        exit(2);
    }

    strcpy(serverport,argv[2]);

    /* construct name of socket to send to */
    bcopy((void *)hp->h_addr, (void *)&sin_addr.sin_addr, hp->h_length);
    sin_addr.sin_family = AF_INET;
    sin_addr.sin_port = htons(atoi(serverport)); /* fixed by adding htons() */

    //if(BIND(sock, (struct sockaddr *)&sin_addr, sizeof(struct sockaddr_in)) < 0) {
     //   perror("error binding stream socket");
      //  exit(1);
   // }

    if(CONNECT(sock, (struct sockaddr *)&sin_addr, sizeof(struct sockaddr_in)) < 0) {
        close(sock);
        perror("error connecting stream socket");
        exit(1);
    }



    /* write filesize to server */
    cout << "Filesize is " << filesize << endl;
    if(SEND(sock, &filesize, 4, 0) < 0) {
        perror("error writing on datagram socket");
        exit(1);
    }
    cout << "Send a filesize of " << filesize << " bytes." << endl;

    /* write filename to server */
cout << "Filename is " << filename << endl;
    if(SEND(sock, &filename, 20, 0) < 0) {
        perror("error writing on datagram socket");
        exit(1);
    }
    cout << "Send a filename of "<< filename << endl;

    while(1)
    {

        /* fill buf from file and write out our socket */
        inputFile.read(buf, BUF_SIZE);

        byteswritten = SEND(sock, buf, inputFile.gcount(), 0);

        cout << "Read " << inputFile.gcount() << " bytes from file. Wrote " << byteswritten <<" bytes to socket...\n";
        totalbytes +=  inputFile.gcount();

        
        if (inputFile.gcount() < 0 || byteswritten < 0)
        {
            perror("Error!");
            exit(1);
        }

        //We know we're done when we've read less then BUF_SIZE bytes
        if (inputFile.gcount() < BUF_SIZE)
        {
            cout << totalbytes << " bytes written" << endl;
            if (totalbytes == filesize)
            {
                cout << "File sent. We're done" << endl;
            }
            else
            {
                cout << "Something appears to be wrong.." << endl;
            }
            break;
        }

    }
    inputFile.close();
    CLOSE(sock);
}



