/*
 * 程序清单32-1：调用pthread_cancel()取消线程
 * */

#include <pthread.h>
#include "tlsp_hdr.h"

static void *
threadFunc(void *arg)
{
    int j;
    printf("New thread started\n"); // May be a cancellation point
    for (j = 1; ; ++j)
    {
        printf("Loop %d\n", j);     // May be a cancellation point
        sleep(1);                   // A cancellation point
    }

    /* NOTEREACED */
    return NULL;
}

int 
main(int argc, char *argv[])
{
    pthread_t thr;
    int s;
    void *res;

    s = pthread_create(&thr, NULL, threadFunc, NULL);
    if (s)
        errExitEN(s, "pthread_create");
    
    sleep(3);

    s = pthread_cancel(thr);
    if (s)
        errExitEN(s, "pthread_cancel");
    
    s = pthread_join(thr, &res);
    if (s)
        errExitEN(s, "pthread_cancel");

    if (res == PTHREAD_CANCELED)
        printf("Thread was canceled\n");
    else
        printf("Thread was not canceled (should not happen)\n");
    exit(EXIT_SUCCESS);
}
