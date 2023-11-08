/*
 * 程序清单53-6：使用s一个POSIX未命名信号量保护全局变量的访问
 * */
#include <semaphore.h>
#include <pthread.h>
#include "tlsp_hdr.h"

static int glob = 0;
static sem_t sem;

static void *
threadFunc(void *arg)
{
    int loops = *((int *)arg);

    int loc, j;
    for (j = 0; j < loops; ++j)
    {
        if (sem_wait(&sem))
            errExit("sem_wait");
        loc = glob;
        ++loc;
        glob = loc;
        if (sem_post(&sem))
            errExit("sem_post");
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t t1, t2;
    int loops, s;

    loops = (argc > 1) ? getInt(argv[1], GN_GT_0, "num-loops") : 10000000;

    if (sem_init(&sem, 0, 1))
        errExit("sem_init");

    s = pthread_create(&t1, NULL, threadFunc, &loops);
    if (s)
        errExitEN(s, "pthread_create");
    s = pthread_create(&t2, NULL, threadFunc, &loops);
    if (s)
        errExitEN(s, "pthread_create");

    s = pthread_join(t1, NULL);
    if (s)
        errExitEN(s, "pthread_join");
    s = pthread_join(t2, NULL);
    if (s)
        errExitEN(s, "pthread_join");
    printf("glob=%d\n", glob);
    exit(EXIT_SUCCESS);
}