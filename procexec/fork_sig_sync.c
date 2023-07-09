/*
 * 《TLPI》程序清单24-6，利用信号同步进程间动作
 * 父进程等待子进程发送SYNC_SIG
 * */
#include <signal.h>
#include "tlsp_hdr.h"

#define SYNC_SIG SIGUSR1

static void
handler(int sig)
{
}

int main(int argc, char **argv)
{
    pid_t childPid;
    sigset_t blockMask, origMask, emptyMask;
    struct sigaction sa;

    setbuf(stdout, NULL);

    sigemptyset(&blockMask);
    sigaddset(&blockMask, SYNC_SIG);
    if (sigprocmask(SIG_BLOCK, &blockMask, &origMask) == -1)
        errExit("sigprocmask");

    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = handler;
    if (sigaction(SYNC_SIG, &sa, NULL) == -1)
        errExit("sigaction");

    switch (childPid = fork())
    {
    case -1:
        errExit("fork");
    case 0: /* Child */
        printf("[%ld] Child started - doing some work\n", (long)getpid());
        sleep(2);
        printf("[%ld] Child about to signal parent\n", (long)getpid());
        if (kill(getppid(), SYNC_SIG) == -1)
            errExit("kill");

        _exit(EXIT_SUCCESS);
    default:
        printf("[%ld] Parent about to wait for signal\n", (long)getpid());
        sigemptyset(&emptyMask);
        if (sigsuspend(&emptyMask) == -1 && errno != EINTR)
            errExit("sigsuspend");

        printf("[%ld] Parent got signal\n", (long)getpid());
        if (sigprocmask(SIG_SETMASK, &origMask, NULL) == -1)
            errExit("sigprocmask");
        exit(EXIT_SUCCESS);
    }
}