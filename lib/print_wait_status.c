#define _GNU_SOURCE
#include <string.h>
#include <sys/wait.h>
#include "print_wait_status.h"
#include "tlsp_hdr.h"

/* NOTE: The following function employs printf(), which is not
    async-signal-safe. As such, this function is also not async-
    signal-safe
*/
void
printWaitStatus(const char * msg, int status)
{
    if (msg)
        printf("%s", msg);
    
    if (WIFEXITED(status))
        printf("child exited, status=%d\n", WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
    {
        printf("child killed by signal %d (%s)", WEXITSTATUS(status), strsignal(WTERMSIG(status)));

#ifdef WCOREDUMP
    if (WCOREDUMP(status))
        printf(" (core dumped)");
#endif
    printf("\n");
    }
    else if (WIFSTOPPED(status))
    {
        printf("child stopped by signal %d (%s)\n",
            WSTOPSIG(status), strsignal(WSTOPSIG(status)));
    }
    else if (WIFCONTINUED(status))
        printf("child continued\n");
    else
        printf("what happened to this child? (status=%x)\n", (unsigned int)status);
}

