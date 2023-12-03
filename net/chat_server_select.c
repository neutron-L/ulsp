#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#include "tlsp_hdr.h"

#define NAMESIZE 16
#define BUFSIZE 512
#define SERVER_PORT 8080
#define CLIENT_FD_NUMBER 32

typedef struct
{
    /* data */
    bool welcome;
    int connfd;
    char name[NAMESIZE];
    char buf[BUFSIZ];
    char *data;
    char *write_buf;
} client_t;

typedef struct
{
    /* data */
    int maxfd;
    fd_set read_fds;
    fd_set write_fds;
    fd_set exception_fds;
    fd_set ready_read_fds;
    fd_set ready_write_fds;
    fd_set ready_exception_fds;
    int nready;
    int maxi;
    client_t clients[CLIENT_FD_NUMBER];
} pool;

static void init_client(client_t *pc)
{
    pc->welcome = true;
    pc->connfd = -1;
    pc->data = pc->buf;
    pc->write_buf = NULL;
    memset(pc->buf, 0, sizeof(pc->name));
    memset(pc->buf, 0, sizeof(pc->buf));
}

void init_pool(int listenfd, pool *p)
{
    client_t *pc;
    p->maxi = -1;
    for (pc = p->clients; pc != &p->clients[CLIENT_FD_NUMBER]; ++pc)
        init_client(pc);
    p->maxfd = listenfd;
    FD_ZERO(&p->read_fds);
    FD_ZERO(&p->write_fds);
    FD_ZERO(&p->exception_fds);

    FD_ZERO(&p->ready_read_fds);
    FD_ZERO(&p->ready_write_fds);
    FD_ZERO(&p->ready_exception_fds);

    FD_SET(listenfd, &p->read_fds);
    FD_SET(listenfd, &p->exception_fds);
}

void add_client(int connfd, pool *p)
{
    int i;
    --p->nready;
    for (i = 0; i < CLIENT_FD_NUMBER; ++i)
    {
        if (p->clients[i].connfd < 0)
        {
            p->clients[i].connfd = connfd;
            FD_SET(connfd, &p->read_fds);
            FD_SET(connfd, &p->exception_fds);
            if (connfd > p->maxfd)
                p->maxfd = connfd;
            if (i > p->maxi)
                p->maxi = i;
            break;
        }
    }
    if (i == CLIENT_FD_NUMBER)
        errExit("add_client: too many clients");
}

void check_clients(pool *p)
{
    int i, n;

    for (i = 0; i <= p->maxi && p->nready > 0; ++i)
    {
        client_t *pc = &p->clients[i];
        if (pc->connfd < 0)
            continue;
        bool Read = FD_ISSET(pc->connfd, &p->ready_read_fds);
        bool Write = FD_ISSET(pc->connfd, &p->ready_write_fds);
        bool Exception = FD_ISSET(pc->connfd, &p->ready_exception_fds);

        if (!Read && !Write && !Exception)
            continue;
        --p->nready;
        if (Write)
        {
            // printf("Write\n");
            write(pc->connfd, pc->write_buf, strlen(pc->write_buf));
            pc->write_buf = NULL;
            FD_CLR(pc->connfd, &p->write_fds);
            FD_SET(pc->connfd, &p->read_fds);
            FD_SET(pc->connfd, &p->exception_fds);
        }
        else
        {
            memset(pc->data, 0, (char *)pc->buf + BUFSIZ - pc->data);
            n = recv(pc->connfd, pc->data, BUFSIZ - 1, 0);
            // printf("%d bytes\n", n);
            assert(n >= 0);
            if (n == 0)
            {
                printf("%s exit\n", pc->name);
                FD_CLR(pc->connfd, &p->read_fds);
                FD_CLR(pc->connfd, &p->write_fds);
                FD_CLR(pc->connfd, &p->exception_fds);
                close(pc->connfd);
                init_client(pc);
            }
            else
            {
                // printf("Recv %s\n", pc->data);
                // 第一次收到用户的姓名
                if (pc->welcome)
                {
                    if (pc->buf[strlen(pc->buf) - 1] == '\n')
                    {
                        pc->buf[strlen(pc->buf) - 1] = '\0';
                        --n;
                    }
                    strncpy(pc->name, pc->buf, n);
                    printf("Welcome %s!\n", pc->buf);
                    pc->welcome = false;
                    pc->buf[n] = ':';
                    pc->buf[n + 1] = ' ';
                    pc->data = (char *)pc->buf + n + 2;
                }
                else // 群发用户消息
                {
                    for (int j = 0; j <= p->maxi; ++j)
                    {
                        if (j == i || p->clients[j].connfd == -1 || p->clients[j].welcome)
                            continue;
                        p->clients[j].write_buf = pc->buf;
                        FD_SET(p->clients[j].connfd, &p->write_fds);
                        FD_CLR(p->clients[j].connfd, &p->read_fds);
                        FD_CLR(p->clients[j].connfd, &p->exception_fds);
                    }
                }
            }
        }
    }
}

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
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    assert(listenfd >= 0);
    assert(bind(listenfd, (struct sockaddr *)&server_address, sizeof(server_address)) == 0);
    assert(listen(listenfd, 5) != -1);

    pool p;
    init_pool(listenfd, &p);

    while (1)
    {
        p.ready_read_fds = p.read_fds;
        p.ready_write_fds = p.write_fds;
        p.ready_exception_fds = p.exception_fds;

        p.nready = select(p.maxfd + 1, &p.ready_read_fds, &p.ready_write_fds, &p.ready_exception_fds, NULL);

        if (p.nready < 0)
        {
            fprintf(stderr, "select failure\n");
            break;
        }

        if (FD_ISSET(listenfd, &p.ready_read_fds))
        {
            int connfd = accept(listenfd, (struct sockaddr *)&client_address, &client_addrlen);
            printf("Accept (%s: %d)\n", inet_ntoa(client_address.sin_addr), client_address.sin_port);
            if (connfd < 0)
            {
                fprintf(stderr, "accept failure\n");
                break;
            }
            add_client(connfd, &p);
        }
        check_clients(&p);
    }

    return 0;
}
