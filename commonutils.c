#include "commonutils.h"

void myWrite(char *msg,int fd_out)
{
    int len=strlen(msg);
    write(fd_out,msg,len);
    if(msg[len-1]!='\n')
        write(fd_out,"\n",1);

}