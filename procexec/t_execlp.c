/*
 * 《TLPI》程序清单27-3.使用execlp()在PATH中搜索文件
 * */
#include "tlsp_hdr.h"

int
main(int argc, char ** argv)
{
    if (argc != 2 || !strcmp(argv[1], "--help"))
        usageErr("%s pathname\n", argv[0]);
    
    execlp(argv[1], argv[1], "hello world", NULL);
    errExit("execlp");
}