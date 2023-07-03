/*
 * 这是exercise 20.16的20-2，使用sigaction修改某阻塞信号的处理函
 * 数，当该信号解除阻塞后，会执行新的处理函数
 * */

#include <signal.h>
#include "signal_functions.h"
#include "tlsp_hdr.h"

static volatile sig_atomic_t gotSigint = 0; // Set nonzero if SIGINT is delivered


static void
handler(int sig)
{
    if (sig != SIGINT)
        printf("Caught %d(%s)\n", sig, strsignal(sig));
    else
        gotSigint = 1;
}

static void
new_handler(int sig)
{
    printf("New handler for sig %d\n", sig);
}

int
main(int argc, char ** argv)
{
    // Check arguments
    if (argc != 2 || !strcmp(argv[1], "--help"))
        usageErr("%s secs\n", argv[0]);

    printf("%s: PID is %ld\n", argv[0], (long)getpid());

    struct sigaction act;

    act.sa_handler = handler;

    (void)sigaction(SIGUSR1, &act, NULL);
    (void)sigaction(SIGUSR2, &act, NULL);
    (void)sigaction(SIGINT, &act, NULL);

    sigset_t blockingSig, pendingSig, emptySig;
    // block 
    sigfillset(&blockingSig);
    if (sigprocmask(SIG_BLOCK, &blockingSig, NULL) == -1)
        errExit("sigprocmask");

    // sleep
    int secs = getInt(argv[1], GN_GT_0, NULL);
    sleep(secs);

    // show pending signal
    if (sigpending(&pendingSig) == -1)
        errExit("sigpendingset");
    
    printf("%s: pending signals are: \n", argv[0]);
    printSigset(stdout, "\t\t", &pendingSig);

    if (sigismember(&pendingSig, SIGUSR1))
    {
        act.sa_handler = new_handler;
        (void)sigaction(SIGUSR1, &act, NULL);
        // printf("Change %d handler\n", SIGUSR1);
    }
    if (sigismember(&pendingSig, SIGUSR2))
    {
        act.sa_handler = SIG_IGN;
        (void)sigaction(SIGUSR2, &act, NULL);
        // printf("Change %d handler\n", SIGUSR1);
    }
    
    // unblock
    sigemptyset(&emptySig);
    if (sigprocmask(SIG_SETMASK, &emptySig, NULL) == -1)
        errExit("sigprocmask");

    while (!gotSigint)
        continue;

    exit(EXIT_SUCCESS);
}