#ifndef SYSTEM_MONITOR_H
#define SYSTEM_MONITOR_H
#define BUFFER_SIZE 1024*10

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../commonutils.h"



void monitor_system(int sockfd);

#endif