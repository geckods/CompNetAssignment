#include "q2_data.h"
#include "q2_functions.c"

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

	int send_base=0;
	int windowPointer=0;

	TimeVal sendTime[WINDOWSIZE];
	int isAcked[WINDOWSIZE];
	packet* inWindow[WINDOWSIZE];

	for(int i=0;i<WINDOWSIZE;i++){
		sendTime[i].tv_sec=INT_MAX;
		sendTime[i].tv_usec=INT_MAX;
		isAcked[i]=0;
	}

	int fileDone=0;

    struct sockaddr_in si_relay1, si_relay2;
    int socket_relay1,socket_relay2, sr1,sr2;
    sr1=sizeof(si_relay1);
    sr2=sizeof(si_relay2);
    
    char buf[BUFSIZE+1];
    char message[PACKET_SIZE+1];
 
    if ((socket_relay1=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
        die("socket");
    }
 

    if ((socket_relay2=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
        die("socket");
    }
 

    memset((char *) &si_relay1, 0, sizeof(si_relay1));
    si_relay1.sin_family = AF_INET;
    si_relay1.sin_port = htons(R1_PORT);
    si_relay1.sin_addr.s_addr = inet_addr(RELAY1_IP);

    memset((char *) &si_relay2, 0, sizeof(si_relay2));
    si_relay2.sin_family = AF_INET;
    si_relay2.sin_port = htons(R2_PORT);
    si_relay2.sin_addr.s_addr = inet_addr(RELAY2_IP);


	TimeVal waitTime;
	TimeVal currTime;
	TimeVal retransmissionTime;
	TimeVal elapsedTime;
	retransmissionTime.tv_sec=RETRANSMISSION_TIME_SEC;
	retransmissionTime.tv_usec=RETRANSMISSION_TIME_USEC;

	int currOldestTimer=0;

	fd_set readfds;

	int myNfds=max(socket_relay1,socket_relay2)+1;

	while(1){
	    memset((char *) &si_relay1, 0, sizeof(si_relay1));
	    si_relay1.sin_family = AF_INET;
	    si_relay1.sin_port = htons(R1_PORT);
	    si_relay1.sin_addr.s_addr = inet_addr(RELAY1_IP);

	    memset((char *) &si_relay2, 0, sizeof(si_relay2));
	    si_relay2.sin_family = AF_INET;
	    si_relay2.sin_port = htons(R2_PORT);
	    si_relay2.sin_addr.s_addr = inet_addr(RELAY2_IP);

		sr1=sr2=sizeof(si_relay1);
		if(!fileDone && windowPointer-send_base < WINDOWSIZE){
			// send next packet, set fileDone if need be
			// ALSO update sendTime
			// ALSO set isAcked to false
			// ALSO put packet in inWindow

			if(windowPointer%2==0){
				inWindow[windowPointer%WINDOWSIZE]=getNextPacket(myFile,1);
				//send the message
				// fprintf(stderr,"SENDING %d\n",inWindow[windowPointer%WINDOWSIZE]->seqNo);
		        if (sendto(socket_relay1, badSerialize(inWindow[windowPointer%WINDOWSIZE]), BUFSIZE , 0 , (struct sockaddr *) &si_relay1, sr1)==-1)
		        {
		            die("sendto()");
		        }
			}
			else{
				inWindow[windowPointer%WINDOWSIZE]=getNextPacket(myFile,2);
				//send the message
		        if (sendto(socket_relay2, badSerialize(inWindow[windowPointer%WINDOWSIZE]), BUFSIZE , 0 , (struct sockaddr *) &si_relay2, sr2)==-1)
		        {
		            die("sendto()");
		        }				
			}

			if(inWindow[windowPointer%WINDOWSIZE]->lastPacket)fileDone=1;
	        gettimeofday(&sendTime[windowPointer%WINDOWSIZE], NULL);
	        isAcked[windowPointer%WINDOWSIZE]=0;
			windowPointer++;
			continue;
		}

		gettimeofday(&currTime,NULL);

		if(isOlder(sendTime[currOldestTimer], time_diff(currTime, retransmissionTime))){
			waitTime.tv_sec=0;
			waitTime.tv_usec=0;
		}
		else{
			elapsedTime=time_diff(currTime,sendTime[currOldestTimer]);
			waitTime=time_diff(retransmissionTime,elapsedTime);
		}

		FD_ZERO(&readfds);
		FD_SET(socket_relay1, &readfds);
		FD_SET(socket_relay2, &readfds);

		int selectReturn = select(myNfds, &readfds, NULL, NULL, &waitTime);
		if(selectReturn==0){
			// timeout - retransmit
			if(currOldestTimer%2==0){
				//send the message
		        if (sendto(socket_relay1, badSerialize(inWindow[currOldestTimer%WINDOWSIZE]), BUFSIZE , 0 , (struct sockaddr *) &si_relay1, sr1)==-1)
		        {
		            die("sendto()");
		        }
			}
			else{
				//send the message
		        if (sendto(socket_relay2, badSerialize(inWindow[currOldestTimer%WINDOWSIZE]), BUFSIZE , 0 , (struct sockaddr *) &si_relay2, sr2)==-1)
		        {
		            die("sendto()");
		        }				
			}
	        gettimeofday(&sendTime[currOldestTimer%WINDOWSIZE], NULL);

	        TimeVal oldestTime;oldestTime.tv_sec=INT_MAX;
	        for(int i=0;i<WINDOWSIZE;i++){
	        	if(isAcked[i])continue;
	        	if(isOlder(sendTime[i], oldestTime)){
	        		oldestTime=sendTime[i];
	        		currOldestTimer=i;
	        	}
	        }

		}
		else{
			// fprintf(stderr, "HIHIHI");

			if(FD_ISSET(socket_relay1, &readfds)){
				// got ack
		        if (recvfrom(socket_relay1, buf, BUFSIZE, 0, (struct sockaddr *) &si_relay1, &sr1) == -1)
		        {
		            die("recvfrom()");
		        }

		        packet* tempPacket = badUnSerialize(buf);
		        if(!tempPacket->ack){
		        	die("dead");
		        }

		        if(tempPacket->lastPacket)break;

		        int hasBeenAcked=(tempPacket->seqNo/PACKET_SIZE);

		        // fprintf(stderr, "GOT ACK:%d , wp %d b %d\n", hasBeenAcked, windowPointer, send_base);

		        isAcked[hasBeenAcked%WINDOWSIZE]=1;
		        // if(send_base==hasBeenAcked)send_base++;
		        while(send_base<windowPointer && isAcked[send_base%WINDOWSIZE])send_base++;

		        if(hasBeenAcked%WINDOWSIZE==currOldestTimer){
    		        TimeVal oldestTime;oldestTime.tv_sec=INT_MAX;
			        for(int i=0;i<WINDOWSIZE;i++){
			        	if(isAcked[i])continue;
			        	if(isOlder(sendTime[i], oldestTime)){
			        		oldestTime=sendTime[i];
			        		currOldestTimer=i;
			        	}
			        }
		        }

			}

			if(FD_ISSET(socket_relay2, &readfds)){
				// got ack

		        if (recvfrom(socket_relay2, buf, BUFSIZE, 0, (struct sockaddr *) &si_relay2, &sr2) == -1)
		        {
		            die("recvfrom()");
		        }

		        packet* tempPacket = badUnSerialize(buf);
		        if(!tempPacket->ack){
		        	die("dead");
		        }

		        if(tempPacket->lastPacket)break;
		        int hasBeenAcked=(tempPacket->seqNo/PACKET_SIZE);

		        // fprintf(stderr, "GOT ACK:%d\n", hasBeenAcked);


		        isAcked[hasBeenAcked%WINDOWSIZE]=1;
		        
		        while(send_base<windowPointer && isAcked[send_base%WINDOWSIZE])send_base++;

		        if(hasBeenAcked%WINDOWSIZE==currOldestTimer){
    		        TimeVal oldestTime;oldestTime.tv_sec=INT_MAX;
			        for(int i=0;i<WINDOWSIZE;i++){
			        	if(isAcked[i])continue;
			        	if(isOlder(sendTime[i], oldestTime)){
			        		oldestTime=sendTime[i];
			        		currOldestTimer=i;
			        	}
			        }
		        }

			}
		}
	}
}