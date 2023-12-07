#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#include "tlsp_hdr.h"

#define BUFSIZE 512
#define SERVER_PORT 8080

char buf[BUFSIZE];

int main(int argc, char **argv)
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

    while (1)
    {
        int connfd = accept(listenfd, (struct sockaddr *)&client_address, &client_addrlen);

        printf("Accept (%s: %d)\n", inet_ntoa(client_address.sin_addr), client_address.sin_port);
        if (connfd < 0)
        {
            fprintf(stderr, "accept failure\n");
            break;
        }
        struct pollfd fd;
        fd.fd = connfd;
        fd.events = POLLIN | POLLRDHUP;
        fd.revents = 0;

        while (1)
        {
            printf("poll...\n");
            ret = poll(&fd, 1, -1);
            if (fd.revents & POLLIN)
            {
                while (1)
                {
                    printf("read...\n");
                    bzero(buf, sizeof(buf));
                    int ret = read(connfd, buf, BUFSIZE - 1);
                    if (ret)
                        printf("%d %s\n", ret, buf);
                    else
                    {
                        printf("brk\n");
                        break;
                    }
                    printf("read done\n");
                }
            }
            if (fd.revents & POLLRDHUP)
            {
                printf("exception \n");
                exit(0);
            }
        }

        close(connfd);
    }

    return 0;
}
