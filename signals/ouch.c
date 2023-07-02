/*
 * 为SIGINT信号设置处理函数，打印一条mesg"Ouch"
 * */

#include <signal.h>
#include "tlsp_hdr.h"

static void
sigHandler(int sig)
{
    printf("Ouch\n");
}

int
main(int argc, char ** argv)
{
    int j;

    if (signal(SIGINT, sigHandler) == SIG_ERR)
        errExit("signal");
    
    for (j = 0; ; j++)
    {
        printf("%d\n", j);
        sleep(3);
    }

    return 0;
}