#define _GNU_SOURCE

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/tcp.h>

#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFSIZE 512
#define SERVER_PORT 8080

char send_buf[BUFSIZE];
char recv_buf[BUFSIZE];

char *name;
int connfd;

int main(int argc, char **argv)
{
    int ret;

    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr);
    server_address.sin_port = htons(SERVER_PORT);

    connfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(connect(connfd, (struct sockaddr *)&server_address, sizeof(server_address)) == 0);

    fd_set read_fds, write_fds, exception_fds;
    fd_set ready_read_fds, ready_write_fds, ready_exception_fds;
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&ready_read_fds);
    FD_ZERO(&ready_write_fds);
    FD_SET(connfd, &read_fds);
    FD_SET(connfd, &write_fds);
    FD_SET(connfd, &exception_fds);

    int nodelay = 1;
    if (setsockopt(connfd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(int)) != 0)
        perror("setsockopt");

    bool brk =false;
    while (1)
    {
        // ready_read_fds = read_fds;
        // ready_write_fds = write_fds;
        // ready_exception_fds = exception_fds;

        // ret = select(connfd + 1, &ready_read_fds, NULL, &ready_exception_fds, NULL);

        // if (ret < 0)
        // {
        //     fprintf(stderr, "select failure\n");
        //     break;
        // }
        // else
        // {
        //     printf("%d\n", sockatmark(connfd));
        //     // 可读事件
        //     if (FD_ISSET(connfd, &ready_read_fds))
        //     {
        //         bzero(recv_buf, sizeof(recv_buf));
        //         printf("read event\n");
        //         if (recv(connfd, recv_buf, 100, 0) == 0)
        //         {
        //             printf("nodata\n");
        //             brk=true;
        //         }
        //         else
        //             printf("%s\n", recv_buf);
        //         // FD_CLR(connfd, &read_fds);
        //     }
        //     printf("%d\n", sockatmark(connfd));

        //     if (FD_ISSET(connfd, &ready_exception_fds))
        //     {
        //         bzero(recv_buf, sizeof(recv_buf));
        //         printf("exception event\n");
        //         if (recv(connfd, recv_buf, 100, MSG_OOB) == 0)
        //             printf("nodata\n");
        //         else
        //             printf("%s\n", recv_buf);
        //     }
        //     printf("%d\n", sockatmark(connfd));

        // }
        // if (brk)
        //     break;
        // printf("========\n");
        scanf("%s", send_buf);
        if (send(connfd, send_buf, strlen(send_buf), 0) < 0)
            perror("send");
    }
    close(connfd);
    return 0;
}
