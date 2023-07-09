/*
 * 探究子进程是否会继承父进程的信号掩码和未处理的信号，根据《APUE》
 * chapter 8 8.3的描述，子进程会继承信号屏蔽，但是其未处理信号集
 * 设为空
 * */

#include <signal.h>
#include "signal_functions.h"
#include "tlsp_hdr.h"

int main(int argc, char **argv)
{
    pid_t childPid;
    sigset_t blockMask;

    // 设置屏蔽的信号
    sigemptyset(&blockMask);
    sigaddset(&blockMask, SIGINT);
    sigaddset(&blockMask, SIGBUS);

    if (sigprocmask(SIG_SETMASK, &blockMask, NULL) == -1)
        errExit("sigprocmask");

    // 父进程发给自己信号
    raise(SIGINT);
    raise(SIGBUS);

    switch (childPid = fork())
    {
    case -1: // Error
        errExit("fork");
    case 0: // Child
        printf("[%ld] Child Process\n");
        printPendingSigs(stdout, "Pending signals");
        printSigMask(stdout, "Blocking signals");
        printf("====================\n");
        _exit(EXIT_SUCCESS);

    default: // Parent
        printf("[%ld] Parent Process\n");
        printPendingSigs(stdout, "Pending signals");
        printSigMask(stdout, "Blocking signals");
        printf("====================\n");
        exit(EXIT_SUCCESS);
    }
}