/*
 * 程序清单54-2：将数据复制到一个POSIX共享内存对象
 * */
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "tlsp_hdr.h"

int main(int argc, char * argv[])
{
    int fd;
    struct stat sb;
    char * addr;
    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s shm-name\n", argv[0]);
    fd = shm_open(argv[1], O_RDWR, 0);
    if (fd == -1)
        errExit("shm_open");

    if (fstat(fd, &sb))
        errExit("fstat");

    addr = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED)
        errExit("mmap");
    if (close(fd) == -1)
        errExit("close");
    write(STDOUT_FILENO, addr, sb.st_size);

    exit(EXIT_SUCCESS);
}
