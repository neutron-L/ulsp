/*
 * 《TLPI》程序清单24-5：fork()后，父子进程竞争输出信息
 * 通过命令行参数指定重复的次数，每一次循环，父进程创建子
 * 进程，然后打印信息并且等待其结束，再执行下一次重复。子
 * 进程则简单打印信息。可以观察父进程和子进程谁先打印信息，
 * 即谁先执行。在我的环境下，结果是几乎每次都是父进程先执行。
 * */

#include <sys/wait.h>
#include "tlsp_hdr.h"

int main(int argc, char **argv)
{
    int numChildren, j;
    pid_t childPid;

    if (argc > 1 && !strcmp(argv[1], "--help"))
        usageErr("%s [num-children]\n", argv[0]);

    numChildren = (argc > 1) ? getInt(argv[1], GN_GT_0, "num-children") : 1;
    setbuf(stdout, NULL);

    for (j = 0; j < numChildren; ++j)
    {
        switch (childPid = fork())
        {
        case -1:
            errExit("fork");
        case 0:
            printf("%d child\n", j);
            _exit(EXIT_SUCCESS);

        default:
            printf("%d parent\n");
            wait(NULL);
            break;
        }
    }

    exit(EXIT_SUCCESS);
}
