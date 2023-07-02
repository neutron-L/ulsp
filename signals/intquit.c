/*
 * 为SIGINT信号和SIGQUIT信号设置同一处理函数，且二者的处理逻辑不同
 * 对于SIGINT信号，打印提示信息以及第几次接收到该号（counter）
 * 对于SIGQUIT信号，打印提示信息并退出
 * 可以使用t_kill.c发送信号给它，也可以通过中断输入发送信号(Ctrl-C to send SIGINT and Ctrl-\ to send SIGQUIT)
 * */

#include <signal.h>
#include "tlsp_hdr.h"

static void
sigHandler(int sig)
{
    static int count = 0;

    /* 
        UNSAFE: This handler uses non-async-signal-safe functions
        (printf, exit)
     */

    if (sig == SIGINT)
    {
        ++count;
        printf("Caught SIGINT (%d)\n", count);
        return; /* Resume execution at point of interruption */
    }  

    /* Must be SIGQUIT - print a message and terminate the process */

    printf("Caught SIGQUIT - that`s all folks!\n");

    exit(EXIT_SUCCESS);
}

int 
main(int argc, char ** argv)
{
    /* Establish same handler for SIGINT and SIGQUIT */

    if (signal(SIGINT, sigHandler) == SIG_ERR)
        errExit("signal");
    if (signal(SIGQUIT, sigHandler) == SIG_ERR)
        errExit("signal");
    
    
    for(;;)      /* Loop forever, waiting for signals */
        pause(); /* Block until a signal is caught */
}