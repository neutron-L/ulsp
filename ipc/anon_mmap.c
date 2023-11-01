/*
 * 程序清单49-3：在父子进程之间共享匿名映射
 * */
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "tlsp_hdr.h"

int main(int argc, char *argv[])
{
    int *addr;

    addr = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
                MAP_SHARED | MAP_ANON, -1, 0);
    if (addr == MAP_FAILED)
        errExit("mmap");
    *addr = 1;

    switch (fork())
    {
    case -1:
        errExit("fork");
        break;
    case 0:
        printf("Child started, value=%d\n", *addr);
        (*addr)++;

        if (munmap(addr, sizeof(int)) == -1)
            errExit("munmap");
        exit(EXIT_SUCCESS);
    default:
        if (wait(NULL) == -1)
            errExit("wait");
        printf("In parent , value = %d\n", *addr);
        if (munmap(addr, sizeof(int)) == -1)
            errExit("munmap");
        exit(EXIT_SUCCESS);
    }
}