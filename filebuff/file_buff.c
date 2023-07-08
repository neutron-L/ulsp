/*
 * 使用《TLPI》setvbuf，设置文件io流的缓冲区
 * 以读写的模式打开一个文件，并为其设置缓冲区，
 * 该缓冲区是读写共用的，如果写入的数据还未被
 * flush到内核缓冲区，此时其他进程查看到的文件
 * 内容是旧的。因为用户空间的缓冲区不像内核缓冲
 * 区，内核会维护一个“一致”的状态。
 * 如果相邻的两次操作不同:
 * case 1: 输出+输入，则先flush，将输出数据flush到内核缓冲区，
 *  输入时读取新的数据覆盖掉缓冲区中缓冲的输出的数据
 * case 2: 输入+输出，输出的数据覆写输入缓冲区的部分
 * */
#include <assert.h>
#include "tlsp_hdr.h"

int
main()
{
    char buf[BUFSIZ];
    char in[64];
    
    FILE * fio = fopen("foo", "r+");

    if (!fio)
    errExit("fopen");

    if (setvbuf(fio, buf, _IOLBF, BUFSIZ))
        errExit("setvbuf");

    // 输出操作
    fprintf(fio, "abc");
    printf("buf: %s\n", buf);
    // fflush(fio);
    system("cat foo");

    // 输入操作
    // 重定位
    // fseek(fio, 0, SEEK_SET);
    fscanf(fio, "%c", in);
    system("cat foo");
    printf("in: %c\n", in[0]);
    printf("buf: %s\n", buf);

    // fflush(fio);
    fprintf(fio, "de");
    printf("buf: %s\n", buf);
    system("cat foo");

    // 输入操作
    fscanf(fio, "%c", in);
    printf("in: %c\n", in[0]);
    printf("buf: %s\n", buf);

    // 再次输入操作
    fscanf(fio, "%c", in);
    printf("in: %c\n", in[0]);
    printf("buf: %s\n", buf);


    // 再次输出操作
    fprintf(fio, "kni");
    printf("buf: %s\n", buf);

    system("cat foo");
    fclose(fio);
    system("cat foo");

    exit(EXIT_SUCCESS);
}