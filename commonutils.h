#include <string.h>

typedef struct file_struct
{
    int name_length;
    char file_name[100];
    int file_size;
    char file_bytes[100];
};

void myWrite(char *msg,int fd_out);