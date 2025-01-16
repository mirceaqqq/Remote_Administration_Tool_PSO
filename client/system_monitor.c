#include "system_monitor.h"

void monitor_system(int sockfd) {
    printf("Sending acknowledgment...\n");
    send(sockfd, "<ACK>", strlen("<ACK>"), 0);

    char buffer[BUFFER_SIZE] = {0};
    strcat(buffer, "=== System Information ===\n\n");

    FILE *cmd = popen("top -bn1 | head -n 5", "r");
    if (cmd) {
        char line[1024];
        strcat(buffer, "CPU Usage:\n");
        while (fgets(line, sizeof(line), cmd)) {
            strcat(buffer, line);
        }
        pclose(cmd);
    }

    cmd = popen("free -h", "r");
    if (cmd) {
        char line[1024];
        strcat(buffer, "\nMemory Usage:\n");
        while (fgets(line, sizeof(line), cmd)) {
            strcat(buffer, line);
        }
        pclose(cmd);
    }
    
    printf("Sending system info...\n");
    strcat(buffer, "<END_OF_DATA>");
    send(sockfd, buffer, strlen(buffer), 0);
}