// In addition to the payload, the packet structure should contain following information in form
// of a header.
// a. The size (number of bytes) of the payload
// b. The Seq. No. (in terms of byte) specifying the offset of the first byte of the packet with
// respect to the input file.
// c. Whether the packet is last packet or not?
// d. The packet is DATA or ACK. In this way, you can utilize the same packet structure for
// both DATA send by the client and ACK send by the server. The Seq. No. field in the
// ACK packet would correspond to it DATA packet with the same Seq. No. value received
// from the client.
// e. The channel information specifying the Id (either 0 or 1) of the channel through which
// the packet has been transmitted.
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

#define PACKET_SIZE 100
#define MAXPENDING 10
#define PORT1 12345
#define PORT2 12346
#define PDR 0

int BUFSIZE = PACKET_SIZE + 24;

typedef struct {
	int size;
	int seqNo;
	int lastPacket;
	int ack;
	int channelID;
	char data[PACKET_SIZE+1];
} packet;

int fileOffset=0;
int fileSize=0;

#define RETRANSMISSION_TIME_SEC 2
#define RETRANSMISSION_TIME_USEC 0

typedef struct timeval TimeVal;
typedef long long ll;


#endif