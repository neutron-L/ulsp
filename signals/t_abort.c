/*
 * 实现abort函数，以及注册SIGABRT信号的处理函数
 * */

#include <signal.h>
#include "tlsp_hdr.h"

static void
my_abort()
{
    raise(SIGABRT);
    exit(134);
}


static void
handler(int sig)
{
    printf("Caught sig %d(%s)\n", sig, strsignal(sig));
}

int main()
{
    // register SIGABRT handler
    if (signal(SIGABRT, handler) == -1)
        errExit("signal");

    // my_abort();
    abort();
}