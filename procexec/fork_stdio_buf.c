/*
 * 《TLPI》程序清单25-2：fork()与stdio缓冲区的交互
 * 这个简短的程序，背后蕴藏着Linux进程的内存管理、stdio缓冲。
 * fork后，子进程会拥有一份和父进程相同的内存“拷贝”，即使存在
 * 写时复制技术，但是两份内存是独立的。如果将该程序的输出重定向
 * 到文件中或者通过管道传入其他进程，则输出的数据会与直接
 * 在终端运行不同。
 * 首先，write是无缓冲IO，会直接将数据“Ciao”写入内核缓冲区。
 * fork后，父子进程拥有两份用户态的缓冲，即先前printf写入的缓
 * 冲区，此时两个缓冲区中均有“Hello world\n”，当程序执行完后，
 * 缓冲区被刷新，数据写入内核缓冲区并显示在终端上。
 * */

#include "tlsp_hdr.h"


int 
main(int argc, char **argv)
{
    printf("Hello world\n");
    write(STDOUT_FILENO, "Ciao\n", 5);

    if (fork() == -1)
        errExit("fork");

    exit(EXIT_SUCCESS);
}
