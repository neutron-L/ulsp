/*
 * 《TLPI》程序清单27-8. 一个缺乏信号处理的system()实现
 * */
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "tlsp_hdr.h"


#define MAX_CMD_LEN 200


int
simple_system(char * command)
{
    pid_t childPid;
    int status;

    switch (childPid = fork())
    {
    case -1: // Error
        return -1;
    case 0: // Child
        execlp("sh", "sh", "-c", command, NULL);
        _exit(127);   
    default:
        if (waitpid(childPid, &status, 0) == -1)
            return -1;
        else    
            return status;
    }
}


int
main(int argc, char ** argv)
{
    char str[MAX_CMD_LEN];
    int status;

    for (;;)
    {
        printf("Command: ");
        fflush(stdout);
        if (fgets(str, MAX_CMD_LEN, stdin) == NULL)
            break;
        
        status = simple_system(str);
        printf("simple_system() returned: status=0x%04x (%d,%d)\n", 
            (unsigned int)status, status >> 8, status & 0xff);
        
        if (status == -1)
            errExit("simple_system");
        else
        {
            if (WIFEXITED(status) && WEXITSTATUS(status) == 127)
                printf("(Probably) could not invoke shell\n");
            else
                printWaitStatus(NULL, status);
        }
    }
    
    exit(EXIT_SUCCESS);
}