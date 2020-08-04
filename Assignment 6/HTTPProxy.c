/*********************************************************************** 
 Yash Butala 17CS30038
 Nikhil Shah - 17CS10030
 Set the browser Proxy IP to your machine IP 
 set the browser proxy to 8181(preferred) and use same in command line. 
 ***********************************************************************/

#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h> 
#include <strings.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include<string.h>
#include <netdb.h>
#define BUFF_SIZE 10240
#define  MAX_CONNECTIONS  100

char first_request[MAX_CONNECTIONS][10240];

int main(int argc, char *argv[]) 
{
    if (argc != 2) 
    {
        printf("Error: Incorrect command line arguments passed\n");
        printf("Try :\t./SimProxy <listen port>\n");
        return 0;
    }


    /*
        making the strutue element for various addresses
        browaddr -> address of the laptop, also input to browser proxy, port from command line
        cliaddr -> address of the browser client
        webaddr -> address of the web server e.g, www.google.com
    */

    struct sockaddr_in browaddr, cliaddr, webaddr[MAX_CONNECTIONS];
    socklen_t len, len_cli;

    // Flags for web addresses
    int fl[MAX_CONNECTIONS];
    memset(fl,-1,sizeof(fl));

    // declaring some file descriptors
    int sel = 1,sock_in, FD, nselect;

    // filling the memory with zero byte string
    bzero(&browaddr, sizeof(browaddr));
    //  bzero(&instiaddr, sizeof(instiaddr));

    // set of file descriptors
    fd_set get_set, send_set;
    
    // arrays of file descriptors, read and write
    int fd_get[MAX_CONNECTIONS], fd_send[MAX_CONNECTIONS],count = 0;

    char buff[BUFF_SIZE];
    bzero(&buff, sizeof(buff));

    // socket to accept incoming connections from the browser
    sock_in = socket(AF_INET, SOCK_STREAM, 0);
    
    // error handling for socket creation
    if (sock_in == 0)
    {
        printf("Socket in creation failed\n");
        exit(EXIT_FAILURE);
    }

    // make the connection non blocking
    int param = fcntl(sock_in, F_SETFL, O_NONBLOCK);
    if ( param == -1) 
    {
        printf("Could not make non-blocking socket\n");
        exit(EXIT_FAILURE);
    }

    // setting socket options for the socket to reuse the same local address
    int sso = setsockopt(sock_in, SOL_SOCKET, SO_REUSEADDR, &sel, sizeof(sel));
    if(sso!=0)
    {
        printf("Setsockopt failure for socket in\n");
        exit(EXIT_FAILURE);
    }

    // setting the properties of sock addresse that is listening to the browser
    // set the browser proxy to this address
    browaddr.sin_family = AF_INET;
    browaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    browaddr.sin_port = htons(atoi(argv[1]));

    // set the timeout interval for select call
    struct timeval timeout;
    timeout.tv_sec  = 1.5;
    timeout.tv_usec = 0;

    // Bind the browser client to sock_in
    int bi = bind(sock_in, (struct sockaddr*)&browaddr, sizeof(browaddr));
    if (bi!=0)
    {
        printf("Socket Bind failure\n");
        exit(EXIT_FAILURE);
    }

   
    // listening at this socket (to accept connect requests from browser)
    if (listen(sock_in, MAX_CONNECTIONS) < 0)
    {
        printf("Failed to Listen\n");
        exit(EXIT_FAILURE);
    }

    // clearing the sets
    FD_ZERO(&get_set);
    FD_ZERO(&send_set);

    // proxy server is listening
    printf("Proxy running on port %s.\n",argv[1]); 

    while(1) 
    {
        if (count < MAX_CONNECTIONS) 
        {	
            // accept the connection from the browser
        	fd_get[count] = accept(sock_in, (struct sockaddr *)&cliaddr, &len_cli);

        	if(fd_get[count]>=0)
	        {
	            char str[100]; 
                // create a socket (that will be used to connect to the web server)
	        	fd_send[count] = socket(AF_INET, SOCK_STREAM, 0);
	            if ( fd_send[count] == -1) 
	            {
                    printf("Exit statement 1\n");
	                fprintf(stdout, "socket() failed: %s\n", strerror(errno));
	                exit(0);
	            }

	            inet_ntop(AF_INET, &(cliaddr.sin_addr), str, 100);	                   
	            printf("Connection accepted from %s:%d\n", str, (int) ntohs(cliaddr.sin_port));
                // make the socket non blocking
	            int fnc = fcntl(fd_send[count], F_SETFL, O_NONBLOCK);
	            if (fnc == -1) 
	            {
                    printf("Exit statement 2\n");
	                fprintf(stdout, "fcntl() failed: %s\n", strerror(errno));
	                exit(EXIT_FAILURE);
	            }
	            
                count+=1;
	        }
	    }    

        FD_ZERO(&get_set);
        FD_ZERO(&send_set);
        FD_SET(0, &get_set);
        FD = 1;

        // add all connections to FD SET
        for (int i = 0; i < count; i++) 
        {
            FD_SET(fd_get[i], &get_set);
            FD_SET(fd_send[i], &get_set);
            FD_SET(fd_get[i], &send_set);
            FD_SET(fd_send[i], &send_set);

            if(fd_get[i] > FD)
            	FD =  fd_get[i];
            if(fd_send[i] > FD)
            	FD =  fd_send[i];
        }

        // choose file descriptor as 1 greater than max numnber used
        FD+=1;


        // use the select call to check if theres is a request from browser
        nselect = select(FD, &get_set, NULL, NULL, &timeout);

        if (nselect > 0) 
        {
            char buff[BUFF_SIZE];
            int p, q;

            if (FD_ISSET(0, &get_set)!=0) 
            {
                memset(buff, 0, sizeof(buff));
                p = read(0, buff, sizeof(buff));
                printf("\nTyped: %s\n", buff);

				int len = strlen(buff);
				if(buff[len-1] == '\n'){
					buff[len-1] = 0;
				}

				char str[] = "exit";
                // if exit, forcefully close all sockets
                if (strcmp(buff, str) == 0) 
                {
                	int i = 0;
					int num = sysconf(_SC_OPEN_MAX);
					for(i = 3; i < num; i++)
						close(i);

                    return 0;
                } 
            }
            
            for (int i = 0; i < count; i++) 
            {
                char buff[BUFF_SIZE];
                int p, q;

                if(FD_ISSET(fd_get[i], &get_set)!=0 && fl[i]==-1)
                {
                	//if(FD_ISSET(fd_send[i], &send_set)!=0) 
                	{
                        /*
                         *first read the data from the fd_get[i]
                         *then parse the data to get the http host and port name
                         *then 'connect' to the host name using the socket fd_send[i]
                        */
	                   
                        memset(buff, 0, sizeof(buff));

	                    p = read(fd_get[i], buff, sizeof(buff));
	                    //printf("%s",buff );

                        // parsing starts here
	                    char request[10],host[200],path[500];
	                    int i0=0;
	                    while(buff[i0]!=' ')
	                    {
	                    	request[i0]=buff[i0];
	                    	i0++;
	                    }
	                    request[i0]='\0';
	                
	                    int cnt=0;
	                    if(strcmp("GET",request)==0 || strcmp("POST",request)==0)
	                    {
	                    	strcpy(first_request[i],buff);
                            printf("------------------------------------------------------\n");
	                    	printf("Request : %s\n", request);
		                    while(cnt<2)
		                    {
		                    	if(buff[i0]=='/')
		                    		cnt++;
		                    	i0++;
		                    }
		                    int i1=0;
		                    while(buff[i0]!='/')
		                    {
		                    	host[i1]=buff[i0];
		                    	i0++;
		                    	i1++;
		                    }
		                    host[i1]='\0';
		                	char host2[200];
		                	int j=0;
		             		while(host[j]!='\0')
		                    {
		                    	if(host[j]==':')
		                    		break;
		                    	host2[j]=host[j];
		                    	j++;
		                    }
		                    host2[j]='\0';
		                    int port=80;

		                    if(host[j]==':')
		                    {
		                    	j++;
			                    int j1=0;
			                    char port_c[100];
			                    while(host[j]!='\0')
			                    {
			                    	port_c[j1]=host[j];
			                    	j1++;j++;
			                    }
			                    port_c[j1]='\0';
			                    port = atoi(port_c);
		                    }

		                    printf("Host : %s\n", host2);
		                    printf("Port : %d\n", port);

		                    int i2=0;
		                    while(buff[i0]!=' ')
		                    {
		                    	path[i2]=buff[i0];
		                    	i0++;
		                    	i2++;
		                    }
		                    path[i2]='\0';

		                    printf("Path  : %s\n",path);

		                    char *ip_addr; 
		                  	struct hostent *add = gethostbyname(host2);
            				ip_addr = inet_ntoa(*((struct in_addr*)add->h_addr_list[0]));
		                    printf("Host IP : %s\n", ip_addr);
                            printf("------------------------------------------------------\n\n");
	                        
                            //store the information of a webaddress in an array
	                        webaddr[i].sin_family = AF_INET; 
    						webaddr[i].sin_port = htons(port); 
						    webaddr[i].sin_addr.s_addr = inet_addr(ip_addr) ; 

                            // mark the connect flag 0 / False
						    fl[i]=0;
	                    }
	                    
	                }
	            } 
            }
        }

        // use the select call to check if there is a response for the first request from the web server
        nselect = select(FD, NULL, &send_set, NULL, &timeout);

        if (nselect > 0) 
        {
        	for(int i=0;i<count;i++)
        	{
        		if(FD_ISSET(fd_send[i], &send_set)!=0 && !fl[i])
        		{
		            int connect_fd = connect(fd_send[i], (struct sockaddr *)&webaddr[i], sizeof(webaddr[i]));
		            usleep(100000);
		            if(connect_fd < 0)
		            {
                        // Error number 115 corresponds to EINPROGRESS
                        if(errno == 115){
                            continue;
                        }

		            }
		            else 
		            {
                        printf("----------------------------------------------------------------------------------\n");
		            	printf("Successfully set up connection to the web server host %s\n",inet_ntoa(webaddr[i].sin_addr));
                        printf("----------------------------------------------------------------------------------\n\n");

                        // mark the flag to 1 / True when ready to connect
		            	fl[i]=1;

		            	char buff_n[1024];
		            	int i1 = 0,i2 = 0;
		            	int flag = 0;

		            	while(first_request[i][i1] != '\0')
		            	{
		 					if(flag == 0 || flag>3 || (flag == 3 && first_request[i][i1] == '/'))
			            	{
			            		buff_n[i2] = first_request[i][i1];
			            		i2++;
			            	}
			            	if(first_request[i][i1] == ' ' || first_request[i][i1] == '/')
			            		flag++;
			            	i1++; 
		            	}

		            	int q = send(fd_send[i], buff_n, 1024, 0);
		            	if(q==-1)
		            	{
                           continue; 
		            	}
		            }	
		        }    
	       	} 
        }
        
        // check for other requests on the same page
        nselect = select(FD, &get_set, &send_set, NULL, &timeout);

        if(nselect>0)
        {
        	for (int i = 0; i < count; i++) 
            {
                char buff[BUFF_SIZE];
                int p, q;

                if(FD_ISSET(fd_get[i], &get_set)!=0 )
                {
                	if(FD_ISSET(fd_send[i], &send_set)!=0) 
                	{
                        // set relative paths in the request payload
	                    memset(buff, 0, sizeof(buff));
	                    p = read(fd_get[i], buff, sizeof(buff));
	                   	char buff_n[1024];

	                   	if(!((buff[0]=='G' && buff[1]=='E' && buff[2]=='T')||(buff[0]=='P' && buff[1]=='O' && buff[2]=='S' && buff[2]=='T')))
	                   		continue;

		            	int i1 = 0,i2 = 0;
		            	int flag = 0;
		            	while(buff[i1]!='\0')
		            	{
		 					if(flag == 0 || flag > 3 || (flag == 3 && buff[i1] == '/'))
			            	{
			            		buff_n[i2] = buff[i1];
			            		i2++;
			            	}
			            	if(buff[i1] == ' ' || buff[i1] == '/')
			            		flag++;
			            	i1++; 
		            	}
	                    
                        q = send(fd_send[i], buff_n, sizeof(buff_n), 0);

                        // handle the error
	                    if (q == -1 || p == -1) 
	                    {
                            continue;    
	                    }
	                    
	                }
	            } 

                if(FD_ISSET(fd_send[i], &get_set)!=0 )
                {	
                	if( FD_ISSET(fd_get[i], &send_set)!=0) 
                	{
	                    memset(buff, 0, sizeof(buff));
	                    p = read(fd_send[i], buff, sizeof(buff));
                        if(p == -1){
                            if(errno == 107){
                                continue;
                            }
                        }

	                    q = send(fd_get[i], buff, p, 0);
	                    if (q == -1) 
	                    {
                            if(errno == 104){
                                // connection closed by peer
                                close(fd_get[i]);
	                            close(fd_send[i]);
                                continue;
                            }
                            else if(errno == 14){
                                continue;
                            }
	                    }
                	}
                }
        	}
    	}
    }

    printf("Terminating the program\n");
    return 0;
}
