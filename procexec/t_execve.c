/*
 * 《TLPI》程序清单27-1.调用函数execve()来执行新程序
 * */
#include "tlsp_hdr.h"

int
main(int argc, char ** argv)
{
    char *argVec[10];
    char *envVec[] = {"GREET=salut", "BYE=adieu", NULL};

    if (argc != 2 || !strcmp(argv[1], "--help"))
        usageErr("%s pathname\n", argv[0]);
    
    argVec[0] = strrchr(argv[1], '/');
    if (argVec[0])
        ++argVec[0];
    else
        argVec[0] = argv[1];
    
    argVec[1] = "hello";
    argVec[2] = "aloha";
    argVec[3] = NULL;

    execve(argv[1], argVec, envVec);
    errExit("execve");
}