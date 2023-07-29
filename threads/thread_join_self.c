/*
 * exercise 29-1：线程join自身
 * expect：预计是会永久阻塞
 * real: 直接返回
 * */

#include <pthread.h>
#include "tlsp_hdr.h"

int main()
{
    pthread_join(pthread_self(), NULL);
    exit(EXIT_SUCCESS);
}