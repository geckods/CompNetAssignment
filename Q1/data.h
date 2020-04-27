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
#ifndef Q1DATA_H
#define Q1DATA_H

// length of a single text block
#define PACKET_SIZE 100

// queue size
#define MAXPENDING 10

#define PORT1 12345
#define PORT2 12346

// packet drop rate (percentage)
#define PDR 20

// timeout values
#define RETRANSMISSION_TIME_SEC 2
#define RETRANSMISSION_TIME_USEC 0

// to accomodate the extra INTs in the packet structure
int BUFSIZE = PACKET_SIZE + 24;

typedef struct {
	int size;
	int seqNo;
	int lastPacket;
	int ack;
	int channelID;
	char data[PACKET_SIZE+1];
} packet;

// file pointers
int fileOffset=0;
int fileSize=0;


typedef struct timeval TimeVal;
typedef long long ll;


#endif