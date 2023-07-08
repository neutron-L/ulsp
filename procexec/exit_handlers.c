/*
 * 《TLPI》程序清单25-1：使用退出处理程序
 * */

#include <stdlib.h>
#include "tlsp_hdr.h"

static void
atexitFunc1()
{
    printf("atexit function 1 called\n");
}

static void
atexitFunc2()
{
    printf("atexit function 2 called\n");
}

static void
onexitFunc(int exitStatus, void *arg)
{
    printf("on_exit function called: status=%d, arg=%ld\n",
        exitStatus, (long) arg);
}

int 
main(int argc, char **argv)
{
    if (on_exit(onexitFunc, (void *)10) != 0)
        fatal("on_exit 1");
    if (atexit(atexitFunc1) != 0)
        fatal("atexit 1");
    if (atexit(atexitFunc2) != 0)
        fatal("atexit 2");
    if (on_exit(onexitFunc, (void *)20) != 0)
        fatal("on_exit 2");

    exit(2);
}