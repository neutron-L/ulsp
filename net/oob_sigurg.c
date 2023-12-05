/*
 * 使用SIGURG信号检测带外数据是否到达
 * */
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#include <signal.h>

#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#define BUF_SIZE 1024
#define SERVER_PORT 8080

static int connfd;

void sig_urg(int sig)
{
    static char buf[BUF_SIZE];

    int save_errno = errno;
    bzero(buf, sizeof(buf));
    int ret = recv(connfd, buf, BUF_SIZE - 1, MSG_OOB);
    printf("get %d bytes of oob data %s\n", ret, buf);

    errno = save_errno;
}

void addsig(int sig, void (*sig_handler)(int))
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sig_handler;
    sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

int main()
{
    int ret;

    struct sockaddr_in server_address, client_address;
    socklen_t client_addrlen = sizeof(client_address);

    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr);
    server_address.sin_port = htons(SERVER_PORT);

    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    int reuse = 1;
    int nodelay = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) != 0)
        perror("setsockopt");
    if (setsockopt(listenfd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(int)) != 0)
        perror("setsockopt");
    assert(listenfd >= 0);
    assert(bind(listenfd, (struct sockaddr *)&server_address, sizeof(server_address)) == 0);
    assert(listen(listenfd, 5) != -1);

    connfd = accept(listenfd, (struct sockaddr *)&client_address, &client_addrlen);

    printf("Accept (%s: %d)\n", inet_ntoa(client_address.sin_addr), client_address.sin_port);
    if (connfd < 0)
    {
        fprintf(stderr, "accept failure\n");
    }
    else
    {
        addsig(SIGURG, sig_urg);
        fcntl(connfd, __F_SETOWN, getpid());

        char buf[BUF_SIZE];

        while (1)
        {
            bzero(buf, sizeof(buf));
            ret = recv(connfd, buf, BUF_SIZE - 1, 0);
            if (ret <= 0)
                break;
            printf("got %d bytes of normal data %s\n", ret, buf);
        }
        close(connfd);
    }
    close(listenfd);

    return 0;
}
