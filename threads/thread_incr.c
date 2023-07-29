/*
 * 程序清单29-2：一个使用pthread_attr的简单程序
 * */

#include <pthread.h>
#include "tlsp_hdr.h"

static int glob = 0;

static void *
threadFunc(void *arg)
{
    int loops = *(int *)arg;
    int loc;

    while (loops--)
    {
        loc = glob;
        ++loc;
        glob = loc;
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
