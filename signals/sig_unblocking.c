/*
 * 探究《TLPI》22.6中，解除对多个信号的阻塞时，信号的传递顺序
 * 原理：目前Linux的实现中，内核按照信号编号的升序传递信号，当
 * 多个解除了阻塞的信号正在等待传递，如果在signal handler执行
 * 期间发生了内核态到用户态的切换，则中断此signal handler的执
 * 行，转而调用第二个signal handler
 * 代码流程：为SIGUSR1、SIGINT和SIGQUIT注册handler。首先阻塞
 * 这两个信号，然后一直等待直到接收到SIGUSR1，再解除阻塞，随即我们
 * 可以观察这两个信号的处理和传递次序。在SIGINT的handler中，会
 * 触发系统调用，进入内核态再回到用户态。
 * 过程：在我的机器上，信号似乎是按照降序传递，且不会被系统调用打断
 * 目前尚不知道具体处理细节，我试了四种信号
 * */

#include <signal.h>
#include "tlsp_hdr.h"

static volatile sig_atomic_t flag = 0;

static void
sigusr1Handler(int sig)
{
    flag = 1;
}

static void
handler(int sig)
{
    printf("Caught %d(%s)\n", sig, strsignal(sig));

    // one syscall
    sleep(1);

    printf("Return from %d(%s)\n", sig, strsignal(sig));
}


int main()
{
    // 注册 handler
    struct sigaction sa;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    sa.sa_handler = sigusr1Handler;
    if (sigaction(SIGUSR1, &sa, NULL) == -1)
        errExit("sigaction");

    sa.sa_handler = handler;
    if (sigaction(SIGINT, &sa, NULL) == -1)
        errExit("sigaction");

    sa.sa_handler = handler;
    if (sigaction(SIGQUIT, &sa, NULL) == -1)
        errExit("sigaction");

    sa.sa_handler = handler;
    if (sigaction(SIGTSTP, &sa, NULL) == -1)
        errExit("sigaction");

    sa.sa_handler = handler;
    if (sigaction(SIGUSR2, &sa, NULL) == -1)
        errExit("sigaction");

    // print PID
    printf("I am proc %ld\n", (long)getpid());

    // block SIGINT SIGQUIT
    sigset_t blockSig, prevSig;
    sigemptyset(&blockSig);
    sigaddset(&blockSig, SIGINT);
    sigaddset(&blockSig, SIGQUIT);
    sigaddset(&blockSig, SIGTSTP);
    sigaddset(&blockSig, SIGUSR2);
    if (sigprocmask(SIG_BLOCK, &blockSig, &prevSig) == -1)
        errExit("sigprocmask");

    while (flag == 0)
        continue;

    // unblock SIGINT SIGQUIT
    if (sigprocmask(SIG_SETMASK, &prevSig, NULL) == -1)
        errExit("sigprocmask");

    exit(EXIT_SUCCESS);
}
