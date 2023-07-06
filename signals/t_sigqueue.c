/*
 * 《TLPI》程序清单22-2，使用sigqueue发送实时信号，和程序清单22-3：处理实时信号配合使用
 * */

#include <signal.h>
#include "tlsp_hdr.h"

int
main(int argc, char ** argv)
{
    pid_t pid;
    int sig, numSigs, j, sigData;
    union sigval sv;

    if (argc < 4 || !strcmp(argv[1], "--help"))
        usageErr("%s pid sig-num data [num-sigs]\n", argv[0]);

    printf("%s: PID is %ld, UID is %ld\n", 
        argv[0], (long)getpid(), (long)getuid());
    
    pid = getLong(argv[1], 0, "pid");
    sig = getInt(argv[2], 0, "sig-num");
    sigData = getInt(argv[3], GN_ANY_BASE, "data");
    numSigs  = (argc > 4) ? getInt(argv[4], GN_GT_0, "num-sigs") : 1;

    for (j = 0; j < numSigs; ++j)
    {
        sv.sival_int = sigData + j;
        if (sigqueue(pid, sig, sv) == -1)
            errExit("sigqueue %d", j);
    }

    exit(EXIT_SUCCESS);
}