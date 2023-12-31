/*
 * 程序清单54-2：将数据复制到一个POSIX共享内存对象
 * */
#include <fcntl.h>
#include <sys/mman.h>
#include "tlsp_hdr.h"

int main(int argc, char * argv[])
{
    int fd;
    size_t len;
    char * addr;
    if (argc != 3 || strcmp(argv[1], "--help") == 0)
        usageErr("%s shm-name string\n", argv[0]);
    fd = shm_open(argv[1], O_RDWR, 0);
    if (fd == -1)
        errExit("shm_open");

    len = strlen(argv[2]);
    if (ftruncate(fd, len))
        errExit("ftruncate");
    printf("Resized to %ld bytes\n", (long)len);

    addr = mmap(NULL, len, PROT_READ | PROT_WRITE , MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED)
        errExit("mmap");
    if (close(fd) == -1)
        errExit("close");
    printf("copying %ld bytes\n", (long)len);
    memcpy(addr, argv[2], len);

    exit(EXIT_SUCCESS);
}
