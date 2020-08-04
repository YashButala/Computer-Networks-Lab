

// TCP Client program for bag of words fetch
#include <arpa/inet.h> 
#include <errno.h> 
#include <netinet/in.h> 
#include <signal.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <netdb.h> 
#include <fcntl.h>
// Change port and buffer size
#define PORT 8101
#define MAXLINE 1024 

// This is individual word size for printing each word
#define WORDSIZE 3000

int main() 
{ 
    int sockfd; 
    char buffer[MAXLINE];  
    struct sockaddr_in servaddr; 
  
    int n, len; 
    // Creating TCP socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
        printf("Failed to create socket!\n"); 
        exit(0); 
    } 
    char in_dir[5];
    printf("Enter the directory you want to get images from :\n");
    scanf("%s",in_dir);
    // Setting 
    memset(&servaddr, 0, sizeof(servaddr)); 
  
    // Server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(PORT); 
    servaddr.sin_addr.s_addr = INADDR_ANY; 

  
    if (connect(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) < 0) { 
        printf("Connection Failed! \n");
        exit(0);
    } 
    printf("%s\n", in_dir);
    send(sockfd,(const char*)in_dir,strlen(in_dir),0);
    int count=0,size1,wl=0;
    char singleword[WORDSIZE];
    while(1)
    {
        if((size1=recv(sockfd,buffer,sizeof(buffer),0))<=0)
        {
            printf("Closing Client\n");
            break;
        }

        buffer[size1]='\0';
        if(strcmp(buffer,"END")==0)
        {
            printf("Received all the images\n");;
            break;
        }        
        printf("%s\n",buffer );
      
        int fd = open(buffer, O_WRONLY | O_CREAT | O_TRUNC, 0644); 
        while(1)
        {
         //   printf("here\n");
            char temp[WORDSIZE] = "";
            int n = recv(sockfd, (char *)temp, WORDSIZE, 0);
                
            if(strcmp(temp,"AND")==0)
            {
                break;
            }
            else
            {
      //          printf("%s\n",temp);
        //        printf("hello %d\n",n);
                temp[n] = '\0';
                write(fd, temp, strlen(temp));
            }
      
        }
        close(fd);
            
    }
//    printf("Number of words received: %d\n",count);
    close(sockfd); 
} 