/*
 * 这是exercise 20.16的20-3，验证sigaction的
 * SA_RESETHAND以及SA_NODEFER标志的效果
 * */

#include <signal.h>
#include "signal_functions.h"
#include "tlsp_hdr.h"

static void
handler(int sig)
{
    printf("Caught %d(%s)\n", sig, strsignal(sig));
}


int
main(int argc, char ** argv)
{
    printf("%s: PID is %ld\n", argv[0], (long)getpid());

    struct sigaction act;

    act.sa_handler = handler;
    act.sa_flags = SA_RESETHAND;
    (void)sigaction(SIGINT, &act, NULL);

    for (int i = 0; ; ++i)
    {
        sleep(1);
        printf("now is %d\n", i);
    }

    exit(EXIT_SUCCESS);
}