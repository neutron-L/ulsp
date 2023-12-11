#ifndef PROCESSPOOL_HH_
#define PROCESSPOOL_HH_

#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class Process
{
private:
    pid_t pid{-1};
    int pipefd[2]{-1,-1};

public:
    Process()=default;
    Process(pid_t p) : pid(p)
    {} 
};


#endif