/*
 * 《TLPI》程序清单27-2.显示参数列表和环境列表。配合t_execve.c使用
 * */
#include "tlsp_hdr.h"

extern char ** environ;

int
main(int argc, char ** argv)
{
    char ** enp;

    for (int j = 0; j < argc; ++j)
        printf("argv[%d] = %s\n", j, argv[j]);

    for (enp = environ; *enp; ++enp)
        printf("environ: %s\n", *enp);
    exit(EXIT_SUCCESS);
}