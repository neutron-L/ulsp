/*
 * 《TLPI》程序清单24-1：调用fork()
 * */
#include "tlsp_hdr.h"

static int idata = 111;

int
main(int argc, char ** argv)
{
    int istack = 222;
    pid_t childPid;

    switch (childPid = fork())
    {
        case -1:
            errExit("fork");
        case 0:
            idata *= 3;
            istack *= 3;
            break;
        default:
            sleep(3);
            break;
    }

    // Both parent and child come here
    printf("PID=%ld %s idata=%d istack=%d\n",
        (long)getpid(), (childPid == 0) ? "(child) " : "(parent)", idata, istack);

    exit(EXIT_SUCCESS);
}