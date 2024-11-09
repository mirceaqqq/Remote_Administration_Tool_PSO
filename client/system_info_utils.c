#include "system_info_utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXLINE 1024

void collect_system_info(int sockfd)
{
    char sys_info[MAXLINE * 10] = "";

    FILE *fp = popen("uname -a", "r");
    if (fp != NULL) {
        if (fgets(sys_info, MAXLINE, fp) != NULL) {
            strcat(sys_info, "\n");
        }
        pclose(fp);
    } else {
        strcpy(sys_info, "Failed to get OS information.\n");
    }

    fp = popen("who", "r");
    if (fp != NULL) {
        strcat(sys_info, "Active Users:\n");
        char line[MAXLINE];
        while (fgets(line, sizeof(line), fp) != NULL) {
            strcat(sys_info, line);
        }
        pclose(fp);
    } else {
        strcat(sys_info, "Failed to get user information.\n");
    }

    fp = popen("ps -aux", "r");
    if (fp != NULL) {
        strcat(sys_info, "\nActive Processes:\n");
        char line[MAXLINE];
        while (fgets(line, sizeof(line), fp) != NULL) {
            if (strlen(sys_info) + strlen(line) < sizeof(sys_info) - 1) {
                strcat(sys_info, line);
            } else {
                break; // Buffer prea mare
            }
        }
        pclose(fp);
    } else {
        strcat(sys_info, "Failed to get process information.\n");
    }

    int bytes_sent = send(sockfd, sys_info, strlen(sys_info), 0);

    // Marker de sfarsit CA ALTFEL NU STIE SA SE OPREASCA FMM
    send(sockfd, "<END_OF_DATA>", strlen("<END_OF_DATA>"), 0);
}
