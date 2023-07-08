/*
 * 《TLPI》程序清单24-2: 在父子进程之间共享文件偏移量和打开文件状态标志
 * 使用mkstemp()打开一个临时文件，接着调用fork()以创建子进程。子进程
 * 改变文件偏移量以及文件状态标志，最后退出。父进程随即获取文件偏移量和
 * 标志，以验证其可以观察到由子进程所造成的变化
 * */
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "tlsp_hdr.h"

int
main(int argc, char ** argv)
{
    int fd, flags;
    char template[] = "/tmp/testXXXXXX";

    setbuf(stdout, NULL);

    fd = mkstemp(template);
    if (fd == -1)
        errExit("mkstemp");

    printf("File offset before fork(): %lld\n", 
        (long long)lseek(fd, 0, SEEK_CUR));
    flags = fcntl(fd, F_GETFL);
    if (flags == -1)
        errExit("fcntl - F_GETFL");
    printf("O_APPEND flag before fork() is: %s\n",
        (flags & O_APPEND) ? "on" : "off");
    
    switch (fork())
    {
    case -1:
        errExit("fork");
    case 0:
        if (lseek(fd, 1000, SEEK_SET) == -1)
            errExit("lseek");
        flags = fcntl(fd, F_GETFL);
        if (flags == -1)
            errExit("fcntl - F_GETFL");
        flags |= O_APPEND;
        if (fcntl(fd, F_SETFL, flags) == -1)
            errExit("fcntl - F_SETFL");
        _exit(EXIT_SUCCESS);
    default:
        if (wait(NULL) == -1)
            errExit("wait");
        printf("Child has exited\n");
        printf("File offset in parent: %lld\n", 
            (long long)lseek(fd, 0, SEEK_CUR));
        flags = fcntl(fd, F_GETFL);
        if (flags == -1)
            errExit("fcntl - F_GETFL");
        printf("O_APPEND flag in parent is: %s\n",
        (flags & O_APPEND) ? "on" : "off");
        exit(EXIT_SUCCESS);
    }
}