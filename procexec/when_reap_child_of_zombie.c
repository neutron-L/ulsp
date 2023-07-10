/*
 * 《TLPI》exercise 26-2.当父进程退出时，其子进程何时被收养
 * 1. 父进程被回收时（如祖父进程调用wait()）
 * 2. 父进程终止时
 * 如书上描述的，情况为第二种
 * */

#include <signal.h>
#include "tlsp_hdr.h"

static volatile sig_atomic_t childExit = 0;

static void
sigchldHandler(int sig)
{
    // UNSAFE

    int status, savedError;
    pid_t childPid;

    savedError = errno;

    while ((childPid = waitpid(-1, &status, WNOHANG)) > 0) // 第三个参数改为0（CSAPP中示例程序有这样做的），则每次都会阻塞直到ECHLD
        printf("[%ld]: reap child [%ld]\n", (long)getpid(), (long)childPid);
    childExit = 1;
    if (childPid == -1 && errno != ECHILD)
        errMsg("waitpid");

    errno = savedError;
}

int main(int argc, char **argv)
{
    pid_t pid;

    // 当前进程为祖父进程

    if ((pid = fork()) == -1)
        errExit("fork");
    else if (pid == 0) // 进入父进程
    {
        printf("I am parent process [%ld]\n", (long)getpid());
        // 创建子进程
        if ((pid = fork()) == -1)
            errExit("fork");
        else if (pid == 0) // 子进程
        {
            printf("I am child process [%ld]\n", (long)getpid());
            // 等待父进程退出
            while (getppid() != 1)
                continue;
            printf("[%ld]: Now my parent is [%ld]\n", (long)getpid(), (long)getppid());
        }
        else
        {
            sleep(2);
            _exit(EXIT_SUCCESS);
        }
    }
    else
    {
        // wait parent to exit
        sigset_t blockMask, emptyMask;
        struct sigaction sa;

        // set SIGCHLD handler
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sa.sa_handler = sigchldHandler;
        if (sigaction(SIGCHLD, &sa, NULL) == -1)
            errExit("sigaction");

        /* Block SIGCHLD to prevent its delivery if a child
            terminates before the  */
        sigemptyset(&blockMask);
        sigaddset(&blockMask, SIGCHLD);
        if (sigprocmask(SIG_SETMASK, &blockMask, NULL) == -1)
            errExit("sigprocmask");

        // give time for parent process to exit
        sleep(5);

        while (!childExit)
            sigsuspend(&emptyMask);

        // exit
        exit(EXIT_SUCCESS);
    }
}