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
#define SERVER_PORT 8080
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

void lt(struct epoll_event *events, int number, int connfd)
{
    for (int i = 0; i < number; ++i)
    {
        if (!(events[i].events & EPOLLIN))
        {
            printf("sth else happened\n");
            continue;
        }

        int sockfd = events[i].data.fd;
        if (sockfd == connfd)
        {
            bzero(buf, sizeof(buf));
            int n = recv(connfd, buf, BUFSIZE - 1, 0);
            printf("%s", n, buf);
        }
        else
        {
            assert(sockfd == STDIN_FILENO);
            bzero(buf, sizeof(buf));
            scanf("%s", buf);
            send(connfd, buf, strlen(buf), 0);
        }
    }
}

void et(struct epoll_event *events, int number, int connfd)
{
    for (int i = 0; i < number; ++i)
    {
        if (!(events[i].events & EPOLLIN))
        {
            printf("sth else happened\n");
            continue;
        }

        int sockfd = events[i].data.fd;
        if (sockfd == connfd)
        {
            while (1)
            {
                bzero(buf, sizeof(buf));
                int n = recv(connfd, buf, BUFSIZE - 1, 0);
                if (n < 0)
                {
                    if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
                    {
                        printf("read later\n");
                        break;
                    }
                    close(connfd);
                    break;
                }
                else if (n == 0)
                    close(connfd);
                else
                    printf("%s", buf);
            }
        }
        else
        {
            assert(sockfd == STDIN_FILENO);
            bzero(buf, sizeof(buf));
            scanf("%s", buf);
            send(connfd, buf, strlen(buf), 0);
        }
    }
}

int main()
{
    int ret, connfd, epollfd;
    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr);
    server_address.sin_port = htons(SERVER_PORT);

    connfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(connect(connfd, (struct sockaddr *)&server_address, sizeof(server_address)) == 0);

    struct epoll_event events[MAX_EVENT_NUMBER];
    epollfd = epoll_create(5);
    assert(epollfd != -1);
    addfd(epollfd, STDIN_FILENO, true);
    addfd(epollfd, connfd, true);

    while (1)
    {
        ret = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if (ret < 0)
        {
            fprintf(stderr, "epoll failure\n");
            break;
        }
        et(events, ret, connfd);
    }
    close(connfd);

    return 0;
}