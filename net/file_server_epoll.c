/*
 * 使用epoll的LT模式和ET模式
 * 实现file server
 * 根据收到的参数，读取文件并返回给请求方
 * */
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_EVENT_NUMBER 1024
#define SERVER_PORT 8088
#define BUFSIZE 512

char buf[BUFSIZE];

struct sockaddr_in client_addr;
socklen_t addrlen = sizeof(client_addr);
struct stat file_stat;

int setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    assert(fcntl(fd, F_SETFL, new_option) == 0);
    return old_option;
}

void addfd(int epollfd, int fd, bool enable_et)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;
    if (enable_et)
        event.events |= EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

void lt(struct epoll_event *events, int number, int epollfd, int listenfd)
{
    for (int i = 0; i < number; ++i)
    {
        int sockfd = events[i].data.fd;
        if (sockfd == listenfd)
        {
            int connfd = accept(listenfd, (struct sockaddr *)&client_addr, &addrlen);
            if (connfd < 0)
                continue;
            addfd(epollfd, connfd, false);
        }
        else if (events[i].events & EPOLLIN)
        {
            printf("event trigger once\n");
            bzero(buf, sizeof(buf));
            int ret = recv(sockfd, buf, BUFSIZE - 1, 0);
            if (ret <= 0)
            {
                perror("recv");
                close(sockfd);
                continue;
            }
            printf("Request file %s\n", buf);

            int fd = open(buf, O_RDONLY);
            if (fd < 0)
            {
                perror("open");
                send(sockfd, "no suck file\n", 100, 0);
                continue;
            }
            fstat(fd, &file_stat);
            sendfile(sockfd, fd, NULL, file_stat.st_size);
            close(fd);
        }
        else
        {
            printf("sth else happened\n");
        }
    }
}

int main()
{
    int ret, listenfd, epollfd;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);
    address.sin_port = htons(SERVER_PORT);

    listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);
    if (bind(listenfd, (struct sockaddr *)&address, sizeof(address)) != 0)
    {
        perror("bind");
        close(listenfd);
        return 0;
    }
    int reuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    assert(listen(listenfd, 5) != -1);

    struct epoll_event events[MAX_EVENT_NUMBER];
    epollfd = epoll_create(5);
    assert(epollfd != -1);
    addfd(epollfd, listenfd, true);

    while (1)
    {
        ret = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if (ret < 0)
        {
            fprintf(stderr, "epoll failure\n");
            break;
        }
        lt(events, ret, epollfd, listenfd);
    }
    close(listenfd);

    return 0;
}