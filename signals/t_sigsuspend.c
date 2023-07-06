/*
 * 《TLPI》程序清单22-5，探究sigsuspend的使用
 * */


#include <string.h>
#include <signal.h>
#include <time.h>

#include "signal_functions.h"
#include "tlsp_hdr.h"

static volatile sig_atomic_t gotSigquit = 0;

static void
handler(int sig)
{
    printf("Caught signal %d (%s)\n", sig, strsignal(sig));

    if (sig == SIGQUIT)
        gotSigquit = 1;
}

int main(int argc, char **argv)
{
    int loopNum;
    time_t startTime;
    sigset_t origMask, blockMask;
    struct sigaction sa;

    printSigMask(stdout, "Initial signal mask is:\n");

    sigemptyset(&blockMask);
    sigaddset(&blockMask, SIGINT);
    sigaddset(&blockMask, SIGQUIT);
    if (sigprocmask(SIG_BLOCK, &blockMask, &origMask) == -1)
        errExit("sigprocmask - SIG_BLOCK");

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = handler;
    if (sigaction(SIGINT, &sa, NULL) == -1)
        errExit("sigaction");
    if (sigaction(SIGQUIT, &sa, NULL) == -1)
        errExit("sigaction");

    for (loopNum = 1; !gotSigquit; ++loopNum)
    {
        printf("== LOOP %d\n", loopNum);

        // Simulate a critical section by delaying a few seconds
        printSigMask(stdout, "Starting critical section, signal mask is: \n");
        for (startTime = time(NULL); time(NULL) < startTime + 4;)
            continue;
        printPendingSigs(stdout, "Before sigsuspend() - pending signals:\n");
        if (sigsuspend(&origMask) == -1 && errno != EINTR)
            errExit("sigsuspend");
    }

    if (sigprocmask(SIG_SETMASK, &origMask, NULL) == -1)
        errExit("sigprocmask - SIG_SETMASK");

    printSigMask(stdout, "=== Exited loop\nRestored signal mask to:\n");

    exit(EXIT_SUCCESS);
}