/*
 * 测试、练习使用《unix/linux系统编程手册》20.9的一些信号集运算接口
 * 包括sigemptyset, sigfillset
 * sigaddset sigdelset
 * sigismember
 * sigandset sigorset
 * sigisemptyset
 * */

#include <signal.h>
#include "tlsp_hdr.h"
#include "signal_functions.h"

int main()
{
    sigset_t sig1, sig2;

    // Empty sig1
    sigemptyset(&sig1);

    printf("Sigset 1:\n");
    printSigset(stdout, "", &sig1);
    printf("is empty: %d\n", sigisemptyset(&sig1));

    // Fill sig2
    printf("Sigset 2:\n");
    sigfillset(&sig2);
    // 通过打印出来的信息显示，1-31是传统或标准的信号，往后是实时信号
    printSigset(stdout, "", &sig2);

    sigemptyset(&sig2);

    // sig1: SIGABRT SIGALARM SIGBUF
    int sigset1[] = {SIGABRT, SIGALRM, SIGBUS};

    printf("Sigset 1:\n");
    for (int i = 0; i < sizeof(sigset1) / sizeof(sigset1[0]); ++i)
        sigaddset(&sig1, sigset1[i]);

    printSigset(stdout, "", &sig1);
    for (int i = 0; i < sizeof(sigset1) / sizeof(sigset1[0]); ++i)
        printf("%d(%s) Is Member: %d\n", sigset1[i], strsignal(sigset1[i]), sigismember(&sig1, sigset1[i]));

    // sig2: SIGFPE SIGILL SIGINT
    int sigset2[] = {SIGFPE, SIGILL, SIGINT};

    printf("Sigset 2:\n");
    for (int i = 0; i < sizeof(sigset2) / sizeof(sigset2[0]); ++i)
        sigaddset(&sig2, sigset2[i]);

    printSigset(stdout, "", &sig2);
    for (int i = 0; i < sizeof(sigset2) / sizeof(sigset2[0]); ++i)
        printf("%d(%s) Is Member: %d\n", sigset2[i], strsignal(sigset2[i]), sigismember(&sig2, sigset2[i]));

    int s = SIGINT;
    // add
    printf("sig1 add SIGINT: \n");
    sigaddset(&sig1, s);
    printf("%d(%s) Is Member: %d\n", s, strsignal(s), sigismember(&sig1, s));
    s = SIGFPE;
    printf("%d(%s) Is Member: %d\n", s, strsignal(s), sigismember(&sig1, s));
    printf("sig1 add SIGFPE: \n");
    sigaddset(&sig1, s);
    printf("%d(%s) Is Member: %d\n", s, strsignal(s), sigismember(&sig1, s));

    // del
    s = SIGCHLD;
    printf("sig2 add SIGCHLD: \n");
    sigaddset(&sig1, s);
    printf("%d(%s) Is Member: %d\n", s, strsignal(s), sigismember(&sig1, s));
    printf("sig2 del SIGCHLD: \n");

    sigdelset(&sig1, s);
    printf("%d(%s) Is Member: %d\n", s, strsignal(s), sigismember(&sig1, s));

    sigset_t res;
    // and
    printf("sig1 & sig2: \n"); // (2 6 7 8 14) & (2 4 8) = (2 8)
    sigandset(&res, &sig1, &sig2);
    printSigset(stdout, "", &res);
    // or
    printf("sig1 ^ sig2: \n"); // (2 6 7 8 14) ^ (2 4 8) = (2 4 6 7 8 14)
    sigorset(&res, &sig1, &sig2);
    printSigset(stdout, "", &res);


    exit(EXIT_SUCCESS);
}