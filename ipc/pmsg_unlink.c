/*
 * 程序清单52-1：使用mq_unlink()断开一个POSIX消息队列的链接
 * */
#include <mqueue.h>
#include "tlsp_hdr.h"

int
main(int argc, char *argv[])
{
    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s mq-name\n", argv[0]);
    if (mq_unlink(argv[1]) == -1)
        errExit("mq_unlink");
    exit(EXIT_SUCCESS);
}