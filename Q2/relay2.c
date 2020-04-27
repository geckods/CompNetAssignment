// 2017A7PS1176P
// ABHINAV RAMACHANDRAN
// Submitted on 27.4.20

#include "data.h"
#include "functions.c"

int main(){
	// RELAY2 should receive from client, add possible delay, possible packet drop, and transmit to server
	// it should also receive acks from server and simply send them to client

	int clientSocket, serverSocket;

	srand(time(0));

    struct sockaddr_in si_me_client, si_client, si_me_server, si_server;
    int si_me_client_len, si_client_len, si_me_server_len, si_server_len;
    si_me_client_len= si_client_len= si_me_server_len= si_server_len=sizeof(struct sockaddr_in);

    //create a UDP socket
    if ((clientSocket=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
     
    // zero out the structure
    memset((char *) &si_me_client, 0, sizeof(si_me_client));
     
    si_me_client.sin_family = AF_INET;
    si_me_client.sin_port = htons(R2_PORT); //using 2's port
    si_me_client.sin_addr.s_addr = htonl(INADDR_ANY);
     
    //bind socket to port
    if( bind(clientSocket , (struct sockaddr*)&si_me_client, sizeof(si_me_client) ) == -1)
    {
        die("bind");
    }

    //create a UDP socket
    if ((serverSocket=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
    
    si_server.sin_family = AF_INET;
    si_server.sin_port = htons(SERVER_PORT);
    si_server.sin_addr.s_addr = inet_addr(SERVER_IP);

    fd_set readfds;

	int myNfds=max(clientSocket,serverSocket)+1;


	char buffer[BUFSIZE+1];

    TimeVal timeoutTime;
    TimeVal sleepTime;
    sleepTime.tv_sec=0;

    int recv_len, send_len;

    while(1){

    	si_server_len=sizeof(si_server);
    	si_client_len=sizeof(si_client);

	    FD_ZERO(&readfds);
	    FD_SET(clientSocket, &readfds);
	    FD_SET(serverSocket, &readfds);
	    timeoutTime.tv_sec=RELAYTIMEOUT;
	    timeoutTime.tv_usec=0;

	    if(select(myNfds,&readfds,NULL,NULL,&timeoutTime)==0){
	    	loggerMessage(RELAY2, TIMEOUT, get_current_time(), ACK, -1, RELAY2, RELAY2);
	    	// fprintf(stderr, "TIMED OUT!\n");
	    	return 0;
	    }


	    if(FD_ISSET(clientSocket, &readfds)){
	    	// read from clientSocket, send to serverSocket
	    	recv_len = recvfrom(clientSocket, buffer, BUFSIZE, 0, (struct sockaddr *)&si_client, &si_client_len);
	    	if(recv_len<0){
	            die("recvfrom()");
	    	}
	    	buffer[recv_len]=0;

	    	// DELAY
	    	sleepTime.tv_usec=rand()%MAXDELAY;
	    	select(0, NULL, NULL, NULL, &sleepTime);

	    	loggerMessage(RELAY2, RECV, get_current_time(), DATA, badUnSerialize(buffer)->seqNo, CLIENT, RELAY2);


	        if(!dropPacket()){
		        if (sendto(serverSocket, buffer, recv_len, 0, (struct sockaddr*) &si_server, si_server_len) == -1)
		        {
		            die("sendto()");
		        }	        	
		    	loggerMessage(RELAY2, SEND, get_current_time(), DATA, badUnSerialize(buffer)->seqNo, RELAY2, SERVER);
	        }
	        else{
	        	// PACKET DROP
		    	loggerMessage(RELAY2, DROP, get_current_time(), DATA, badUnSerialize(buffer)->seqNo, RELAY2, RELAY2);
	        }
	    }

    	si_server_len=sizeof(si_server);
    	si_client_len=sizeof(si_client);


	    if(FD_ISSET(serverSocket, &readfds)){
	    	// read from server, send to client
	    	recv_len = recvfrom(serverSocket, buffer, BUFSIZE, 0, (struct sockaddr *)&si_server, &si_server_len);
	    	if(recv_len<0 ){
	            die("recvfrom()");
	    	}
	    	buffer[recv_len]=0;

	    	if(!(ntohs(si_server.sin_port)==SERVER_PORT)){
	    		continue;
	    	}
	    	loggerMessage(RELAY2, RECV, get_current_time(), ACK, badUnSerialize(buffer)->seqNo, SERVER, RELAY2);

	        if (sendto(serverSocket, buffer, recv_len, 0, (struct sockaddr*) &si_client, si_client_len) == -1)
	        {
	            die("sendto()");
	        }
	    	loggerMessage(RELAY2, SEND, get_current_time(), ACK, badUnSerialize(buffer)->seqNo, RELAY2,CLIENT);

	    }
    }
    // WRAPPING UP
    close(clientSocket);
    close(serverSocket);    
}