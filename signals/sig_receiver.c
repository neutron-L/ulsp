/*
 * 这个程序和sig_sender.c是配套的，两个程序可以同时运行，以探究信号pending的现象
 * 原理：发送程序会在每次获得调度而运行时发送多个信号给接收者，然而，当接收进程运
 * 行时，传递来的信号只有一个，只会把他们中的一个标记为等待状态。
 * */

#define _GNU_SOURCE
#include <signal.h>
#include "signal_functions.h"
#include "tlsp_hdr.h"

static int sigCnt[NSIG];

static volatile sig_atomic_t gotSigint = 0; // Set nonzero if SIGINT is delivered

static void
handler(int sig)
{
    if (sig == SIGINT)
        gotSigint = 1;
    else
        ++sigCnt[sig];
}

int
main(int argc, char ** argv)
{
    printf("%s: PID is %ld\n", argv[0], (long)getpid());

    // Same handler for all signals Ignore errors
    struct sigaction act;
    act.sa_handler = handler;
    for (int i = 1; i < NSIG; ++i)
        (void)sigaction(i, &act, NULL);

    /* If a sleep time was specified, temporarily block all signals, 
        sleep (while another process sends us signals), and then
        display the mask of pending signals and unblock all signals */
    if (argc > 1)
    {
        int numSecs = getInt(argv[1], GN_GT_0, NULL);
        sigset_t pendingMask, blockingMask, emptyMask;


        sigfillset(&blockingMask);
        if (sigprocmask(SIG_SETMASK, &blockingMask, NULL) == -1)
            errExit("sigprocmask");

        printf("%s: sleeping for %d seconds\n", argv[0], numSecs);
        sleep(numSecs);

        if (sigpending(&pendingMask) == -1)
            errExit("sigpending");

        printf("%s: pending signals are: \n", argv[0]);
        printSigset(stdout, "\t\t", &pendingMask);

        sigemptyset(&emptyMask);
        if (sigprocmask(SIG_SETMASK, &emptyMask, NULL) == -1)
            errExit("sigprocmask");
    }

    // Loop until SIGINT signals
    while (!gotSigint)
        continue;

    for (int i = 1; i < NSIG; ++i)
        if (sigCnt[i])
            printf("%s: signal %d caught %d time%s\n",
                argv[0], i, sigCnt[i], (sigCnt[i] == 1) ? "" : "s");

    exit(EXIT_SUCCESS);
}