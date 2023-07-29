/*
 * 程序清单30-2：使用mutex同步对全局变量的访问
 * */


#include <pthread.h>
#include "tlsp_hdr.h"

static int glob = 0;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

static void *
threadFunc(void *arg)
{
    int loops = *(int *)arg;
    int loc, s;

    while (loops--)
    {
        if (s = pthread_mutex_lock(&mtx))
            errExitEN(s, "pthread_mutex_lock");
        loc = glob;
        ++loc;
        glob = loc;

        if (s = pthread_mutex_unlock(&mtx))
            errExitEN(s, "pthread_mutex_unlock");
    }

    return NULL;
}

int main(int argc, char **argv)
{
    pthread_t t1, t2;
    int loops, s;

    loops = (argc > 1) ? getInt(argv[1], GN_GT_0, "num-loops") : 1000;

    s = pthread_create(&t1, NULL, threadFunc, &loops);
    if (s)
        errExitEN(s, "pthread_create");
    s = pthread_create(&t2, NULL, threadFunc, &loops);
    if (s)
        errExitEN(s, "pthread_create");

    s = pthread_join(t1, NULL);
    if (s)
        errExitEN(s, "pthread_create");
    s = pthread_join(t2, NULL);
    if (s)
        errExitEN(s, "pthread_create");

    printf("glob = %d\n", glob);

    exit(EXIT_SUCCESS);
}
