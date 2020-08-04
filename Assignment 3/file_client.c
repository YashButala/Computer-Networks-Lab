#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<netdb.h>
#include<sys/types.h>
#include<sys/socket.h>	
#include<unistd.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>
#define MAX 100
void fun(int sockfd,int new_sock)
{
	char buff[MAX];
	int n;
	printf("Enter the fle you want to access\n");
	scanf("%s",buff);
	strcat(buff, "$");
	send(sockfd,(const char*)buff,strlen(buff),0);
	 
  	int flag=0,fd, c_flag = 1;
  	int char_count=0;
  	int word_count=0;
	while(1)
	{
		int len = recv(sockfd,buff,strlen(buff),0);
		if(len>0)
		{
			if(!flag)
			{
				fd = open("received.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644); 
				if (fd < 0) 
  				{ 
     				perror("r1"); 
     				exit(1); 
  				}
  				flag = 1;	
			}
			buff[len]='\0';
			//printf("%s\n",buff);
			write(fd, buff, strlen(buff));
			char_count+=len;
			for(int i=1;i<len;i++)
			{
				if(buff[i-1]==' '||buff[i-1]==';'||buff[i-1]=='.'||buff[i-1]==','||buff[i-1]=='\t' || buff[i-1]=='\n')
				{
					if(buff[i]!=' '&&buff[i]!=';'&&buff[i]!='.'&& buff[i]!=',' && buff[i]!='\t' && buff[i-1]!='\n')
					{
						word_count++;
					}				
				}	
			}
			if(buff[0]!=' '&&buff[0]!=';'&&buff[0]!='.'&& buff[0]!=',' && buff[0]!='\t' && buff[0]!='\n' && c_flag){
				word_count++;
			}
			
			if(buff[len-1]==' '||buff[len-1]==';'||buff[len-1]=='.'||buff[len-1]==','||buff[len-1]=='\t' || buff[len-1]=='\n'){
				c_flag = 1;
			}else{
				c_flag = 0;
			}
		}
		else if(len==0)
		{
			if(!flag)
			{
				printf("File not Found\n");
				break;
			}
			printf("Byte size is = %d\n",char_count);
			printf("Word count is = %d\n",word_count);
			printf("Saved in file \'received.txt\'\n");
			printf("Closing Connection\n");
			break;
		}
		else
		{
			printf("recv failed with error\n");
			break;
		}

	}
}
int main()
{
	int sockfd,connfd;
	struct sockaddr_in serv_addr, clie_addr;
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd == -1)
	{
		printf("socket creation failed...");
		exit(0);
	}	
	else
	{
		printf("socket creation successful...\n");
	}
	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(8181);
	
	int new_sock;
	new_sock=connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr));
	if(new_sock!=0)
	{
		printf("Connecting server failed...\n");
		exit(0);
	}
	else
	{
		printf("connected to the server....\n");
	}
	fun(sockfd,new_sock);
	close(sockfd);

	return 0;
}
