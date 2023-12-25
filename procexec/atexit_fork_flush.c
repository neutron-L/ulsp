#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

void fun1()
{
    printf("%ld: call fun1\n", getpid());
}


void fun2()
{
    printf("%ld: call fun2\n", getpid());
}

int main()
{
    pid_t cpid;

    atexit(fun1);
    atexit(fun2);
    cpid=fork();
    if (cpid < 0)
    {
        perror("fork");
        exit(1);
    }
    else if (cpid == 0)
    {
        printf("Child %ld\n", getpid());
        fflush(stdout);
        _exit(0);
        // exit(0);
    }
    else
    {
        printf("Parent %ld\n", getpid());
        sleep(3);
    }
    exit(0);
}