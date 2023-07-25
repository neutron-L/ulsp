/*
 * 《TLPI》程序清单28-3. 使用clone()创建子线程
 * */
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sched.h>
#include "tlsp_hdr.h"



static int
childFunc(void * arg)
{
    if (close(*((int *)arg)) = -1)
        errExit("close");
    return 0;
}




int
main(int argc, char ** argv)
{
    const int STACK_SIZE = 65536;
    char * stack;
    char * stackTop;
    int s, fd, flags;


    // Child will close this fd
    if ((fd = open("/dev/null", O_RDWR)) == -1)
        errExit("open");
    
    // If argc > 1, child shares file descriptor table with parent
    flags = (argc > 1) ? CLONE_FILES : 0;

    // Allocate stack for child
    stack = malloc(STACK_SIZE);
    if (!stack)
        errExit("malloc");
    
    stackTop = stack + STACK_SIZE;

    
}