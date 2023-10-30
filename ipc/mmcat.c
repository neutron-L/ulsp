/*
 * 程序清单49-1：使用mmap()创建一个私有文件映射
 * */
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "tlsp_hdr.h"

int
main(int argc, char *argv[])
{
    char * addr;
    int fd;
    struct stat sb;

    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s file\n", argv[0]);
    
    if ((fd = open(argv[1], O_RDONLY)) == -1)
        errExit("open");

    if (fstat(fd, &sb))
        errExit("fstat");
    addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (addr == MAP_FAILED)
        errExit("mmap");
    if (write(STDOUT_FILENO, addr, sb.st_size) != sb.st_size)
        fatal("partial/failed write");
    exit(EXIT_SUCCESS);
}