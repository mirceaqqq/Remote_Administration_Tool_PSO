#include "commonutils.h"

void myWrite(char *msg,int fd_out)
{
    int len=strlen(msg);
    write(fd_out,msg,len);
    if(msg[len-1]!='\n')
        write(fd_out,"\n",1);

}

int recvData(int sockfd, char* buffer)
{
    char* ret_buffer=(char *)malloc(8192*10);
    int bytes_read;
    char read_buffer[8192];
    int total_bytes_read=0;

    ret_buffer[0]=0;

    while((bytes_read=read(sockfd,read_buffer,sizeof(read_buffer)-1))>0)
    {
        read_buffer[bytes_read]='\0';
        char *end_marker=strstr(read_buffer,"<END_OF_DATA>");
        if(end_marker!=NULL)
        {
            *end_marker='\0';
            strcat(ret_buffer,read_buffer);
            myWrite("received data",STDOUT_FILENO);
        }
        strcat(ret_buffer,read_buffer);
        total_bytes_read+=bytes_read;
    }

    buffer=ret_buffer;
    return total_bytes_read;
    
}

void sendData(int sockfd, char* buffer, int size);

void flushSocketRead(int sockfd)
{
    char temp_buffer[8192];

    ssize_t bytes_read;

    while((bytes_read=recv(sockfd,temp_buffer,sizeof(temp_buffer),MSG_DONTWAIT))>0) {}

}