/*
 * 《TLPI》程序清单22-7，探究signalfd的使用
 * */

#include <sys/signalfd.h>
#include <signal.h>
#include "tlsp_hdr.h"

int main(int argc, char **argv)
{
    sigset_t mask;
    int sfd, j;
    struct signalfd_siginfo fdsi;
    ssize_t s;

    if (argc < 2 || !strcmp(argv[1], "--help"))
        usageErr("%s sig-num...\n", argv[0]);

    printf("%s: PID = %ld\n", argv[0], (long)getpid());

    // block the specified signals
    sigemptyset(&mask);
    for (j = 1; j < argc; ++j)
        sigaddset(&mask, atoi(argv[j]));
    
    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1)
        errExit("sigprocmask");
    
    sfd = signalfd(-1, &mask, 0);
    if (sfd == -1)
        errExit("signalfd");
    
    for (;;)
    {
        s = read(sfd, &fdsi, sizeof(struct signalfd_siginfo));
        if (s != sizeof(struct signalfd_siginfo))
            errExit("read");
        printf("%s: got signal %d(%s); ssi_pid=%d; ", argv[0], fdsi.ssi_signo, strsignal(fdsi.ssi_signo), fdsi.ssi_pid);
        if (fdsi.ssi_code == SI_QUEUE)
            printf("ssi_int = %d", fdsi.ssi_int);
        printf("\n");
    }
}
