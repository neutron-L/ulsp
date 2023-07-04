/*
 * 
 * */
#define __USE_XOPEN 
#include <crypt.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "tlsp_hdr.h"

static char * str2;
static int handled = 0;

static void
handler(int sig)
{
    (void)sig;
    crypt(str2, "xx");
    ++handled;
}

int 
main(int argc, char ** argv)
{
    char * cr1;
    int callNum, mismatch;
    struct sigaction sa;

    if (argc != 3)
        usageErr("%s str1 str2\n", argv[0]);

    str2 = argv[2];
    cr1 = strdup(crypt(argv[1], "xx"));

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = handler;
    if (sigaction(SIGINT, &sa, NULL) == -1)
        errExit("sigaction");
    
    /* Repeatly call crypt() using argv[1]. If interrupted by
     a signal handler, then the static storage returned by crypt()
     will be overwritten by the results of encrypting argv[2], and
     strcmp() will detect a mismatch with the value in 'cr1 .*/
    for (callNum = 1, mismatch = 0; ; ++callNum)
    {
        if (strcmp(crypt(argv[1], "xx"), cr1))
        {
            ++mismatch;
            printf("Mismatch on call %d (mismatch=%d handled=%d)\n",
                callNum, mismatch, handled);
        }
    }

    exit(EXIT_SUCCESS);
}

