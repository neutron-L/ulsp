/*
 * 程序清单29-1：一个使用pthreads的简单程序
 * */

#include <pthread.h>
#include "tlsp_hdr.h"

static void *
threadFunc(void *arg)
{
    char * s = (char *)arg;
    printf("%s", s);

    return (void *)strlen(s);
}

int main()
{
    pthread_t t1;
    void * res;
    int s;

    s = pthread_create(&t1, NULL, threadFunc, "Hello threads\n");

    if (s != 0)
        errExitEN(s, "pthread_create");
    // sleep(1);
    
    printf("Message from main()\n");
    s = pthread_join(t1, &res);

    if (s != 0)
        errExitEN(s, "pthread_join");
    printf("Thread returned %ld\n", (long)res);

    exit(EXIT_SUCCESS);
}