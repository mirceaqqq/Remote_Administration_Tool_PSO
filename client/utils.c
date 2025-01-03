#include "utils.h"

char** parse_line(char* line)
{
    int nr_args=0;

    char** parsed_line;
    parsed_line=(char**)malloc(30*sizeof(char*));

    for(int i=0;i<30;i++)
    {
        parsed_line[i]=(char*)malloc(30*sizeof(char));
    }

    char* p=strtok(line," \n");

    while(p!=NULL)
    {   
        strcpy(parsed_line[nr_args],p);           
        nr_args++;
        p=strtok(NULL," \n");
    }

    parsed_line[nr_args]=NULL;

    return parsed_line;
}


void get_command_output(char *command)
{   
    int pipe_fd[2];
    pid_t pid=fork();

    if(pid==-1)
    {
        myWrite("error on fork",STDOUT_FILENO);
        return;
    }
    if(pid==0)
    {   
        char** parsed_command=parse_line(command);
        dup2(pipe_fd[1],STDOUT_FILENO);
        close(pipe_fd[0]);
        close(pipe_fd[1]);

        execvp(command[0],command);
        exit(0);
        
    }
    else
    {   
        wait();
        close(pipe_fd[1]);

        int bytesRead;
        char buffer[BUFFSIZE];
        read(pipe_fd[0],buffer,1024);
    }
}