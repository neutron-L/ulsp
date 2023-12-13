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
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "lst_timer.hh"

static int setnonblocking(int fd);
static void addfd(int epollfd, int fd);
static void removefd(int epollfd, int fd);
static void sig_handler(int sig);
static void addsig(int sig, void (*handler)(int), bool restart = true);

static int sig_pipefd[2];

/// @brief 任务类，需要交由进程池处理的任务必须继承自该类，实现相应的方法
///        任务类需要完成与客户端的通信，因此包含一个socket，以及客户端地址信息
///        任务类还包括一个epoll描述符，进程管理的所有任务类的socket都通过该描述符监听
///        这个epoll描述符由进程管理
class Conn
{
protected:
    int epollfd;
    int sockfd;
    sockaddr_in address;

public:
    void init(int epfd, int sockfd, const sockaddr_in &addr)
    {
        epollfd = epfd;
        this->sockfd = sockfd;
        address = addr;
    }

    virtual bool process() 
    {
        return true;
    }
    virtual void cb_func() const
    {
    }
};

/// @brief 子进程包装类
class Process
{
public:
    pid_t pid{-1};         // 进程id
    int pipefd[2]{-1, -1}; // 与父进程通信的管道

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
    static const int user_per_process = 65536;
    static const int max_event_number = 10000;
    static const int timeslot = 5;
    static ProcessPool<T> *instance;

    int listenfd;
    int process_number;
    int idx{-1};
    int epollfd;

    bool stop{false};
    timer_wheel<T> timer_lst{};
    Process *sub_process;

    // 构造函数为私有，以支持单例模式
    ProcessPool(int lsfd, int proc_num = 8);

    void setup_sig_pipe();
    void timer_handler();

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
        delete[] sub_process;
    }

    void run();
};

template <typename T>
ProcessPool<T> *ProcessPool<T>::instance = nullptr;

template <typename T>
ProcessPool<T>::ProcessPool(int lsfd, int proc_num)
    : listenfd(lsfd), process_number(proc_num)
{
    assert(proc_num > 0 && proc_num <= max_process_number);
    sub_process = new Process[proc_num];
    assert(sub_process);

    int ret;
    for (int i = 0; i < proc_num; ++i)
    {
        assert(!socketpair(PF_UNIX, SOCK_STREAM, 0, sub_process[i].pipefd));
        assert((sub_process[i].pid = fork()) >= 0);

        if (sub_process[i].pid == 0) // 子进程
        {
            close(sub_process[i].pipefd[1]);
            idx = i;
            break;
        }
        else
        {
            close(sub_process[i].pipefd[0]);
        }
    }
}

/// @brief 统一事件源
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
    addsig(SIGCHLD, sig_handler);
    addsig(SIGTERM, sig_handler);
    addsig(SIGINT, sig_handler);
    addsig(SIGPIPE, sig_handler);
}

/// @brief 统一事件源
template <typename T>
void ProcessPool<T>::timer_handler()
{
    timer_lst.tick();
    alarm(timer_lst.getTimeSlot());
}

template <typename T>
void ProcessPool<T>::run()
{
    if (idx == -1)
        run_parent();
    else
        run_child();
}

template <typename T>
void ProcessPool<T>::run_child()
{
    setup_sig_pipe();

    // 子进程设置alarm信号
    addsig(SIGALRM, sig_handler);

    // 子进程通过idx找到与父进程通信的管道
    int pipefd = sub_process[idx].pipefd[0];

    addfd(epollfd, pipefd);

    epoll_event events[max_event_number];
    client_data<T> *users = new client_data<T>[max_process_number];
    int signals[1024];

    assert(users);
    int number = 0;
    int ret;
    bool timeout = false;

    alarm(timer_lst.getTimeSlot());

    printf("Child %d wait...\n", idx);
    while (!stop)
    {
        number = epoll_wait(epollfd, events, max_event_number, -1);

        if (number < 0 && errno != EINTR)
        {
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < number; ++i)
        {
            int sockfd = events[i].data.fd;

            // 来自父进程的管道
            if (sockfd == pipefd)
            {
                int nouse;
                ret = recv(sockfd, &nouse, sizeof(nouse), 0);
                if ((ret < 0 && (errno != EAGAIN)) || ret == 0)
                    continue;
                struct sockaddr_in client;
                socklen_t addrlen = sizeof(client);
                int connfd = accept(listenfd, (struct sockaddr *)&client, &addrlen);
                if (connfd < 0)
                {
                    perror("accept");
                    continue;
                }
                addfd(epollfd, connfd);
                users[connfd].data.init(epollfd, connfd, client);

                // 设置当前连接socket的超时事件
                util_timer<T> *timer = new util_timer<T>(&users[connfd], time(NULL) + 3 * timeslot);
                users[connfd].timer = timer;
                timer_lst.add_timer(timer);
            }
            // 处理信号
            else if (sockfd == sig_pipefd[0])
            {
                ret = recv(sockfd, signals, sizeof(signals), 0);
                if (ret <= 0)
                    continue;
                ret /= sizeof(int);
                for (int j = 0; j < ret; ++j)
                {
                    switch (signals[j])
                    {
                    case SIGALRM:
                        timeout = true;
                        break;
                    case SIGCHLD:
                        pid_t pid;
                        int stat;
                        while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
                            continue;
                        break;
                    case SIGTERM:
                    case SIGINT:
                        stop = true;
                    default:
                        break;
                    }
                }
            }
            else if (events[i].events & EPOLLIN)
            {
                printf("Child %d process...\n", idx);
                bool res = users[events[i].data.fd].data.process();
                util_timer<T> *timer = static_cast<util_timer<T> *> (users[events[i].data.fd].timer);
                // 处理过程中遇到错误或者对方已经关闭连接
                if (!res)
                {
                    users[events[i].data.fd].data.cb_func();
                    if (timer)
                    {
                        timer_lst.del_timer(timer);
                    }
                    printf("user close...\n");
                }
                else
                {
                    // 更新计时器
                    if (timer)
                    {
                        timer->setExpire(time(NULL) + 3 * timeslot);
                        printf("Adjust timer\n");
                        timer_lst.adjust_timer(timer);
                    }
                }
            }
            else
                continue;
        }

        if (timeout)
        {
            timer_handler();
            timeout = false;
        }
    }

    delete[] users;
    users = nullptr;
    close(epollfd);
}

template <typename T>
void ProcessPool<T>::run_parent()
{
    setup_sig_pipe();

    // 子进程通过idx找到与父进程通信的管道
    int pipefd = sub_process[idx].pipefd[0];

    addfd(epollfd, listenfd);
    epoll_event events[max_event_number];
    int signals[1024];

    int new_conn = 1; // 仅用于向子进程传输内容以提示其accept listenfd
    int number = 0;
    int ret;
    int counter = 0;

    printf("Parent wait...\n");
    while (!stop)
    {
        number = epoll_wait(epollfd, events, max_event_number, -1);

        if (number < 0 && errno != EINTR)
        {
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < number; ++i)
        {
            int sockfd = events[i].data.fd;

            // 来自父进程的管道
            if (sockfd == listenfd)
            {
                int j = counter;
                do
                {
                    if (sub_process[j].pid != -1)
                        break;
                    j = (j + 1) % process_number;
                    /* code */
                } while (j != counter);

                if (sub_process[j].pid == -1)
                {
                    printf("None running child process\n");
                    stop = true;
                    break;
                }

                counter = (counter + 1) % process_number;
                send(sub_process[j].pipefd[1], &new_conn, sizeof(new_conn), 0);
                printf("send request to child %d\n", j);
            }
            // 处理信号
            else if (sockfd == sig_pipefd[0])
            {
                ret = recv(sockfd, signals, sizeof(signals), 0);
                if (ret <= 0)
                    continue;
                ret /= sizeof(int);
                for (int j = 0; j < ret; ++j)
                {
                    switch (signals[j])
                    {
                    case SIGCHLD:
                    {
                        pid_t pid;
                        int stat;
                        while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
                        {
                            for (int k = 0; k < process_number; ++k)
                            {
                                if (sub_process[k].pid == pid)
                                {
                                    printf("child %d exit\n", k);
                                    close(sub_process[k].pipefd[1]);
                                    sub_process[k].pid = -1;
                                    break;
                                }
                            }
                        }

                        // 检查是否所有子进程已经退出
                        stop = true;
                        for (int k = 0; k < process_number; ++k)
                        {
                            if (sub_process[k].pid != -1)
                            {
                                stop = false;
                                break;
                            }
                        }
                    }
                    break;
                    case SIGTERM:
                    case SIGINT:
                        // 杀死所有子进程
                        for (int k = 0; k < process_number; ++k)
                        {
                            if (sub_process[k].pid != -1)
                                kill(sub_process[k].pid, signals[j]);
                        }
                    default:
                        break;
                    }
                }
            }
            else
                continue;
        }
    }

    close(epollfd);
    printf("Parent exit\n");
}

/* Static-function definition */

/// @brief 设置文件描述符为非阻塞
/// @param fd 待设置的文件描述符
/// @return 旧的打开文件标志
static int setnonblocking(int fd)
{
    int option = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, option | O_NONBLOCK);
    return option;
}

/// @brief 添加监听描述符
/// @param epollfd epoll的描述符
/// @param fd 待添加的描述符
static void addfd(int epollfd, int fd)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

/// @brief 取消对一个描述符的监听
/// @param epollfd epoll描述符
/// @param fd 待取消监听的描述符
static void removefd(int epollfd, int fd)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
}

/// @brief 信号处理函数
/// @param sig 触发的信号
static void sig_handler(int sig)
{
    int olderr = errno;
    send(sig_pipefd[1], (const void *)&sig, sizeof(sig), 0);
    errno = olderr;
}

/// @brief 设置信号的处理函数
/// @param sig 信号
/// @param handler 信号处理函数
/// @param restart 对于被信号中断的系统调用是否重启
static void addsig(int sig, void (*handler)(int), bool restart)
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