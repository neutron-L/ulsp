/*
 * 使用fork、pipe和dup2，实现ls | sort
 * 该程序可以作为shell模拟实现多个进程通过管道通信的功能
 * */
#include <stdio.h>

#include "tlsp_hdr.h"

int main()
{
    int pipefd[2];
    pid_t pid;

    // 创建管道
    if (pipe(pipefd))
        errExit("pipe");

    // 创建子进程执行ls
    if ((pid = fork()) == -1)
        errExit("fork");
    else if (pid == 0)
    {
        printf("[%ld] ls:\n", getpid());

        dup2(pipefd[1], 1);
        close(pipefd[0]);
        close(pipefd[1]);

        execlp("ls", NULL);
    }

    // 创建子进程执行sort
    if ((pid = fork()) == -1)
        errExit("fork");
    else if (pid == 0)
    {
        printf("[%ld] sort:\n", getpid());
        dup2(pipefd[0], 0);
        close(pipefd[0]);
        close(pipefd[1]);

        execlp("sort", NULL);
    }

    // 等待子进程结束
    close(pipefd[0]);
    close(pipefd[1]);
    while ((pid = wait(NULL)) > 0)
        printf("[%ld] child exit\n", pid);

    return 0;
}