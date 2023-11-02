/*
 * 程序清单50-2：验证madvise()MADV_DONTNEED操作对MAP_PRIVATE映射的影响
 * */
#include <fcntl.h>
#include <sys/mman.h>
#include "tlsp_hdr.h"

#define LEN 32
int
main(int argc, char *argv[])
{
    char * addr;
    int fd;

    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s file\n", argv[0]);

    if ((fd = open(argv[1], O_RDONLY)) == -1)
        errExit("open");

    /* 私有文件映射，写入映射区域后再使用madvise()，下一次读取映射区域时用文件内容初始化 */
    printf("============Private file mmaped===========\n");
    addr = mmap(NULL, 32, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (addr == MAP_FAILED)
        errExit("mmap");
    printf("Before madvise read: \n");
    for (int i = 0; i < LEN; ++i)
        printf("%c ", addr[i]);
    printf("\n");
    printf("Before madvise write: \n");
    for (int i = 0; i < LEN; ++i)
        addr[i] = 'a' + i % 26;
    printf("\n");
    printf("Before madvise read: \n");
    for (int i = 0; i < LEN; ++i)
        printf("%c ", addr[i]);
    printf("\n");
    madvise(addr, LEN / 2, MADV_DONTNEED); // LEN会被向上舍入到分页大小，因此整个LEN都会被madvise影响
    printf("After madvise read: \n");
    for (int i = 0; i < LEN; ++i)
        printf("%c ", addr[i]);
    printf("\n");

    munmap(addr, LEN);
    close(fd);

    /* 私有匿名映射，写入映射区域后再使用madvise()，下一次读取映射区域时用0内容初始化 */
    printf("============Private anon mmaped===========\n");
    addr = mmap(NULL, 32, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    if (addr == MAP_FAILED)
        errExit("mmap");
    printf("Before madvise read: \n");
    for (int i = 0; i < LEN; ++i)
        printf("%d ", (int)addr[i]);
    printf("\n");
    printf("Before madvise write: \n");
    for (int i = 0; i < LEN; ++i)
        addr[i] = 'a' + i % 26;
    printf("\n");
    printf("Before madvise read: \n");
    for (int i = 0; i < LEN; ++i)
        printf("%c ", addr[i]);
    printf("\n");
    madvise(addr, LEN / 2, MADV_DONTNEED); // LEN会被向上舍入到分页大小，因此整个LEN都会被madvise影响
    printf("After madvise read: \n");
    for (int i = 0; i < LEN; ++i)
        printf("%d ", (int)addr[i]);
    printf("\n");
    munmap(addr, LEN);


    exit(EXIT_SUCCESS);
}