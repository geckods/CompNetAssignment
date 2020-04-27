#include "q2_data.h"
#include "q2_functions.c"

int main(int argc, char* argv[]){

	srand(time(0));

	// setting up output file
	char* fileName = "output.txt";
	if(argc>2){
		perror("Expected one or less command line argument, got more.");
		return -1;
	}
	else if (argc==2){
		fileName=argv[1];
	}

	// Open file
	FILE* myFile = fopen(fileName,"w");
	if(myFile==NULL){
		fprintf(stderr,"Could not open file %s: ", fileName);
		return -1;
	}


	struct sockaddr_in si_me, si_relay;
    int sr = sizeof(si_relay),recv_len;
    char buf[BUFSIZE];
     
    int socket_relay;
    //create a UDP socket
    if ((socket_relay=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
     
    // zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));
     
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(SERVER_PORT);
    si_me.sin_addr.s_addr = inet_addr(SERVER_IP);
     
    //bind socket to port
    if( bind(socket_relay , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
    {
        die("bind");
    }
     
    int basePointer=0;
    int inBuffer[WINDOWSIZE];
    char bufferArr[WINDOWSIZE][PACKET_SIZE+1];
    memset(bufferArr, 0 ,sizeof(bufferArr));
    memset(inBuffer, 0 ,sizeof(inBuffer));

    //keep listening for data

    int lastPacketSeq=INT_MAX;
    while(1)
    {         

    	while(inBuffer[basePointer%WINDOWSIZE]){
    		fwrite(bufferArr[basePointer%WINDOWSIZE], sizeof(char), strlen(bufferArr[basePointer%WINDOWSIZE]), myFile);
    		if(basePointer>lastPacketSeq)break;
    		inBuffer[basePointer%WINDOWSIZE]=0;
    		basePointer++;
    	}
    	if(basePointer>lastPacketSeq)break;

        //try to receive some data, this is a blocking call
        sr=sizeof(si_relay);
        if ((recv_len = recvfrom(socket_relay, buf, BUFSIZE, 0, (struct sockaddr *) &si_relay, &sr)) == -1)
        {
            die("recvfrom()");
        }
        

        // TODO - either buffer the packet, or ack it

        packet * newPacket = badUnSerialize(buf);
        
        int theSeqNum=newPacket->seqNo/PACKET_SIZE;

		if(theSeqNum<basePointer)continue;        

        newPacket->data[newPacket->size]=0;

        if(newPacket->lastPacket){
        	lastPacketSeq=theSeqNum;
        }

        inBuffer[theSeqNum%WINDOWSIZE]=1;
        strcpy(bufferArr[theSeqNum%WINDOWSIZE],newPacket->data);
        bufferArr[theSeqNum%WINDOWSIZE][newPacket->size]=0;
        fprintf(stderr,"Received packet from %s:%d\n", inet_ntoa(si_relay.sin_addr), ntohs(si_relay.sin_port));
        fprintf(stderr,"seqNo: %d, size %d, isLast %d\n",newPacket->seqNo, newPacket->size, newPacket->lastPacket);

        packet * ackPacket = getAckPacket(newPacket);

        if (sendto(socket_relay, badSerialize(ackPacket), BUFSIZE, 0, (struct sockaddr*) &si_relay, sr) == -1)
        {
            die("sendto()");
        }

    }
    fclose(myFile);
    close(socket_relay);
}