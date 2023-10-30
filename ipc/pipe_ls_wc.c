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
        if (pfd[1] != STDOUT_FILENO)
        {
            if (dup2(pfd[1], STDOUT_FILENO) == -1)
                errExit("dup2");
            if (close(pfd[1]))
                errExit("close");
        }

        execlp("ls", "ls", NULL);
        errExit("exec");
    default:
        break;
    }

    switch (fork())
    {
    case -1:
        errExit("fork");
    case 0:
        if (close(pfd[1]))
            errExit("close");
        if (pfd[0] != STDIN_FILENO)
        {
            if (dup2(pfd[0], STDIN_FILENO) == -1)
                errExit("dup2");
            if (close(pfd[0]))
                errExit("close");
        }

        execlp("wc", "wc", NULL);
        errExit("exec");
    default:
        break;
    }
    if (close(pfd[0]))
        errExit("close");
    if (close(pfd[1]))
        errExit("close");
    if (wait(NULL) == -1)
        errExit("wait");
    if (wait(NULL) == -1)
        errExit("wait");
    exit(EXIT_SUCCESS);
}