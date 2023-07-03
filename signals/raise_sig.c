/*
 * 首先程序注册一个信号的处理函数，然后调用raise，给进程自身发送信号，查看效果
 * 再修改处理函数，处理函数还会给自身发送一个相同的信号（此时该进程还在执行，应
 * 该默认是阻塞该信号，下次调度到该进程的时候再触发处理函数），推测执行结果是反
 * 反复复执行处理函数
 * NOTE: 具体的进程调度对信号的处理有什么影响目前还不清楚
 * */
#include <signal.h>
#include "signal_functions.h"
#include "tlsp_hdr.h"


static void
sigHandler(int sig)
{
    printf("Caught %d(%s)\n", sig, strsignal(sig));
    sleep(1);
    raise(SIGUSR1);
    sleep(1);
    printPendingSigs(stdout, "");
    printf("exit\n");
}

int 
main(int argc, char ** argv)
{
    /* Establish handler for SIGUSR1 */

    if (signal(SIGUSR1, sigHandler) == SIG_ERR)
        errExit("signal");
    
    raise(SIGUSR1);
    
    exit(EXIT_SUCCESS);
}