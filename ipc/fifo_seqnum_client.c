#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "fifo_seqnum.h"

static char clientFifoName[CLIENT_FIFO_NAME_LEN];

static void
deleteFifo()
{
    unlink(clientFifoName);
}

int main(int argc, char *argv[])
{
    int serverFifo, clientFifo;
    pid_t pid;

    pid = getpid();
    if (atexit(deleteFifo))
        fatal("atexit");

    snprintf(clientFifoName, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE, pid);
    if (mkfifo(clientFifoName, S_IRUSR | S_IWUSR | S_IWGRP) && errno != EEXIST)
        fatal("mkfifo");
    if ((serverFifo = open(SERVER_FIFO, O_WRONLY)) == -1)
        errExit("open fifo");

    struct request req;
    struct response resp;

    // send request
    req.pid = pid;
    req.seqLen = atoi(argv[1]);

    if (write(serverFifo, &req, sizeof(struct request)) != sizeof(struct request))
    {
        if (errno != EPIPE)
            fatal("write server fifo");
    }

    if ((clientFifo = open(clientFifoName, O_RDONLY)) == -1)
        errExit("open client fifo");

    if (read(clientFifo, &resp, sizeof(struct response)) != sizeof(struct response))
        fatal("read client fifo");

    printf("%d\n", resp.seqNum);
    
    close(serverFifo);
    close(clientFifo);

    return 0;
}