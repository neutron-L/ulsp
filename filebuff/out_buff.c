/*
 * 使用《TLPI》setvbuf，设置stdout的缓冲区
 * */
#include <assert.h>
#include "tlsp_hdr.h"

int
main()
{
    char buf[BUFSIZ];

    if (setvbuf(stdout, buf, _IOLBF, BUFSIZ))
        errExit("setvbuf");
    if (setvbuf(stdin, buf, _IOLBF, BUFSIZ))
        errExit("setvbuf");
    
        printf("Hello world");
    assert(strcmp(buf, "Hello world") == 0);
    fflush(stdout);
    assert(strcmp(buf, "Hello world") == 0);

    exit(EXIT_SUCCESS);
}