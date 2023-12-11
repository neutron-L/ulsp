/*
 * 混合使用库函数和系统调用进行文件I/O
 * */
#include <assert.h>
#include "tlsp_hdr.h"

int main()
{
    setvbuf(stdout, NULL, _IOLBF, 0); // 设置输出为无缓冲
    printf("If I had more time, \n"); // 加上回车符
    // fflush(stdout);  // 刷新缓冲区
    write(STDOUT_FILENO, "I would have written you a shorter letter.\n", 44);
    exit(EXIT_SUCCESS);
}