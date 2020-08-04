// A Simple Client Implementation
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#define MAXLINE 1024  
int main() 
{ 
    int sockfd; 
    struct sockaddr_in servaddr,cliaddr; 
  
    // Creating socket file descriptor 
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if ( sockfd < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
    memset(&cliaddr, 0, sizeof(cliaddr));
    memset(&servaddr, 0, sizeof(servaddr)); 
      
    // Server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(8181); 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    char serverMessage[50];  
    int n;
    socklen_t len; 
    char *filename = "file.txt";   
    sendto(sockfd, (const char *)filename, strlen(filename), 0,(const struct sockaddr *) &servaddr, sizeof(servaddr)); 


    n = recvfrom(sockfd, (char *)serverMessage, MAXLINE, 0, (struct sockaddr *) &servaddr, &len);
    serverMessage[n] = '\0';
    char not_found[30] = "NOTFOUND ";
    strcat(not_found,filename);
    //if message returned is not found then exit with appropriate message
    if(!strcmp(serverMessage,not_found))   
    {
        close(sockfd);
        printf("File %s not found\n", filename);
        return 0;
    }

    printf("SERVER : %s",serverMessage);
    FILE *fptr;
    fptr = fopen("rec.txt","w");
 //   printf("file opened\n");
//    printf("first compare %d\n", strcmp(serverMessage,"END"));
    int i = 1;
    //client runs till END is received from server
    while(strcmp(serverMessage,"END"))
    {
        char word[40] = "Word";
        char integer_string[32]; 
        sprintf(integer_string, "%d", i++);
        strcat(word, integer_string);
        printf("CLIENT : %s\n\n",word );
        //send Wordi to server 
        sendto(sockfd, (const char *)word, strlen(word), 0,(const struct sockaddr *) &servaddr, sizeof(servaddr));
        //receive the ith word
        n = recvfrom(sockfd, (char *)serverMessage, MAXLINE, 0, (struct sockaddr *) &servaddr, &len);
        serverMessage[n] = '\0';
        printf("SERVER : %s\n\n",serverMessage);
        //print wordi in new file
        fprintf(fptr, "%s\n", serverMessage);
         
    }
    close(sockfd);
    return 0; 
} 
