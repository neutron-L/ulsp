#define _POSIX_SOURCE
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "fifo_seqnum.h"

static void
deleteFifo()
{
    unlink(SERVER_FIFO);
}

int main(int argc, char *argv[])
{
    int readFifo, writeFifo;
    char clientFifoName[CLIENT_FIFO_NAME_LEN];

    if (atexit(deleteFifo))
        fatal("atexit");

    if (mkfifo(SERVER_FIFO, S_IRUSR | S_IWUSR | S_IWGRP) && errno != EEXIST)
        fatal("mkfifo");
    if ((readFifo = open(SERVER_FIFO, O_RDONLY)) == -1 ||
        (writeFifo = open(SERVER_FIFO, O_WRONLY)) == -1)
        errExit("open fifo");

    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = SIG_IGN;

    if (sigaction(SIGPIPE, &sa, NULL) == -1)
        errExit("sigaction");
    
    struct request req;
    struct response resp;
    int clientFifo;
    int seqNum = 0;
    while (1)
    {
        if (read(readFifo, &req, sizeof(struct request)) != sizeof(struct request))
            fatal("read");
        snprintf(clientFifoName, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE, req.pid);
        if ((clientFifo = open(clientFifoName, O_WRONLY)) == -1)
            errExit("open client fifo");
        
        resp.seqNum = seqNum;
        if (write(clientFifo, &resp, sizeof(struct response)) != sizeof(struct response))
        {
            if (errno != EPIPE)
                fatal("write client fifo");
        }
        close(clientFifo);
        seqNum += req.seqLen;
    }

    close(readFifo);
    close(writeFifo);

    return 0;
}