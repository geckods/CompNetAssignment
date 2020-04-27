#include <q1_data.h>

char* badSerialize(packet* thePacket){
	return (char*)thePacket;
}

packet* badUnSerialize(char* thePacket){
	return (packet*)thePacket;
}


int isOlder(TimeVal t1, TimeVal t2){
	return ((t1.tv_sec<t2.tv_sec)||((t1.tv_sec==t2.tv_sec)&&(t1.tv_usec<=t2.tv_usec)));
}

TimeVal time_diff(TimeVal x ,TimeVal y)
{
	ll x_ms , y_ms , diff;
	
	x_ms = x.tv_sec*1000000 + x.tv_usec;
	y_ms = y.tv_sec*1000000 + y.tv_usec;
	
	diff = x_ms - y_ms;
	
	TimeVal t3;
	t3.tv_sec = diff/1000000;
	t3.tv_usec = diff%1000000;

	return t3;
}

int max(int a, int b){
	return (a>b)?a:b;
}

int getNextBlock(FILE* inputFile, char* buffer, int *isFinalBlock){
	// returns offset of returned block
	// fills in isFinalBlock as necessary
	// length of string is implicit in strlen(buffer)
	int readSize = fread(buffer, sizeof(char),PACKET_SIZE, inputFile);
	buffer[readSize]='\0';
	*isFinalBlock=0;
	// fread(void *restrict __ptr, size_t __size, size_t __n, FILE *restrict __stream)
	if(feof(inputFile) || ftell(inputFile) == fileSize)*isFinalBlock=1;
	int toReturn=fileOffset;
	fileOffset+=readSize;
	return toReturn;
}

packet *getNextPacket(FILE* inputFile, int channelID){
	// returns isFinalPacket?


	packet* thePacket = malloc(sizeof(packet));
	thePacket->channelID = channelID;
	thePacket->seqNo = getNextBlock(inputFile, thePacket->data, &thePacket->lastPacket);
	thePacket->size = strlen(thePacket->data);
	thePacket->ack = 0;
	// fprintf(stderr, "%d %d\n",thePacket->size, thePacket->seqNo);
	// fprintf(stderr, "%s\n",thePacket->data);
	return thePacket;
}

packet *getAckPacket(packet* iPacket){
	packet* thePacket = malloc(sizeof(packet));
	thePacket->channelID = iPacket->channelID;
	thePacket->seqNo = iPacket->seqNo;
	thePacket->size = 0;
	thePacket->ack = 1;
	thePacket->lastPacket=iPacket->lastPacket;
	thePacket->data[0]=0;
	return thePacket;
}

void pcktPrint(int sent, int seqNo, int size, int channel){
	if(sent){
		printf("SENT PKT: Seq. No %d of size %d bytes from channel %d.\n",seqNo,size,channel);
	}
	else{
		printf("RCVD PKT: Seq. No %d of size %d bytes from channel %d.\n",seqNo,size,channel);		
	}
}

void ackPrint(int sent, int seqNo, int channel){
	if(sent){
		printf("SENT ACK: For PKT with Seq. No %d from channel %d.\n",seqNo,channel);
	}
	else{
		printf("RCVD ACK: For PKT with Seq. No %d from channel %d.\n",seqNo,channel);
	}
}