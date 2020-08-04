// Server program for both UDP and TCP clients
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
#include <sys/stat.h>
#include <dirent.h>
// Change port and buffer size
#define PORT 8101
#define MAXLINE 1024 
#define WORDSIZE 1024

// Start of main program
int main() 
{ 
    // Socket descriptors
    int tcpfd,udpfd; 
    char buffer[MAXLINE]; 
    socklen_t len;
    fd_set rs;      
    struct sockaddr_in cli_addr, serv_addr; 

    //************************************ TCP Socket ************************************//

    tcpfd = socket(AF_INET, SOCK_STREAM, 0); 
    if(tcpfd<0){
        printf("Socket creation failed!\n");
        exit(0);
    }

    memset(&serv_addr,0,sizeof(serv_addr)); 
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    serv_addr.sin_port = htons(PORT); 
 // 	setsockopt(2)
    // Binding to local address 
    int b = bind(tcpfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 
    if(b<0){
        printf("Bind failed-TCP!\n");
        exit(0);
    }
    listen(tcpfd, 10); 
  
    //************************************ UDP Socket ************************************//

    udpfd = socket(AF_INET, SOCK_DGRAM, 0); 
    b = bind(udpfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 
    if(b<0){
        printf("Bind failed-UDP!\n");
        exit(0);
    }
    // Set descriptors to zero initially 
    FD_ZERO(&rs); 
  
    // Get maximum of two descriptors to set the limit 
    int maxi = (tcpfd>udpfd)?tcpfd:udpfd+ 1; 
    DIR* FD;
    struct dirent* in_file;
    while(1) 
    { 
  
        // Set tcpfd and udpfd 
        FD_SET(tcpfd, &rs); 
        FD_SET(udpfd, &rs); 
  
        // Using select statement to select ready descriptor 
        select(maxi, &rs, NULL, NULL, NULL); 
  
        //************************************ TCP part ************************************//
        if (FD_ISSET(tcpfd, &rs)) 
        { 
            printf("\n\nTCP running!\n");
            len = sizeof(cli_addr); 
            int newfd = accept(tcpfd, (struct sockaddr*)&cli_addr, &len); 
            // Creating new process using fork
            if (fork() == 0) 
            { 
                // Child process
                close(tcpfd);  // Closing the original tcpfd in child process
                //receive dirname as in_dir
			    char  in_dir[5]="";
			    while(1)
			    {
			        char temp[5] = "";
			        int n = recv(newfd, (char *)temp, 5, 0);
			        
			        if(temp[n-1] == '$'){
			            temp[n-1] = '\0';
			            strcat(in_dir, temp);
			            break;
			        }else{
			            strcat(in_dir, temp);
			        }
			  
			    }
                char dir[50]="";
                strcat(dir,"image/");
                strcat(dir,in_dir);
                dir[9]='/';
            //    dir[9]='\0';
                printf("%s\n",dir );
             //   char * dir_a="image/im1";
                FD = opendir (dir);
			    if (NULL == FD) 
			    {
			        printf( "Error : Failed to open input directory - %s\n", strerror(errno));
		            send(newfd,(const char*)"END",strlen("END"),0); 
	            	close(newfd);	
	            	printf("TCP closed\n"); 
	            	exit(0); 

			    }
			    printf("Sub-directory found\n");
			    while ((in_file = readdir(FD))) 
			    {

			        if (!strcmp (in_file->d_name, "."))
			            continue;
			        if (!strcmp (in_file->d_name, ".."))    
			            continue;
			        char th_file[50] ="";
			        strcat(th_file,dir);
			        strcat(th_file,in_file->d_name);
			        int entry_file = open(th_file, O_RDONLY , 0);
			        printf("%s\n", in_file->d_name);
			        if (entry_file == -1)
			        {
			            printf( "Error : Failed to open entry file - %s\n", strerror(errno));
			            return 1;
			        } 
	           
	                printf("File %s opened! Reading Images!\n",in_file->d_name);
	                char temp[WORDSIZE];
	                char fname[10]="";
	                strcat(fname,in_file->d_name);
	                strcat(fname,"$");
	                printf("%s\n", fname);
	                send(newfd,(const char*)fname,strlen(fname),0);   
	                printf("send success\n");
	                while(1)
	                {
				        int len = read (entry_file, temp, MAXLINE-1); 
				        printf("%d\n",len);
				        if(len<=0)
				            break;
				        printf("%s\n",temp);
				        send(newfd,(const char*)temp,strlen(temp),0);                   
	                }
	                close(entry_file);
				    send(newfd,(const char*)"AND",strlen("AND"),0);                   

	            }
	            send(newfd,(const char*)"END$",strlen("END$"),0); 
            	close(newfd);
            	printf("TCP closed\n"); 
            	exit(0); 
        	}
        	close(newfd);
        	//close(sockfd);
        	
        } 
        //************************************ UDP part ************************************// 
        if (FD_ISSET(udpfd, &rs)) { 
	    printf("\n\nUDP Running!\n");
            ssize_t n;
            len = sizeof(cli_addr); 
            memset(buffer,0,sizeof(buffer)) ;
            // Receive domain name from client
            n = recvfrom(udpfd, buffer, sizeof(buffer), 0,(struct sockaddr*)&cli_addr, &len);
            buffer[n]='\0' ;
            printf("Recieved domain Name: %s\n",buffer); 
            char *name;
            // Using gethostbyname to get the ip
            struct hostent *add=gethostbyname(buffer);
            name = inet_ntoa(*((struct in_addr*)add->h_addr_list[0]));            
            strcpy(buffer,name);
            printf("IP: %s\n",buffer);
            sendto(udpfd, buffer, sizeof(buffer), 0,(struct sockaddr*)&cli_addr, sizeof(cli_addr)); 
        } 
    } 
    return 0;
} 
