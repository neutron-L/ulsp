/*
 * 程序清单54-1：创建一个POSIX共享内存对象
 * */
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "tlsp_hdr.h"

static void
usageError(const char * progName)
{
    fprintf(stderr, "Usage: %s [-cx] name size [octal-perms]\n", progName);
    fprintf(stderr, "   -c  Create shared memory (O_CREAT)\n");
    fprintf(stderr, "   -x  Create exclusively (O_EXCL)\n");
    exit(EXIT_FAILURE);
}

int 
main(int argc, char * argv[])
{
    int flags, opt ,fd;
    mode_t perms;
    size_t size;
    void *addr;

    flags = O_RDWR;
    while ((opt = getopt(argc, argv, "cx")) != -1)
    {
        switch (opt)
        {
        case 'c':
            /* code */
            flags |= O_CREAT; 
            break;
        case 'x':
            /* code */
            flags |= O_EXCL; 
            break;
        
        default:
            usageError(argv[0]);
        }
    }
    if (optind + 1 >= argc)
        usageError(argv[0]);
    size = getLong(argv[optind + 1], GN_ANY_BASE, "size");
    perms = (argc <= optind + 2) ? (S_IRUSR | S_IWUSR) : 
    getLong(argv[optind + 2], GN_BASE_8, "octal-perms");

    fd = shm_open(argv[optind], flags, perms);
    if (fd == -1)
        errExit("shm_open");
    if (ftruncate(fd, size) == -1)
        errExit("ftruncate");
    
    addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED)
        errExit("mmap");
    write(STDOUT_FILENO, addr, 5);
    memcpy(addr, "world", 5);
    write(STDOUT_FILENO, addr, 5);
    
    exit(EXIT_SUCCESS);
}