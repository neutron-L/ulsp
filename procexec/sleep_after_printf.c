/*
 * 《TLPI》exercisde 27-6. 原因应该是：printf将待输出的数据
 * 放入用户缓冲区，执行execlp后，进程的内存映像被重置（代码段重新加载，
 * 数据段、栈段、堆段也重新从可执行文件加载），因此丢失了带输出的数据
 * */
#include "tlsp_hdr.h"



int
main(int argc, char ** argv)
{
    printf("Hello world");
    execlp("ls", "ls", "-l", NULL);
    errExit("execlp");
}