#include<sys/socket.h>
#include<netinet/in.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include<fcntl.h>
#include<string.h>

#define max(a, b) (a > b ? a : b)
#define MAXLINE 1024
#define PORT 8080
#define SA struct sockaddr



void str_cli(FILE *fp, int sockfd)
{
	int maxfdp1, stdineof;
	fd_set rset;
	
	char buf[MAXLINE], user[MAXLINE], pass[MAXLINE];
	
	char *userMsg = "Enter user id: ";
	char *passMsg = "Enter password: ";
	
	int n, loginStatus = -1;
	
	stdineof = 0;
	
	FD_ZERO(&rset);
	
	for ( ; ; ) 
	{
		bzero(buf, sizeof(buf));
		bzero(user, sizeof(user));
		bzero(pass, sizeof(pass));
		
		if (stdineof == 0)
			FD_SET(fileno(fp), &rset);
		
		FD_SET(sockfd, &rset);
		maxfdp1 = max(fileno(fp), sockfd) + 1;
		
		//User not logged in.
		while(loginStatus == -1)
		{
			write(1, userMsg, strlen(userMsg));
			read(1, user, MAXLINE);

			write(1, passMsg, strlen(passMsg));
			read(1, pass, MAXLINE);

			//If userId or password is null, ask user to re-enter.
			if(strcmp(user, "\n") == 0 || strcmp(pass, "\n") == 0)
			{
				bzero(user, strlen(user));
				bzero(pass, strlen(pass));

				char *msg = "Enter again: \n";
				write(1, msg, strlen(msg));
				continue;
			}

			strcat(user, pass);
			write(sockfd, user, strlen(user));
			loginStatus = 0;
		}

		select(maxfdp1, &rset, NULL, NULL, NULL);
		
		if (FD_ISSET(sockfd, &rset))
		{ 
			//Socket is readable
			if ( (n = read(sockfd, buf, MAXLINE)) == 0) 
			{
				if (stdineof == 1)
				{	
					printf("Normal termination..\n");
					//Normal termination
					return;
				}
				else
				{
					//Server terminated prematurely.
					char *msg = "Server terminated prematurely..\n";
					write(fileno(stdout), msg, strlen(msg));
					exit(0);
				}
			}

			if(strcasecmp(buf, "Sorry your id and password is not correct\n") != 0)
				//User logged in.
				loginStatus = 0;
			else
				//Incorrect userId or password.
				loginStatus = -1;

			
			write(fileno(stdout), buf, n);
			
			if(loginStatus == -1)
				continue;
		}

		if (FD_ISSET(fileno(fp), &rset)) 
		{ 
			//Input is readable
			if ( (n = read(fileno(fp), buf, MAXLINE)) == 0) 
			{
				stdineof = 1;
				
				//Send FIN 
				shutdown(sockfd, SHUT_WR);
				FD_CLR(fileno(fp), &rset);
				continue;
			}
			if(strcasecmp("BYE\n", buf) == 0)
			{
				write(sockfd, buf, strlen(buf));
				stdineof = 1;
				shutdown(sockfd, SHUT_WR);
				
				continue;
			}

            char store[] = "STORE\n";
            int res = strcasecmp(buf, store);
            
			// Special handling for 'STORE' command.
			while(strcasecmp(buf, store) == 0) 
            {
            	char first[MAXLINE];
            	bzero(first, sizeof(first));
				read(fileno(fp), first, MAXLINE);
				
				char second[MAXLINE];
				bzero(second, sizeof(second));
				read(fileno(fp), second, MAXLINE);
      			
				// Prompt user again if empty arguments for 'STORE'.
				if(strcasecmp(second, "\n") == 0 || strcasecmp(first, "\n") == 0)
				{
					char *msg = "Enter again: \n";
					write(1, msg, strlen(msg));
					bzero(first, strlen(first));
					bzero(second, strlen(second));
					continue;
				}
				else
				{	
					// non-empty arguments, concatenate and store in buffer.
					strcat(buf, first);
					strcat(buf, second);
					break;
				}
            }
			
			write(sockfd, buf, strlen(buf));
		}
	}
}

int main()
{
	int sockFd;
	struct sockaddr_in servaddr;
	char buff[1024];
	sockFd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockFd == -1)
	{
		printf("Socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created...\n");
	
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_port =htons(PORT);
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if(connect(sockFd, (SA*) &servaddr, sizeof(servaddr)) != 0)
	{
		printf("Connection with the server failed...\n");
		exit(0);
	}
	else
		printf("Connected to the SOUND server..\n");
		
	str_cli(stdin, sockFd);
	close(sockFd);
}
