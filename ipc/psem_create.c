/*
 * 程序清单53-1：使用msem_open()打开或创建一个POSIX命名信号量
 * */
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "tlsp_hdr.h"

int main(int argc, char *argv[])
{
    int flags, opt;
    mode_t perms;
    unsigned int value;
    sem_t *sem;

    flags = 0;
    while ((opt = getopt(argc, argv, "cx")) != -1)
    {
        switch (opt)
        {
        case 'c':
            /* code */
            flags |= O_CREAT;
            break;
        case 'x':
            flags |= O_EXCL;
            break;
        default:
            break;
        }
    }

    if (argc <= optind)
        usageErr(argv[0]);

    perms = (argc <= optind + 1) ? (S_IRUSR | S_IWUSR) : getInt(argv[optind + 1], GN_BASE_8, "octal-perms");
    value = (argc <= optind + 2) ? 0 : getInt(argv[optind + 2], 0, "value");

    sem = sem_open(argv[optind], flags, perms, value);
    if (sem == SEM_FAILED)
        errExit("sem_open");
    exit(EXIT_SUCCESS);
}