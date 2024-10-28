#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT 8080
#define MAXLINE 1024

void exec_on_client(int clientfd)
{   
    char input_message[MAXLINE];
    strcpy(input_message,"Enter command to execute on client: ");
    write(STDOUT_FILENO,input_message,sizeof(input_message));

    char command[MAXLINE];

    read(STDIN_FILENO,command,sizeof(command));

    write(clientfd,command,strlen(command));

    char command_result[MAXLINE*MAXLINE];

    int recv_len;
    recv_len=recv(clientfd,command_result,MAXLINE*MAXLINE,0);

    write(STDOUT_FILENO,command_result,recv_len);
    write(STDOUT_FILENO,"\n",1);
}

int main()
{
    int sockfd;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr;

    sockfd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

    if(sockfd<0)
    {
        perror("socket creation failed\n");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr,0,sizeof(servaddr));

    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(PORT);
    servaddr.sin_addr.s_addr=INADDR_ANY;

    if(bind(sockfd,(const struct sockaddr*)&servaddr,sizeof(servaddr))<0)
    {
        perror("socket bind failed\n");
        exit(EXIT_FAILURE);
    }

    if(listen(sockfd,20)<0)
    {
        perror("socket listen error\n");
        exit(EXIT_FAILURE);
    }
    else 
        printf("Server listening...\n");
    
    struct sockaddr_in client_addr;
    int connfd;
    int len=sizeof(client_addr);

    connfd=accept(sockfd,(struct sockaddr*) &client_addr,&len);

    if(connfd<0)
    {
        perror("server accept error\n");
        exit(EXIT_FAILURE);
    }
    else
        printf("Server accepted client connection\n");

    while(1)
    {
        exec_on_client(connfd);
    }

    close(connfd);
    
    return 0;
}