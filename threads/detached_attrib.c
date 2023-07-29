/*
 * 程序清单29-2：一个使用pthread_attr的简单程序
 * */

#include <pthread.h>
#include "tlsp_hdr.h"

static void *
threadFunc(void *arg)
{
    return arg;
}

int main()
{
    pthread_t thr;
    pthread_attr_t attr;
    int s;

    s = pthread_attr_init(&attr);
    if (s != 0)
        errExitEN(s, "pthread_attr_init");
    
    s = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (s != 0)
        errExitEN(s, "pthread_attr_setdetachstate");
    
    s = pthread_create(&thr, &attr, threadFunc, (void *)1);
    if (s != 0)
        errExitEN(s, "pthread_create");
    s = pthread_attr_destroy(&attr);
    if (s != 0)
        errExitEN(s, "pthread_attr_destroy");
    s = pthread_join(thr, NULL);
    if (s != 0)
        errExitEN(s, "pthread_join");

    exit(EXIT_SUCCESS);
}