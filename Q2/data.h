#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>
#include <limits.h>
#include <poll.h>
#ifndef Q2DATA_H
#define Q2DATA_H

// length of char packets
#define PACKET_SIZE 100

// window for selective repeat
#define WINDOWSIZE 10

// in microseconds, delay added by relayes 
#define MAXDELAY 2000

// in seconds, time taken without message for relays to terminate
#define RELAYTIMEOUT 10

// percentage chance of packet drop
#define PDR 30

// timeout for client
#define RETRANSMISSION_TIME_SEC 2
#define RETRANSMISSION_TIME_USEC 0

typedef long long ll;

enum NODENAME{
	CLIENT,SERVER,RELAY1,RELAY2
};

enum EVENTTYPE{
	SEND,RECV,DROP,TIMEOUT,RETRANS
};

enum PACKETTYPE{
	DATA,ACK
};

// to accomodate ints
int BUFSIZE = PACKET_SIZE + 24;

typedef struct {
	int size;
	int seqNo;
	int lastPacket;
	int ack;
	int channelID;
	char data[PACKET_SIZE+1];
} packet;


#define CLIENT_IP "127.0.0.1"
#define SERVER_IP "127.0.0.1"
#define RELAY1_IP "127.0.0.1"
#define RELAY2_IP "127.0.0.1"

#define R1_PORT 12345
#define R2_PORT 12346
#define SERVER_PORT 12347


int fileOffset=0;
int fileSize;

typedef struct timeval TimeVal;

#endif