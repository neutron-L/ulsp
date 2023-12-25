#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

void fun1()
{
    printf("%ld: call fun1\n", getpid());
    
}


void fun2()
{
    printf("%ld: call fun2\n", getpid());
    // printf("call _exit, and I am the last function called\n");
    // _exit(0);
    raise(SIGINT);
}


void fun3()
{
    printf("%ld: call fun3\n", getpid());
}

int main()
{
    pid_t cpid;

    atexit(fun1);
    atexit(fun2);
    atexit(fun3);
    
    exit(0);
}