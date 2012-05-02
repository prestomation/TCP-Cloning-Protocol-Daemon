# Makefile for ftpc and ftps
SHELL := /bin/bash

UNAME := $(shell uname)
BINDIR := bin/
CPP = g++
IPC_SRCS = tcpdaemon/api.cpp tcpdaemon/packets/IPCPackets.cpp
TCPP_SRCS = tcpdaemon/packets/TCPPackets.cpp
CLIENT_SRCS = client.cpp $(IPC_SRCS)
SERVER_SRCS = server.cpp  $(IPC_SRCS)
TIMER_SRCS = tcpdaemon/Timer/TimerService.cpp
DAEMON_SRCS = tcpmain.cpp tcpdaemon/TCPDaemon.cpp  tcpdaemon/TCPConn.cpp $(IPC_SRCS) $(TIMER_SRCS) $(TCPP_SRCS)
DAEMON_HDRS = tcpdaemon/TCPDaemon.h  tcpdaemon/packets/IPCPackets.h
CFLAGS = -ansi -D_GNU_SOURCE  -Wall -g
LIBS = 

# setup for system
ifeq ($(UNAME), Linux)
LIBS += 
endif
ifeq ($(UNAME), SunOS)
LIBS += -lnsl -lsocket 
endif


all: mkd ftpc ftps tcpdaemon 

ftpc:	$(CLIENT_SRCS) 
	$(CPP) $(CFLAGS) -o bin/$@ $(CLIENT_SRCS) $(LIBS)

ftps:	$(SERVER_SRCS)
	$(CPP) $(CFLAGS) -o bin/$@ $(SERVER_SRCS)  $(LIBS)

tcpdaemon:	$(DAEMON_SRCS) $(DAEMON_HDRS) 
	$(CPP) $(CFLAGS) -o bin/$@ $(DAEMON_SRCS) $(LIBS)


mkd:
	mkdir -p $(BINDIR) 

clean:
	rm -rf $(BINDIR) 

