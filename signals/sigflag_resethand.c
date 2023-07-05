/*
 * 这是exercise 20.16的20-3，验证sigaction的
 * SA_RESETHAND标志的效果
 * 过程：使用sigaction设置SIGINT的处理函数，并且sigaction的flag为SA_RESETHAND
 * 使用sig_sender发送两次SIGINT，第一次调用处理函数，第二次则执行默认行为，即终止
 * 程序
 * 原理：设置了SA_RESETHAND，会在调用处理器函数之前将信号处置重置为默认值
 * */

#include <signal.h>
#include "signal_functions.h"
#include "tlsp_hdr.h"

static void
handler(int sig)
{
    printf("Caught %d(%s)\n", sig, strsignal(sig));
}


int
main(int argc, char ** argv)
{
    (void)argc;

    printf("%s: PID is %ld\n", argv[0], (long)getpid());

    struct sigaction act;

    act.sa_handler = handler;
    act.sa_flags = SA_RESETHAND;
    sigemptyset(&act.sa_mask);
    (void)sigaction(SIGINT, &act, NULL);

    for (int i = 0; ; ++i)
    {
        sleep(1);
        printf("now is %d\n", i);
    }

    exit(EXIT_SUCCESS);
}