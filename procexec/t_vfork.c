/*
 * 《TLPI》程序清单24-4：使用vfork()
 * */
#include "tlsp_hdr.h"

static int idata = 111;

int
main(int argc, char ** argv)
{
    int istack = 222;
    pid_t childPid;

    switch (childPid = vfork())
    {
        case -1:
            errExit("fork");
        case 0:
            idata *= 3;
            istack *= 3;
            if (execl("/usr/bin/cat", "cat", "t_vfork.c", NULL))
                errExit("execl");
            break;
        default:
            sleep(3);
            break;
    }

    // parent come here
    printf("PID=%ld %s idata=%d istack=%d\n",
        (long)getpid(), (childPid == 0) ? "(child) " : "(parent)", idata, istack);

    exit(EXIT_SUCCESS);
}
