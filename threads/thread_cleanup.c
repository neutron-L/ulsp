/*
 * 程序清单32-2：使用清理函数
 * */

#include <pthread.h>
#include "tlsp_hdr.h"

static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static int glob = 0;

static void
cleanupHandler(void *arg)
{
    int s;

    printf("cleanup: freeing block at %p\n", arg);
    free(arg);

    printf("cleanup: unlocking mutex\n");
    s = pthread_mutex_unlock(&mtx);
    if (s)
        errExitEN(s, "pthread_mutex_unlock");
}

static void *
threadFunc(void *arg)
{
    (void)arg;

    int s;
    void *buf = NULL;

    buf = malloc(0x1000);
    printf("thread: allocate memory at %p\n", buf);

    s = pthread_mutex_lock(&mtx);
    if (s)
        errExitEN(s, "pthread_mutex_lock");

    pthread_cleanup_push(cleanupHandler, buf);

    while (glob == 0)
    {
        s = pthread_cond_wait(&cond, &mtx);
        if (s)
            errExitEN(s, "pthread_cond_wait");
    }

    printf("thread: condition wait loop completed\n");
    pthread_cleanup_pop(1);
    return NULL;
}

int main(int argc, char *argv[])
{
    (void)argc, (void)argv;

    pthread_t thr;
    int s;
    void *res;

    s = pthread_create(&thr, NULL, threadFunc, NULL);
    if (s)
        errExitEN(s, "pthread_create");

    if (argc == 1)
    {
        printf("main: about to cancel thread\n");
        s = pthread_cancel(thr);
        if (s)
            errExitEN(s, "pthread_cancel");
    }
    else
    {
        sleep(2);
        printf("main: about to signal condition variable\n");
        s = pthread_mutex_lock(&mtx);
        if (s)
            errExitEN(s, "pthread_mutex_lock");
        glob = 1;
        s = pthread_mutex_unlock(&mtx);
        if (s)
            errExitEN(s, "pthread_mutex_unlock");
        s = pthread_cond_signal(&cond);
        if (s != 0)
            errExitEN(s, "pthread_cond_signal");
    }

    s = pthread_join(thr, &res);
    if (s)
        errExitEN(s, "pthread_join");

    if (res == PTHREAD_CANCELED)
        printf("Thread was canceled\n");
    else
        printf("Thread terminated normally\n");
    exit(EXIT_SUCCESS);
}
