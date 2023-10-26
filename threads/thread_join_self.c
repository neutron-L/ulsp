/*
 * exercise 29-1：线程join自身
 * expect：预计是会永久阻塞
 * real: 直接返回，返回一个错误码但是不会修改errno，ERROR [EDEADLK/EDEADLOCK Resource deadlock avoided] pthread_join
 * */

#include <pthread.h>
#include "tlsp_hdr.h"

int main()
{
    int s;
    if ((s = pthread_join(pthread_self(), NULL)))
    {
        perror("pthread_join");
        printf("%s\n", strerror(s));
        errExitEN(s, "pthread_join\n");
    }
    
    exit(EXIT_SUCCESS);
}