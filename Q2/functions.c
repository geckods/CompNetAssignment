#include "data.h"

int max(int a, int b){
	return (a>b)?a:b;
}

void die(char *s)
{
    perror(s);
    exit(1);
}

int isOlder(TimeVal t1, TimeVal t2){
	return ((t1.tv_sec<t2.tv_sec)||((t1.tv_sec==t2.tv_sec)&&(t1.tv_usec<=t2.tv_usec)));
}

char* badSerialize(packet* thePacket){
	return (char*)thePacket;
}

packet* badUnSerialize(char* thePacket){
	return (packet*)thePacket;
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

int dropPacket(){
	return rand()<((RAND_MAX)*(((double)PDR)/100));
}

char* get_current_time(){
	char* str = (char *)malloc(sizeof(char)*20);
	int ab;
	time_t currTime;
	struct tm* timeptr;
	TimeVal tv;

	currTime = time(NULL);
	timeptr = localtime(&currTime);
	gettimeofday(&tv,NULL);
	ab = strftime(str,20, "%H:%M:%S", timeptr);

	char milli[8];
	sprintf(milli, ".%06ld",tv.tv_usec);
	strcat(str, milli);
	return str;
}

char *nameToString(enum NODENAME name){
	char* ret = malloc(sizeof(char)*7);
	ret[6]=0;
	switch (name){
		case (CLIENT):
			strcpy(ret,"CLIENT");
			break;
		case(SERVER):
			strcpy(ret,"SERVER");
			break;
		case(RELAY1):
			strcpy(ret,"RELAY1");
			break;
		case(RELAY2):
			strcpy(ret,"RELAY2");
			break;
	}
	return ret;
}

char *typeToString(enum PACKETTYPE type){
	char* ret = malloc(sizeof(char)*5);
	ret[4]=ret[3]=0;
	switch (type){
		case (DATA):
			strcpy(ret,"DATA");
			break;
		case(ACK):
			strcpy(ret,"ACK");
			break;
	}
	return ret;
}

char *eventToString(enum EVENTTYPE type){
	char* ret = malloc(sizeof(char)*3);
	ret[2]=0;
	switch (type){
		case (SEND):
			strcpy(ret,"S");
			break;
		case(RECV):
			strcpy(ret,"R");
			break;
		case (DROP):
			strcpy(ret,"D");
			break;
		case(TIMEOUT):
			strcpy(ret,"TO");
			break;
		case (RETRANS):
			strcpy(ret,"RE");
			break;			
	}
	return ret;
}


void loggerMessage(enum NODENAME name,enum  EVENTTYPE event, char timestamp[20], enum PACKETTYPE type, int seqNo, enum NODENAME src, enum NODENAME dst){
	printf("%-6s %-2s %-20s %-4s %-3d %-6s %-6s\n",nameToString(name),eventToString(event),timestamp,typeToString(type),seqNo,nameToString(src),nameToString(dst));
}