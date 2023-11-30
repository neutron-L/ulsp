/*
 * 程序清单52-6：通过信号接收消息通知
 * */
#include <signal.h>
#include <mqueue.h>
#include <fcntl.h>
#include "tlsp_hdr.h"

#define NOTIFY_SIG SIGUSR1

static void 
handler(int sig)
{

}

int main(int argc, char * argv[])
{
    if (argc != 2 || strcmp(argv[1], "--help") == -1)
        usageErr("%s mq-name\n", argv[0]);
    
    mqd_t mqd;

    // open mq
    mqd = mq_open(argv[1], O_RDONLY | O_NONBLOCK);
    if (mqd == (mqd_t)-1)
        errExit("mq_open");

    // get the msg size of mq
    struct mq_attr attr;
    void * buffer;
    if (mq_getattr(mqd, &attr))
        errExit("mq_getattr");
    buffer = malloc(attr.mq_msgsize);
    if (buffer == NULL)
        errExit("malloc");
    
    // set signal handler
    sigset_t blockMask, emptyMask;
    struct sigaction sa;
    sigemptyset(&blockMask);   
    sigaddset(&blockMask, NOTIFY_SIG); 
    if (sigprocmask(SIG_BLOCK, &blockMask, NULL) == -1)
        errExit("sigprocmask");
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = handler;
    if (sigaction(NOTIFY_SIG, &sa, NULL) == -1)
        errExit("sigaction");
    
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = NOTIFY_SIG;
    if (mq_notify(mqd, &sev) == -1)
        errExit("mq_notify");
    
    sigemptyset(&emptyMask);

    ssize_t numRead;
    for (;;)
    {
        sigsuspend(&emptyMask);
        if (mq_notify(mqd, &sev))
            errExit("mq_notify");
        while ((numRead = mq_receive(mqd, buffer, attr.mq_msgsize, NULL)) >= 0)
            printf("Read %ld bytes\n", (long)numRead);
        if (errno != EAGAIN)
            errExit("mq_receive");
    }
}

