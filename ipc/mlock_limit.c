/*
 * 习题50-1：验证RLIMT_MEMLOCK资源限制的作用
 * */

#include <stdio.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include "tlsp_hdr.h"

long pageSize;

static void
displayMincore(char *addr, size_t length)
{
    unsigned char *vec;
    long numPages, j;

    addr = (char *)(((unsigned long)addr + pageSize - 1) / pageSize * pageSize);
    numPages = (length + pageSize - 1) / pageSize;
    vec = malloc(numPages);
    if (vec == NULL)
        errExit("malloc");
    if (mincore(addr, length, vec) == -1)
        errExit("mincore");

    for (j = 0; j < numPages; ++j)
    {
        if (j % 64 == 0)
            printf("%s%10p: ", (j == 0) ? "" : "\n", addr + j * pageSize);
        printf("%c", (vec[j] & 1) ? '*' : '.');
    }
    printf("\n");
    free(vec);
}


int main() {
    struct rlimit rlim;
    int resource = RLIMIT_MEMLOCK; // 资源类型（此处示例为内存锁定）

    pageSize = sysconf(_SC_PAGESIZE);

    // 获取资源限制
    if (getrlimit(resource, &rlim) == 0) {
        printf("Soft limit: %lld\n", (long long)rlim.rlim_cur); // 软限制
        printf("Hard limit: %lld\n", (long long)rlim.rlim_max); // 硬限制
    } else {
        perror("getrlimit");
    }
    if (mlockall(MCL_FUTURE) == -1)
        errExit("mlockall");
    char * addr;

    addr = malloc(rlim.rlim_cur / pageSize * pageSize);
    if (addr == NULL)
        errExit("malloc");
    displayMincore(addr, rlim.rlim_cur/2);
    free(addr);
    addr = malloc(rlim.rlim_max  + 102);
    if (addr == NULL)
        errExit("malloc");
    return 0;
}
