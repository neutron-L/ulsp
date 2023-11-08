/*
 * 程序清单53-5：使用sem_getvalue()获取一个信号量的值
 * */
#include <semaphore.h>
#include "tlsp_hdr.h"

int
main(int argc, char *argv[])
{
    int value;
    sem_t *sem;

    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s sem-name\n", argv[0]);
    sem = sem_open(argv[1], 0);
    if (sem == SEM_FAILED)
        errExit("sem_open");
    if (sem_getvalue(sem, &value))
        errExit("sem_getvalue");
    printf("%d\n", value);
    exit(EXIT_SUCCESS);
}