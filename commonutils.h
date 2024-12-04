#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct file_struct
{
    int name_length;
    char file_name[100];
    int file_size;
    char file_bytes[100];
};

void myWrite(char *msg,int fd_out);

int recvData(int sockfd, char *buffer);

void sendData(int sockfd, char* buffer, int size);

void flushSocketRead(int sockfd);