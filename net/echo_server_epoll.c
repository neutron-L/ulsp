/*
 * 同时处理TCP请求和UDP请求的回射服务器
 * 监听多个端口
 * 使用epoll
 * */
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFSIZE 512
#define MAX_EVENT_NUMBER 1024
#define SERVER_PORT 8080

char buf[BUFSIZ];

int setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void addfd(int epollfd, int fd)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

int main()
{
    int listenfd, udpfd, ret, number;
    struct sockaddr_in address;

    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);
    address.sin_port = htons(SERVER_PORT);

    listenfd = socket(PF_INET, SOCK_STREAM, 0);
    int reuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    assert(listenfd >= 0);
    assert(bind(listenfd, (struct sockaddr *)&address, sizeof(address)) == 0);
    assert(listen(listenfd, 5) == 0);

    udpfd = socket(PF_INET, SOCK_DGRAM, 0);
    assert(udpfd >= 0);
    assert(bind(udpfd, (struct sockaddr *)&address, sizeof(address)) == 0);

    struct epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);
    assert(epollfd != -1);

    addfd(epollfd, listenfd);
    addfd(epollfd, udpfd);

    struct sockaddr_in client_address;
    socklen_t client_addrlen = sizeof(client_address);

    while (1)
    {
        number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);

        if (number < 0)
        {
            fprintf(stderr, "epoll failure\n");
            break;
        }
        for (int i = 0; i < number; ++i)
        {
            int sockfd = events[i].data.fd;
            if (sockfd == listenfd)
            {
                int connfd = accept(listenfd, (struct sockaddr *)&client_address, &client_addrlen);
                if (connfd < 0)
                {
                    perror("accept");
                    continue;
                }
                addfd(epollfd, connfd);
            }
            else if (sockfd == udpfd)
            {
                bzero(buf, sizeof(buf));
                ret = recvfrom(udpfd, buf, BUFSIZE - 1, 0, (struct sockaddr *)&client_address, &client_addrlen);
                if (ret > 0)
                    sendto(udpfd, buf, BUFSIZE - 1, 0, (struct sockaddr *)&client_address, &client_addrlen);
            }
            else if (events[i].events & EPOLLIN)
            {
                while (1)
                {
                    // bzero(buf, sizeof(buf));
                    ret = recv(sockfd, buf, BUFSIZE - 1, 0);
                    if (ret < 0)
                    {
                        if (!(errno == EAGAIN || errno == EWOULDBLOCK))
                            close(sockfd);
                        break;
                    }
                    else if (ret == 0)
                    {
                        close(sockfd);
                    }
                    else
                        send(sockfd, buf, ret, 0);
                }
            }
            else
                printf("sth else happened\n");
        }
    }

    close(listenfd);
    close(udpfd);

    return 0;
}
