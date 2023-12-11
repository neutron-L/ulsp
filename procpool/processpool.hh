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

static int setnonblocking(int fd);
static void addfd(int epollfd, int fd);
static void removefd(int epollfd, int fd);
static void sig_handler(int sig);
static void addsig(int sig, void (*handler)(int), bool restart = true);

static int sig_pipefd[2];

/// @brief 子进程包装类
class Process
{
private:
    pid_t pid{-1};         // 进程id
    int pipefd[2]{-1, -1}; // 与父进程通信的管道

public:
    Process() = default;
    Process(pid_t p) : pid(p)
    {
    }
};

/// @brief 进程池类模板，T必须提供一些处理接口以供事件发生时进程调用
template <typename T>
class ProcessPool
{
private:
    static const int max_process_number = 16;
    static const int user_per_process =
        static const int max_event_number = 10000;
    static ProcessPool<T> *instance{nullptr};

    int listenfd;
    int process_number;
    int idx{-1};
    int epollfd;

    bool stop{false};

    Process *sub_process;

    // 构造函数为私有，以支持单例模式
    ProcessPool(int lsfd, int proc_num = 8);

    void setup_sig_pipe();
    void run_child();
    void run_parent();

public:
    static ProcessPool<T> *create(int listenfd, int process_number = 8)
    {
        if (!instance)
        {
            instance = new ProcessPool<T>(listenfd, process_number);
        }

        return instance;
    }

    ~ProcessPool()
    {
        delete [] sub_process;
    }
};

template <typename T>
ProcessPool<T>::ProcessPool(int lsfd, int proc_num)
: listenfd(lsfd), process_number(proc_num)
{

}


template <typename T>
void ProcessPool<T>::setup_sig_pipe()
{
    epollfd = epoll_create(5);

    assert(epollfd != -1);

    // 创建sig pipe用于读取触发的信号
    int ret = socketpair(PF_UNIX, SOCK_STREAM, 0, sig_pipefd);
    assert(ret != -1);

    setnonblocking(sig_pipefd[1]);
    addfd(epollfd, sig_pipefd[0]);

    // 统一信号源
    addsig(SIGCHID, sig_handler);
    addsig(SIGTERM, sig_handler);
    addsig(SIGINT, sig_handler);
    addsig(SIGPIPE, sig_handler);
}

/* Static-function definition */
static int setnonblocking(int fd)
{
    int option = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, option | O_NONBLOCK);
    return option;
}
static void addfd(int epollfd, int fd)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

static void removefd(int epollfd, int fd)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
}

static void sig_handler(int sig)
{
    int olderr = errno;
    send(sig_pipefd[1], (const void *)&sig, sizeof(sig), 0);
    errno = olderr;
}

static void addsig(int sig, void (*handler)(int), bool restart = true)
{
    struct sigaction sa;
    bzero((void *)&sa, sizeof(sa));
    sa.sa_handler = handler;
    if (restart)
        sa.sa_flags |= SA_RESTART;

    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}



#endif