/*
 * 使用sigfillset和sigprocmask，阻塞所有信号，
 * 并为SIGINT注册一个处理函数。
 * 然而kill发送SIGKILL信号还是可以杀死进程，发
 * 送SIGSTOP还是可以暂停进程的执行。不过SIGSTOP
 * 后发送SIGINT还是可以使得进程继续执行，但是不会
 * 触发其处理函数的执行
 * 原理：SIGKILL和SIGSTOP不能被忽视，但是阻塞它们
 * 也不会触发错误
 * */

#include <signal.h>
#include "tlsp_hdr.h"

static void 
sigHandler(int sig)
{
    printf("Caught SIGCONT\n");
}

int
main()
{
    sigset_t blockSet;
    sigfillset(&blockSet);

    if (sigprocmask(SIG_BLOCK, &blockSet, NULL) == -1)
        errExit("sigprocmask");
    
    if (signal(SIGCONT, sigHandler) == SIG_ERR)
        errExit("signal");

    for (int i = 0; ; ++i)
    {
        printf("now is %d\n", i);
        sleep(1);
    }

    exit(EXIT_SUCCESS);
}