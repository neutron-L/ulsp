#include <signal.h>
#include <libgen.h>
#include "tlsp_hdr.h"

#define CMD_SIZE 200

static volatile sig_atomic_t childExit = 0;

static void
handler(int sig)
{
    childExit = 1;
}

int 
main(int argc, char **argv)
{
    char cmd[CMD_SIZE];
    pid_t childPid;

    setbuf(stdout, NULL);
    printf("Parent PID=%ld\n", (long)getpid());

    // wait child exit, as wait SIGCHLD

    // block SIGCHLD
    sigset_t blockMask, origMask, emptyMask;
    sigemptyset(&blockMask);
    sigaddset(&blockMask, SIGCHLD);
    if (sigprocmask(SIG_SETMASK, &blockMask, &origMask) == -1)
        errExit("sigprocmask - SIG_SETMASK");

    // sigaction SIGCHLD handler
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = handler;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
        errExit("sigprocmask");

    switch (childPid = fork())
    {
    case -1:
        errExit("fork");
    case 0: /* Child: immediately exits to becom zombie */
        printf("Child (PID=%ld) exiting\n", (long)getpid());
        _exit(EXIT_SUCCESS);
    default:
        // sleep(3); // Give child a chance to start and exit

        sigemptyset(&emptyMask);
        while (!childExit)
            sigsuspend(&emptyMask);
        printf("Parent continue running...\n");
        if (sigprocmask(SIG_SETMASK, &origMask, NULL) == -1)
            errExit("sigprocmask");

        snprintf(cmd, CMD_SIZE, "ps | grep %s", basename(argv[0]));
        cmd[CMD_SIZE - 1] = '\0';

        // Now send the "sure kill" signal to the zombie
        if (kill(childPid, SIGKILL) == -1)
            errMsg("kill");
        sleep(3);
        printf("After sending SIGKILL to zombie (PID=%ld):\n", (long)childPid);
        system(cmd);

        exit(EXIT_SUCCESS);
    }
}