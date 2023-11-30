/*
 * 程序清单54-1：创建一个POSIX共享内存对象
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
    fd = shm_open(argv[1], );

    exit(EXIT_SUCCESS);
}
