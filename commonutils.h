#pragma once
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#define MAXLINE 1024
#define CONN_TIMEOUT 30
#define CONN_CLOSED -1
#define CONN_ERROR -2
typedef struct file_struct
{
    int name_length;
    char file_name[100];
    int file_size;
    char file_bytes[100*MAXLINE];
};

void myWrite(char *msg,int fd_out);

int recvData(int sockfd, char *buffer);

void sendData(int sockfd, char* buffer, int size);

void flushSocketRead(int sockfd);