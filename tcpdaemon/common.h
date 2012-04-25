
//This file contains constants that are used by both the daemon and the client API


#define DAEMON_SOCKET "/tmp/TCPDaemon"
#define OPCODE_BIND_REQUEST '\x00'
#define OPCODE_BIND_RESPONSE '\x01'
#define OPCODE_ACCEPT_REQUEST '\x02'
#define OPCODE_ACCEPT_RESPONSE '\x03'
#define OPCODE_RECV_REQUEST '\x04'
#define OPCODE_RECV_RESPONSE '\x05'
#define OPCODE_SEND_REQUEST '\x06'
#define OPCODE_SEND_RESPONSE '\x07'
#define OPCODE_CLOSE_REQUEST '\x08'
#define OPCODE_CLOSE_RESPONSE '\x09'
#define OPCODE_CONNECT_REQUEST '\x10'
#define OPCODE_CONNECT_RESPONSE '\x11'

