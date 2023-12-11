/*
 * 使用《TLPI》setvbuf，设置stdout的缓冲区
 * */
#include <assert.h>
#include "tlsp_hdr.h"

int main()
{
    char buf[BUFSIZ];

    if (setvbuf(stdout, buf, _IOLBF, BUFSIZ))
        errExit("setvbuf");
    fprintf(stderr, "output without \\n: \n");
    printf("Hello world");

    fflush(stdout);
    sleep(3);
    fprintf(stderr, "buf: %s\n", buf);
    fprintf(stderr, "output line with \\n: \n");
    printf(" and new\n");
    fprintf(stderr, "buf: %s\n", buf);
    printf("new start\n");
    fprintf(stderr, "buf: %s\n", buf);
    fprintf(stderr, "flush buf: %s\n", buf);
    fflush(stdout);
    exit(EXIT_SUCCESS);
}