// 2017A7PS1176P
// ABHINAV RAMACHANDRAN
// Submitted on 27.4.20

#include "data.h"
#include "functions.c"

int main(int argc, char* argv[]){

	// randomness for packet drops
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



	struct sockaddr_in serverAddress1, clientAddress1,serverAddress2, clientAddress2;


	// connection number 1
	/*CREATE A TCP SOCKET */
	int serverSocket1 = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket1 < 0) { 
		printf ("Error while server socket creation"); exit (0); 
	}
	/*CONSTRUCT LOCAL ADDRESS STRUCTURE*/
	memset (&serverAddress1, 0, sizeof(serverAddress1));
	serverAddress1.sin_family = AF_INET;
	serverAddress1.sin_port = htons(PORT1);
	serverAddress1.sin_addr.s_addr = htonl(INADDR_ANY);
	int temp = bind(serverSocket1, (struct sockaddr*) &serverAddress1,sizeof(serverAddress1));
	if (temp < 0)
	{ printf ("Error while binding\n");
		exit (0);
	}
	int temp1 = listen(serverSocket1, MAXPENDING);
	if (temp1 < 0)
	{ printf ("Error in listen");
		exit (0);
	}
	char msg[BUFSIZE];
	int clientLength1 = sizeof(clientAddress1);



	// connection number 2
	/*CREATE A TCP SOCKET*/
	int serverSocket2 = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket2 < 0) { 
		printf ("Error while server socket creation"); exit (0); 
	}
	/*CONSTRUCT LOCAL ADDRESS STRUCTURE*/
	memset (&serverAddress2, 0, sizeof(serverAddress2));
	serverAddress2.sin_family = AF_INET;
	serverAddress2.sin_port = htons(PORT2);
	serverAddress2.sin_addr.s_addr = htonl(INADDR_ANY);
	temp = bind(serverSocket2, (struct sockaddr*) &serverAddress2,sizeof(serverAddress2));
	if (temp < 0)
	{ printf ("Error while binding\n");
		exit (0);
	}
	temp1 = listen(serverSocket2, MAXPENDING);
	if (temp1 < 0)
	{ printf ("Error in listen");
		exit (0);
	}


	int clientSocket1 = accept (serverSocket1, (struct sockaddr*) &clientAddress1, &clientLength1);
	if (clientLength1 < 0) {printf ("Error in client socket"); exit(0);}
	int clientLength2 = sizeof(clientAddress2);
	int clientSocket2 = accept (serverSocket2, (struct sockaddr*) &clientAddress2, &clientLength2);
	if (clientLength2 < 0) {printf ("Error in client socket"); exit(0);}

	int expectedSeqNumber=0;

	int inBufferSeqNumber=-1;
	char dataBuffer[PACKET_SIZE+1]={0};

	char gottenPacket[BUFSIZE];
	packet *sendPacket;

	packet *p1,*p2;

	int toAck=1;
	// used to not ACK if dropping a packet

	int lastAck=INT_MAX;

	while(1){

		// if you can move the buffer to file, do so
		if(inBufferSeqNumber==expectedSeqNumber){
			inBufferSeqNumber=-1;
			expectedSeqNumber+= fwrite(dataBuffer, sizeof(char), strlen(dataBuffer), myFile);
			memset(dataBuffer,0,sizeof(dataBuffer));
		}

		if(expectedSeqNumber>lastAck)break;

		toAck=1;
		struct pollfd pfds[2]; 
	    pfds[0].fd = clientSocket1;
	    pfds[0].events = POLLIN; // Tell me when ready to read
	    pfds[1].fd = clientSocket2;
	    pfds[1].events = POLLIN; // Tell me when ready to read

	    int num_events = poll(pfds, 2, -1);

	    if(pfds[0].revents&POLLIN){
	    	// received data on channel 1

	    	temp = recv(clientSocket1, gottenPacket, BUFSIZE,0);

	    	p1=badUnSerialize(gottenPacket);

	    	p1->data[PACKET_SIZE]='\0';

	    	if(dropPacket()){
	    		// PACKET DROP
	    	}
	    	else{
		    	// write to outputfile
		    	if(p1->seqNo==expectedSeqNumber){
		    		expectedSeqNumber+=fwrite(p1->data, sizeof(char),p1->size, myFile);
		    	}
		    	else if (p1->seqNo>expectedSeqNumber){
		    		if(inBufferSeqNumber==-1){
		    			inBufferSeqNumber=p1->seqNo;
		    			strcpy(dataBuffer,p1->data);
		    		}
		    		else{
		    			// DROP PACKETS WHEN BUFFER IS FULL AND OUT OF ORDER
		    			toAck=0;
		    		}
		    	}

		    	if(toAck){
			    	pcktPrint(0, p1->seqNo, p1->size, 1);
			    	// send ack
			    	sendPacket = getAckPacket(p1);
			    	send(clientSocket1,badSerialize(sendPacket),sizeof(packet),0);
					ackPrint(1,sendPacket->seqNo,1);

			    	if(p1->lastPacket)lastAck=p1->seqNo;		    		
		    	}

	    	}
	    }
	    toAck=1;
	    if(pfds[1].revents&POLLIN){


	    	temp = recv(clientSocket2, gottenPacket, BUFSIZE,0);
	    	p2=badUnSerialize(gottenPacket);

	    	p2->data[PACKET_SIZE]='\0';

	    	if(dropPacket()){
	    		// packet drop

	    	}
	    	else{
		    	// write to outputfile
		    	if(p2->seqNo==expectedSeqNumber){
		    		expectedSeqNumber+=fwrite(p2->data, sizeof(char),p2->size, myFile);
		    	}
		    	else if (p2->seqNo>expectedSeqNumber){
		    		if(inBufferSeqNumber==-1){
		    			inBufferSeqNumber=p2->seqNo;
		    			strcpy(dataBuffer,p2->data);
		    		}
		    		else{
		    			// drop
		    			toAck=0;
		    		}
		    	}

		    	if(toAck){
			    	pcktPrint(0, p2->seqNo, p2->size, 2);		    		
			    	// send ack
			    	sendPacket = getAckPacket(p2);
			    	send(clientSocket2,badSerialize(sendPacket),sizeof(packet),0);
					ackPrint(2,sendPacket->seqNo,2);

			    	if(p2->lastPacket)lastAck=p1->seqNo;		    		
		    	}

	    	}
	    	
	    }
	}

	// wrapping up
	fclose(myFile);
	close(serverSocket1);
	close(serverSocket2);
	close(clientSocket1);
	close(clientSocket2);

}