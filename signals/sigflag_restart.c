/*
 * 验证sigaction的 SA_RESTART标志的效果
 * 过程：程序从中断读取输入，可以在终端输入Ctrl-C和Ctrl-Z发送信号，或者使用sig_sender
 * 发送其他信号，为这些信号设置处理函数（自定义或者忽略信号均可），观察read()是否
 * 会被中断。
 * 使用sigaction设置信号的处理函数，并且通过设置或者重置sigaction的SA_RESTART标志
 * 观察其行为上的差别。
 * 原理：设置了SA_RESTART，该信号中断的系统调用会重启，但是键入Ctrl-D本程序会退出，
 * 此时n为0，如果将while循环改为>=0，则会死循环
 * */

#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include "tlsp_hdr.h"

static void
sigHandler(int sig)
{
    // UNSAFE
    printf("Caught %d(%s) and errno is %d(%s)\n",
           sig, strsignal(sig), errno, strerror(errno));
}

int main(int argc, char **argv)
{
    char buf[1024];
    int n;
    struct sigaction sa;

    printf("I am proc %ld\n", (long)getpid());

    // register SIGINT/SIGTSTP/SIGUSR1 signal handler
    sa.sa_handler = sigHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    // sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1)
        errExit("sigaction");
    if (sigaction(SIGTSTP, &sa, NULL) == -1)
        errExit("sigaction");

    while ((n = read(STDIN_FILENO, buf, 1024)) >= 0)
        continue;
    printf("errno is %d(%s)\n", errno, strerror(errno));

    exit(EXIT_SUCCESS);
}