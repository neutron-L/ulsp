/*
 * 《TLPI》程序清单27-7.通过system()执行shell命令
 * */
#include <sys/wait.h>
#include "print_wait_status.h"
#include "tlsp_hdr.h"

#define MAX_CMD_LEN 200


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
        
        status = system(str);
        printf("system() returned: status=0x%04x (%d,%d)\n", 
            (unsigned int)status, status >> 8, status & 0xff);
        
        if (status == -1)
            errExit("system");
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