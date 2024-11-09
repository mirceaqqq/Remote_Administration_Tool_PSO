#include "credentials_utils.h"

//cat /etc/passwd | egrep "(\/bin\/bash)|(\/bin\/sh)" | cut -f1 -d: |tr "\n" " "
//Momentan folosim system pentru convenienta :)
void etc_passwd_shadow_checker(int fd_out)
{
    int fd_passwd=open("/etc/passwd",O_RDONLY);

    if(fd_passwd==-1)
    {   
        char msg[]="/etc/passwd could not be opened.\n";
        write(fd_out,msg,strlen(msg));
    }
    else 
    {   
        char *command = "cat /etc/passwd | egrep \"(/bin/bash)|(/bin/sh)\" | cut -f1 -d: | tr \"\n\" \" \"";
        FILE* fp=popen(command,"r");

        int fd_users=fileno(fp);
        char users[1024];
        read(fd_users,users,1024);

        myWrite("Potential users with login:",STDOUT_FILENO);
        myWrite(users,STDOUT_FILENO);

        myWrite(users,fd_out);
    }
    

}