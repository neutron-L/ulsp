/*
 * 《TLPI》exercise 26-1.父进程在子进程之前终止，此时查看子进程的父进程id，验证是否为1，即init进程收养子进程
 * */

#include <signal.h>
#include "tlsp_hdr.h"


int 
main(int argc, char **argv)
{
    switch (fork())
    {
    case -1:
        errExit("fork");
    case 0: // Child
        sleep(3); // give time to parent to exit
        printf("I am Child process [%ld], my parent is [%ld]\n", (long)getpid(), (long)getppid());
        _exit(EXIT_SUCCESS);
    default:
        printf("I am Parent process [%ld]\n", (long)getpid());
        exit(EXIT_SUCCESS);
    }
}