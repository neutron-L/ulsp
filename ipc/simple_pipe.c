/*
 * 程序清单44-2：在父进程和子进程间使用管道通信
 * */
#include <sys/wait.h>
#include "tlsp_hdr.h"

#define BUF_SIZE 10

int
main(int argc, char *argv[])
{
    int pfd[2];
    char buf[BUF_SIZE];
    ssize_t numRead;

    if (pipe(pfd) == -1)
        errExit("pipe");
    switch (fork())
    {
    case -1:
        errExit("fork");
    case 0:
        if (close(pfd[1]) == -1)
            errExit("close - child");
        while (1)
        {
            numRead = read(pfd[0], buf, BUF_SIZE);
            if (numRead == -1)
                errExit("read");
            else if (numRead == 0)
                break;
            else
            {
                if (write(STDOUT_FILENO, buf, numRead) != numRead)
                    fatal("child - partial/failed write");
            }
        }
        write(STDOUT_FILENO, "\n", 1);
        if (close(pfd[0]) == -1)
            errExit("close");
        _exit(EXIT_SUCCESS);    
    default:
        if (close(pfd[0]) == -1)
            errExit("close - parent");
        if (write(pfd[1], argv[1], strlen(argv[1])) != strlen(argv[1]))
            fatal("parent - partial/failed write");
        if (close(pfd[1]) == -1)
            errExit("close");
        wait(NULL);
    }
    return 0;
}
