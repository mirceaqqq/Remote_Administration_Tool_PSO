#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFFSIZE 8192

void myWrite(char *msg,int fd_out);

void get_command_output(char * command);

char** parse_line(char* line);