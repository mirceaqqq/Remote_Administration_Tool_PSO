#include "system_monitor.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXLINE 1024

void monitor_system(int sockfd)
{
    char sys_info[MAXLINE * 10] = "";
    FILE *fp = fopen("/proc/stat", "r");
    if (fp != NULL) {
        strcat(sys_info, "CPU Usage:\n");
        char line[MAXLINE];
        for (int i = 0; i < 5 && fgets(line, sizeof(line), fp) != NULL; i++) {
            strcat(sys_info, line);
        }
        fclose(fp);
    } else {
        strcat(sys_info, "Failed to read /proc/stat.\n");
    }

    fp = fopen("/proc/meminfo", "r");
    if (fp != NULL) {
        strcat(sys_info, "\nMemory Info:\n");
        char line[MAXLINE];
        while (fgets(line, sizeof(line), fp) != NULL) {
            if (strlen(sys_info) + strlen(line) < sizeof(sys_info) - 1) {
                strcat(sys_info, line);
            } else {
                break; // buffer prea mare
            }
        }
        fclose(fp);
    } else {
        strcat(sys_info, "Failed to read /proc/meminfo.\n");
    }

    myWrite("Sending system monitoring data...");
    int bytes_sent = send(sockfd, sys_info, strlen(sys_info), 0);
    if (bytes_sent == -1) {
        perror("Error sending system monitoring data");
    }
    send(sockfd, "<END_OF_DATA>", strlen("<END_OF_DATA>"), 0);
}
