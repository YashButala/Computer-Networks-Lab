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
#include <fcntl.h>
#include <sys/stat.h>
#define MAXLINE 10

int main(){
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    int addrlen = sizeof(servaddr);

    // Create socket file descriptor

    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    
	// carry out the sanity check for the socket
	if (sockfd < 0) { 
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }else{
        printf("A new socket is created \n");
    }

    // Now we need to attach the socket to a port 
    //fill the memory at servaddr and cliaddr with zero
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr,0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(8181); 


    // Bind the socket with the server address
    if (bind(sockfd, (const struct sockaddr * )&servaddr, sizeof(servaddr)) < 0){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // if no error then the server is running
    printf("\nServer is running\n");

    // After binding write the listen function
    int confirmation = listen(sockfd, 3);
    if (confirmation < 0){ 
        perror("listen failed"); 
        exit(EXIT_FAILURE); 
    }

    printf("\nWaiting for the client\n");
    
    // accept the packets
    int new_socket = accept(sockfd, (struct sockaddr *)&servaddr, (socklen_t*)&addrlen);
    if (new_socket < 0) 
    { 
        perror("accept failed"); 
        exit(EXIT_FAILURE); 
    } 
    
    printf("\nAccepted the client\n");
    
    // send or recieve should be implemented here
    int n;
    socklen_t len;
    char filename[MAXLINE] = "";
    len = sizeof(cliaddr);


    // recieve should be in a loop
    while(1){
        char temp[MAXLINE] = "";
        int n = recv(new_socket, (char *)temp, MAXLINE, 0);
		// error handling
		
		if(n <= 0){
			printf("\nConnection Closed\n");
			close(new_socket);
			close(sockfd);
			exit(-1);
		}
        
        if(temp[n-1] == '$'){
            temp[n-1] = '\0';
            strcat(filename, temp);
            break;
        }else{
            strcat(filename, temp);
        }
  
    }

    // filename is received, server will open the file and send the chunks
    // of data in strings, marking the end of a chunk by a '$'
    
    printf("\nCLIENT: %s\n", filename);
	
	//buffer contains the filename, read the file
	
    int file;
    file = open(filename, O_RDONLY , 0); 
	if(file == -1)
    {
        printf("File not found\n");
		close(new_socket);
		close(sockfd);
        exit(0);
    }
    else
    {
        printf("File open successful...\n");
    }
    while(1)
    {
        char temp[MAXLINE];
  //      int n = recv(file, (char *)temp, MAXLINE, 0);
        int len = read (file, temp, MAXLINE-1); 
        //printf("%d\n",len);
        if(len<=0)
            break;
        temp[len] = '\0';
        //write(1, temp, MAXLINE);
        //printf("%s\n",temp);
       // fflush(stdout);
        send(new_socket,(const char*)temp,strlen(temp),0);
    }   
    close(new_socket);
    close(sockfd);
    return 0;
} 
