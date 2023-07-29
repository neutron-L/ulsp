/*
 * 程序清单30-4：可连接任意已经终止线程的主线程
 * */

#include <pthread.h>
#include "tlsp_hdr.h"

static pthread_cond_t threadDied = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t threadMutex = PTHREAD_MUTEX_INITIALIZER;

static int totThreads = 0;
static int numLive = 0;

static int numUnjoined = 0;

enum tstate
{
    TS_ALIVE,
    TS_TERMINATED,
    TS_JOINED
};

static struct
{
    pthread_t tid;
    enum tstate state;
    int sleepTime;
} *thread;

static void *
threadFunc(void *arg)
{
    int idx = *(int *)arg;
    int s;
    sleep(thread[idx].sleepTime);

    printf("Thread [%d] terminating\n", idx);

    s = pthread_mutex_lock(&threadMutex);
    if (s != 0)
        errExitEN(s, "pthread_mutex_lock");

    ++numUnjoined;
    thread[idx].state = TS_TERMINATED;

    s = pthread_mutex_unlock(&threadMutex);
    if (s != 0)
        errExitEN(s, "pthread_mutex_unlock");
    s = pthread_cond_signal(&threadDied);
    if (s != 0)
        errExitEN(s, "pthread_mutex_signal");
    free(arg);

    return NULL;
}

int main(int argc, char **argv)
{
    int s, idx;

    if (argc < 2 || !strcmp(argv[1], "--help"))
        usageErr("%s nsecs...\n", argv[0]);

    thread = calloc(argc - 1, sizeof(*thread));
    if (thread == NULL)
        errExit("calloc");

    // Create all threads
    for (idx = 0; idx < argc - 1; ++idx)
    {
        thread[idx].sleepTime = getInt(argv[idx + 1], GN_NONNEG, NULL);
        thread[idx].state = TS_ALIVE;

        // allocate idx
        int * p = malloc(sizeof(int));
        *p = idx;
        s = pthread_create(&thread[idx].tid, NULL, threadFunc, p);
        if (s != 0)
            errExitEN(s, "pthread_create");
    }

    numLive = totThreads = argc - 1;

    // Join with terminated threads
    while (numLive)
    {
        s = pthread_mutex_lock(&threadMutex);
        if (s)
            errExitEN(s, "pthread_mutex_lock");

        while (numUnjoined == 0)
        {
            s = pthread_cond_wait(&threadDied, &threadMutex);
            if (s)
                errExitEN(s, "pthread_cond_wait");
        }

        for (idx = 0; idx < totThreads; ++idx)
        {
            if (thread[idx].state == TS_TERMINATED)
            {
                s = pthread_join(thread[idx].tid, NULL);
                if (s != 0)
                    errExitEN(s, "pthread_join");
                
                thread[idx].state = TS_JOINED;
                --numLive;
                --numUnjoined;

                printf("Reaped thread %d (numLive=%d)\n", idx, numLive);
            }
        }

        s = pthread_mutex_unlock(&threadMutex);
        if (s)
            errExitEN(s, "pthread_mutex_unlock");
    }

    exit(EXIT_SUCCESS);
}