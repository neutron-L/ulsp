/*
 * 顾名思义，本程序用于发送信号，用法为：./sig_sender PID num-sigs sig-num [sig-num-2]
 * 向进程PID发送num-sigs次sig-num信号，如果有可选参数sig-num-2，再发送一次sig-num-2
 * 可以与sig_receiver.c程序同时运行，以检验如下原理：
 * 等待信号集只是一个掩码，仅表明一个信号是否发生，而未标明其发生次数，如果同一信号在阻塞状态
 * 下产生多次，那么会将该信号记录在信号集中，并稍后仅传递一次
 * */

#include <signal.h>
#include "tlsp_hdr.h"

int
main(int argc, char ** argv)
{
    int numSigs, sig, j;
    pid_t pid;

    if (argc < 4 || !strcmp(argv[1],"--help"))
        usageErr("%s pid num-sigs sig-num [sig-num-2]\n", argv[0]);
    pid = getLong(argv[1], 0, "PID");
    numSigs = getInt(argv[2], GN_GT_0, "num-sigs");
    sig = getInt(argv[3], 0, "sig-num");

    // Send signals to receiver
    printf("%s: sending signal %d to process %ld %d times\n",
        argv[0], sig, (long) pid, numSigs);

    for (j = 0; j < numSigs; ++j)
        if (kill(pid, sig) == -1)
            errExit("kill");

    // If fourth command-line argument was specified, send that signal
    if (argc > 4)
         if (kill(pid, getInt(argv[4], 0, "sig-num-2")) == -1)
            errExit("kill");
    printf("%s: exiting\n", argv[0]);
    
    exit(EXIT_SUCCESS);
}