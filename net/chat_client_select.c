#define _GNU_SOURCE

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
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

void signal_handler(int sig)
{
    (void)sig;
    printf("%s is offline\n", name ? name : "???");
    if (connfd)
        close(connfd);
    exit(0);
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "usage: %s <name>\n", argv[0]);
        exit(0);
    }

    // signal handler
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sa.sa_flags = 0;
    sigfillset(&sa.sa_mask);
    assert(sigaction(SIGINT, &sa, NULL) != -1);

    name = argv[1];
    printf("%s is online\n", name);
    int ret;

    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr);
    server_address.sin_port = htons(SERVER_PORT);

    connfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(connect(connfd, (struct sockaddr *)&server_address, sizeof(server_address)) == 0);

    fd_set read_fds, write_fds;
    fd_set ready_read_fds, ready_write_fds;
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&ready_read_fds);
    FD_ZERO(&ready_write_fds);
    FD_SET(STDIN_FILENO, &read_fds);
    FD_SET(connfd, &read_fds);
    FD_SET(connfd, &write_fds);

    bool welcome = true;

    int pipefd[2];
    assert(pipe(pipefd) == 0);

    while (1)
    {
        ready_read_fds = read_fds;
        ready_write_fds = write_fds;
        ret = select(connfd + 1, &ready_read_fds, &ready_write_fds, NULL, NULL);

        if (ret < 0)
        {
            fprintf(stderr, "select failure\n");
            break;
        }
        else
        {
            // 可写事件，每次写完取消该描述符的监听
            // 直到从标准输入读取到了数据
            if (FD_ISSET(connfd, &ready_write_fds))
            {
                if (welcome)
                {
                    write(connfd, name, strlen(name));
                    welcome = false;
                }
                else
                {
                    // printf("write %s\n", send_buf);
                    write(connfd, send_buf, strlen(send_buf));
                }
                FD_CLR(connfd, &write_fds);
            }

            // 可读事件
            if (FD_ISSET(STDIN_FILENO, &ready_read_fds))
            {
                // printf("kedu");
                memset(send_buf, 0, sizeof(send_buf));
                scanf("%[^\n]", send_buf);
                assert(getchar() == '\n');
                FD_SET(connfd, &write_fds);
                // ret = splice(STDIN_FILENO, NULL, pipefd[1], NULL, BUFSIZ, 0);
                // ret = splice(pipefd[0], NULL, connfd, NULL, BUFSIZ, 0);
            }
            else
                FD_CLR(connfd, &write_fds);
            if (FD_ISSET(connfd, &ready_read_fds))
            {
                bzero(recv_buf, BUFSIZE);
                if ((ret = read(connfd, recv_buf, BUFSIZE)) == 0)
                    break;
                printf("%d %s\n", ret, recv_buf);
            }
        }
    }
    close(connfd);
    
    return 0;
}
