#define _GNU_SOURCE 1
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFSIZE 64
#define SERVER_PORT 8080

char buf[BUFSIZE];

int main()
{
    int connfd, ret;
    struct sockaddr_in server_address;

    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr);
    server_address.sin_port = htons(SERVER_PORT);

    connfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(connfd >= 0);
    assert(connect(connfd, (struct sockaddr *)&server_address, sizeof(server_address)) == 0);

    struct pollfd fds[2];
    bzero(fds, sizeof(fds));

    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    fds[1].fd = connfd;
    fds[1].events = POLLIN | POLLRDHUP;

    int pipefds[2];
    assert(pipe(pipefds) == 0);

    while (1)
    {
        ret = poll(fds, 2, -1);

        if (ret < 0)
        {
            fprintf(stderr, "poll failure\n");
            break;
        }

        if (fds[1].revents & POLLRDHUP)
        {
            printf("server close the connection\n");
            break;
        }
        if (fds[1].revents & POLLIN)
        {
            memset(buf, 0, sizeof(buf));
            ret = read(fds[1].fd, buf, BUFSIZE - 1);
            printf("%s", buf);
        }

        if (fds[0].revents & POLLIN)
        {
            assert(splice(STDIN_FILENO, NULL, pipefds[1], NULL, BUFSIZE, SPLICE_F_MORE | SPLICE_F_MOVE) >= 0);
            assert(splice(pipefds[0], NULL, connfd, NULL, BUFSIZE, SPLICE_F_MORE | SPLICE_F_MOVE) >= 0);
        }
    }

    close(connfd);
    close(pipefds[0]);
    close(pipefds[1]);

    return 0;
}