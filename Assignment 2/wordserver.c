// server implementation

// include the required libraries

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#define MAXLINE 1024

int main(){
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;

    // Create socket file descriptor

    sockfd = socket(AF_INET, SOCK_DGRAM, 0); 
    printf("A new socket is created \n");
    
	// carry out the sanity check for the socket
	if (sockfd < 0) { 
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    //fill the memory at servaddr and cliaddr with zero
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr,0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET; // set the address family to contain addresses of type IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY; // specifies input addresses hashed to in
    servaddr.sin_port = htons(8181); // host-to-network port


    // Bind the socket with the server address
    if (bind(sockfd, (const struct sockaddr * )&servaddr, sizeof(servaddr)) < 0){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // if no error then the server is running
    printf("\nServer is running\n");

    int n;
    socklen_t len;
    char filename[MAXLINE];

    len = sizeof(cliaddr);
    n = recvfrom(sockfd, (char *)filename, MAXLINE, 0, (struct sockaddr *) &cliaddr, &len);
    filename[n] = '\0';

	printf("\nCLIENT: %s\n", filename);
	
	//buffer contains the filename, read the file
	
	FILE *fptr = fopen(filename, "r");

	printf("\nServer has opend the file successfully\n");

	if(fptr == NULL){
		printf("\nSERVER: The file does not exist\nTerminating...\n");
		
		//send message not found to the client
		char notfound[30] = "NOTFOUND ";
		strcat(notfound, filename);
		sendto(sockfd, (const char* )notfound, strlen(notfound), 0, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
		close(sockfd);
		exit(0);
	}else{
		//read the file word by word, the first word must be hello
		char *temp;
		//fscanf(fptr, "%s", temp);
		
		size_t l = 0;
		getline(&temp, &l, fptr);
		// print the word on server display
		printf("\nSERVER: %s\n", temp);

		//send this word to client
		sendto(sockfd, (const char* )temp, strlen(temp), 0, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
		
		// now check if this word is END, if yes stop else scan next word and send that
		while(strcmp(temp, "END"))
		{
			printf("\nSERVER: Sent the message to the client, waiting for reply..\n");	
			
			char clientMessage[MAXLINE];
			n = recvfrom(sockfd, (char *)clientMessage, MAXLINE, 0, (struct sockaddr *) &cliaddr, &len);
			clientMessage[n] = '\0';
			printf("\nCLIENT: %s\n", clientMessage);
		
			// read the next word from the file
			fscanf(fptr, "%s", temp);
			
			// print the word on server display
			printf("\nSERVER: %s\n", temp);
			
			//send this word to client
			sendto(sockfd, (const char* )temp, strlen(temp), 0, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));

		}

	}

	printf("\nSERVER: Terminating...\n");
	close(sockfd);
    return 0;

}
