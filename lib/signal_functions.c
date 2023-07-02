#define _GNU_SOURCE
#include <string.h>
#include <signal.h>
#include "signal_functions.h"
#include "tlsp_hdr.h"

/* NOTE: All of the following functions employ fprintf(), which
    is not async-signal-safe. As such, these functions are also not async-signal-safe */

void // Print list of signals within a signal set
printSigset(FILE *of, const char *prefix, const sigset_t *sigset)
{
    int sig, cnt;

    cnt = 0;
    for (sig = 1; sig < NSIG; ++sig)
    {
        if (sigismember(sigset, sig))
        {
            ++cnt;
            fprintf(of, "%s%d (%s)\n", prefix, sig, strsignal(sig));
        }
    }

    if (!cnt)
        fprintf(of, "%s<empty signal set>\n", prefix);
}

int printSigMask(FILE *of, const char *msg)
{
    sigset_t currMask;

    if (msg != NULL)
        fprintf(of, "%s", msg);
    if (sigprocmask(SIG_SETMASK, NULL, &currMask) == -1)
        return -1;

    printSigset(of, "\t\t", &currMask);

    return 0;
}

int printPendingSigs(FILE *of, const char *msg)
{
    sigset_t pendingSigs;

    if (msg != NULL)
        fprintf(of, "%s", msg);
    if (sigpending(&pendingSigs) == -1)
        return -1;

    printSigset(of, "\t\t", &pendingSigs);

    return 0;
}