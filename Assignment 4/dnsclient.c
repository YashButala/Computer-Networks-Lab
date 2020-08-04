// implementation of the dns client
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#define MAXLINE 200
  
int main() { 
    int sockfd; 
    struct sockaddr_in servaddr; 
  
    // Creating socket file descriptor 
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if ( sockfd < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
  
    memset(&servaddr, 0, sizeof(servaddr)); 
      
    // Server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(8101); 
    servaddr.sin_addr.s_addr = INADDR_ANY;
     
    int n;
    socklen_t len; 
    char *host_name = "www.iitkgp.ac.in"; 

    // send the host name to the server
      
    sendto(sockfd, (const char *)host_name, strlen(host_name), 0, 
			(const struct sockaddr *) &servaddr, sizeof(servaddr)); 
    printf("\nClient: www.iitkgp.ac.in\n"); 

    // receive the ip address from the server

    char ip_address[MAXLINE]; 
 
    len = sizeof(servaddr);
    n = recvfrom(sockfd, (char *)ip_address, MAXLINE, 0, 
			( struct sockaddr *) &servaddr, &len); 
    ip_address[n] = '\0'; 
    printf("\nThe IP Address corresponding to the hostname www.iitkgp.ac.in is %s\n\n", ip_address);

    close(sockfd); 
    return 0; 
}
