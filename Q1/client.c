#include <data.h>
#include <functions.c>

int main(int argc, char* argv[]){

	char* fileName = "input.txt";
	if(argc>2){
		perror("Expected one or less command line argument, got more.");
		return -1;
	}
	else if (argc==2){
		fileName=argv[1];
	}

	// Open file
	FILE* myFile = fopen(fileName,"r");
	if(myFile==NULL){
		fprintf(stderr,"Could not open file %s: ", fileName);
		return -1;
	}

	// Initialize some file pointers
	fseek(myFile, 0L, SEEK_END);
	fileSize = ftell(myFile);
	rewind(myFile);
	fileOffset=0;


// TESTING getNextBlock
	// char myBuffer[PACKET_SIZE+1];
	// int done=0;
	// while(!done){
	// 	getNextBlock(myFile, myBuffer, &done);
	// 	puts(myBuffer);
	// 	printf("\n\n\n%d %d %lu\n\n\n\n", fileOffset, done, strlen(myBuffer));
	// }
	// return 0;


	// Setup and connext sockets
	int socket1 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(socket1<0){
		perror("Error in opening socket1");
		return -1;
	}
	
	int socket2 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(socket2<0){
		perror("Error in opening socket2");
		return -1;
	}

	struct sockaddr_in serverAddr1;
	memset (&serverAddr1,0,sizeof(serverAddr1));
	serverAddr1.sin_family = AF_INET;
	serverAddr1.sin_port = htons(PORT1); 
	serverAddr1.sin_addr.s_addr = inet_addr("127.0.0.1");

	struct sockaddr_in serverAddr2;
	memset (&serverAddr2,0,sizeof(serverAddr2));
	serverAddr2.sin_family = AF_INET;
	serverAddr2.sin_port = htons(PORT2); 
	serverAddr2.sin_addr.s_addr = inet_addr("127.0.0.1");

	int c = connect (socket1, (struct sockaddr*) &serverAddr1, sizeof(serverAddr1));
	if (c < 0)
	{
		perror("Error while establishing connection with socket 1");
		exit (0);
	}

	c = connect (socket2, (struct sockaddr*) &serverAddr2, sizeof(serverAddr2));
	if (c < 0)
	{
		perror("Error while establishing connection with socket 2");
		exit (0);
	}

	// setup fd_set
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(socket1, &readfds);
	FD_SET(socket2, &readfds);
	int myNfds=max(socket1,socket2)+1;

	// setup times
	TimeVal sendTime1;
	TimeVal sendTime2;
	TimeVal waitTime;
	TimeVal currTime;
	TimeVal retransmissionTime;
	TimeVal elapsedTime;
	TimeVal olderSendTime;
	retransmissionTime.tv_sec=RETRANSMISSION_TIME_SEC;
	retransmissionTime.tv_usec=RETRANSMISSION_TIME_USEC;
	int oneIsOlder = 1;
	sendTime1.tv_sec=sendTime2.tv_sec=INT_MAX;

	int selectReturn;

	packet *p1=NULL,*p2=NULL;

	int done=0;
	int sentLastPacket=0;
	while(!done){
		// set up the select wait time
		gettimeofday(&currTime, NULL);
		if(oneIsOlder){
			olderSendTime=sendTime1;
		}
		else{
			olderSendTime=sendTime2;
		}

		// fprintf(stderr, "SENDTIME:%ld %ld\n",olderSendTime.tv_sec,olderSendTime.tv_usec);
		// fprintf(stderr, "CURRTIME:%ld %ld\n",currTime.tv_sec,currTime.tv_usec);

		if(olderSendTime.tv_sec==INT_MAX || isOlder(olderSendTime, time_diff(currTime,retransmissionTime))){
			// fprintf(stderr, "MESSED_UP\n");
			waitTime.tv_sec=0;
			waitTime.tv_usec=0;
		}
		else{
			elapsedTime=time_diff(currTime,olderSendTime);
			waitTime=time_diff(retransmissionTime,elapsedTime);
		}


		selectReturn = select(myNfds, &readfds, NULL, NULL, &waitTime);
		if(selectReturn==0){
			// TIMEOUT OCCURED
			if(oneIsOlder){
				// one timed out
				if(p1==NULL){
					p1=getNextPacket(myFile,1);
				}
				sentLastPacket=p1->lastPacket;

				int bytesSent = send (socket1, badSerialize(p1), sizeof(packet), 0);
				pcktPrint(1,p1->seqNo, p1->size,1);

				if (bytesSent != sizeof(packet))
				{ printf("Error while sending the message channel 1");
					exit(0);
				}
				// printf ("Data Sent\n");


				// fprintf(stderr, "HIHI1");

				gettimeofday(&sendTime1,NULL);
				oneIsOlder=0;
			}
			else{
				// two timed out
				if(p2==NULL){
					p2=getNextPacket(myFile,2);
				}
				sentLastPacket=p2->lastPacket;

				int bytesSent = send (socket2, badSerialize(p2), sizeof(packet), 0);
				pcktPrint(1,p2->seqNo, p2->size,2);
				if (bytesSent != sizeof(packet))
				{ printf("Error while sending the message channel 2");
					exit(0);
				}
				// printf ("Data Sent\n");

				gettimeofday(&sendTime2,NULL);
				oneIsOlder=1;
			}
		}
		else{
			if(FD_ISSET(socket1, &readfds)){
				// GOT ACK ON 1
				char recvBuffer[BUFSIZE+1];
				int bytesRecvd = recv (socket1, recvBuffer, BUFSIZE, 0);
				if (bytesRecvd < 0)
				{ printf ("Error while receiving data from server");
					exit (0);
				}
				recvBuffer[bytesRecvd] = '\0';



				packet *ackPacket = badUnSerialize(recvBuffer);
				ackPrint(0,ackPacket->seqNo,1);
				if(ackPacket->ack && ackPacket->seqNo==p1->seqNo){

					if(ackPacket->lastPacket)break;

					if(!sentLastPacket){
						p1=getNextPacket(myFile,1);
						sentLastPacket=p1->lastPacket;

						int bytesSent = send (socket1, badSerialize(p1), sizeof(packet), 0);
						pcktPrint(1,p1->seqNo, p1->size,1);
						if (bytesSent != sizeof(packet))
						{ printf("Error while sending the message channel 1");
							exit(0);
						}						
					}
					// printf ("Data Sent\n");

					gettimeofday(&sendTime1, NULL);
					oneIsOlder = 0;
				}
				else{
					// TODO (what to do?)
				}
			}
			if (FD_ISSET(socket2, &readfds)){
				// fprintf(stderr, "HIHIHI");
				// GOT ACK ON 2
				char recvBuffer[BUFSIZE+1];
				int bytesRecvd = recv (socket2, recvBuffer, BUFSIZE, 0);
				if (bytesRecvd < 0)
				{ printf ("Error while receiving data from server");
					exit (0);
				}
				recvBuffer[bytesRecvd] = '\0';

				packet *ackPacket = badUnSerialize(recvBuffer);

				ackPrint(0,ackPacket->seqNo,2);

				if(ackPacket->ack && ackPacket->seqNo==p2->seqNo){

					if(ackPacket->lastPacket)break;

					if(!sentLastPacket){
						p2=getNextPacket(myFile,2);
						sentLastPacket=p2->lastPacket;

						int bytesSent = send (socket2, badSerialize(p2), sizeof(packet), 0);
						pcktPrint(1,p2->seqNo, p2->size,2);
						if (bytesSent != sizeof(packet))
						{ printf("Error while sending the message channel 2");
							exit(0);
						}						
					}
					// printf ("Data Sent\n");

					gettimeofday(&sendTime2, NULL);
					oneIsOlder = 1;
				}
				else{
					// TODO (what to do?)
				}
			}
		}

		FD_ZERO(&readfds);
		FD_SET(socket1, &readfds);
		FD_SET(socket2, &readfds);
	}

	close(socket1);
	close(socket2);
}