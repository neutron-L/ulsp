#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "tlsp_hdr.h"

#define MAX_ERROR_LEN 256

static pthread_once_t once_var = PTHREAD_ONCE_INIT;
static pthread_key_t strerrorKey;

static void
dest(void *value)
{
    printf("dest: free buf %p\n", value);
    free(value);
}

static void
createKey()
{
    int s;

    s = pthread_key_create(&strerrorKey, dest);
    if (s)
        errExitEN(s, "pthread_key_create");
}

char *
strerror(int err)
{
    int s;
    char *buf;

    s = pthread_once(&once_var, createKey);
    if (s)
        errExitEN(s, "pthread_once");

    buf = pthread_getspecific(strerrorKey);
    if (buf == NULL)
    {
        buf = malloc(256);
        s = pthread_setspecific(strerrorKey, buf);
        if (s)
            errExitEN(s, "pthread_setspecific");
        printf("add (key, thread, value) = (%d, %ld, %p)\n", strerrorKey, pthread_self(), buf);
    }
    if (err == EINVAL)
        snprintf(buf, MAX_ERROR_LEN, "EINVAL\n");
    else if (err == EPERM)
        snprintf(buf, MAX_ERROR_LEN, "EPERM\n");
    else
        snprintf(buf, MAX_ERROR_LEN, "Unknown error %d", err);

    return buf;
}


static void *
threadFunc(void *arg)
{
    char *str;

    printf("Other thread about to call strerror()\n");
    str = strerror(EPERM);
    printf("Other thread: str (%p) = %s\n", str, str);
    return NULL;
    // pthread_exit(NULL);   // 会触发dest
    // exit(0);
}

int 
main(int argc, char*argv)
{
    pthread_t t;
    int s;
    char *str;

    str = strerror(EINVAL);
    printf("Main thread has called strerror()\n");

    s = pthread_create(&t, NULL, threadFunc, NULL);
    if (s)
        errExitEN(s, "pthread_create");
        
    s = pthread_join(t, NULL);
    if (s)
        errExitEN(s, "pthread_join");

    printf("Main thread: str (%p) = %s\n", str, str);

    // pthread_key_delete(strerrorKey); // 不会触发dest
    // pthread_exit(NULL);   // 会触发dest
    // exit(EXIT_SUCCESS);
    return 0;
}