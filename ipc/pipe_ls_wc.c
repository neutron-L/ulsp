/*
 * 程序清单44-3：使用管道同步多个进程
 * */
#include <sys/wait.h>
#include <tlsp_hdr.h>

int main(int argc, char *argv[])
{
    int pfd[2];

    if (pipe(pfd))
        errExit("pipe");

    switch (fork())
    {
    case -1:
        errExit("fork");
    case 0:
        if (close(pfd[0]))
            errExit("close");
        if (close(STDOUT_FILENO))
            errExit("close");
        dup2(pfd[1], STDOUT_FILENO);
        execlp("ls", "ls", NULL);
        errExit("exec");
    default:
        break;
    }

    if (close(pfd[1]))
        errExit("close");
    switch (fork())
    {
    case -1:
        errExit("fork");
    case 0:
        if (close(STDIN_FILENO))
            errExit("close");
        dup2(pfd[0], STDIN_FILENO);
        execlp("wc", "wc", NULL);
        errExit("exec");
    default:
        break;
    }
    if (close(pfd[0]))
        errExit("close");
    exit(EXIT_SUCCESS);
}