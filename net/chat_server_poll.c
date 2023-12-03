#define _GNU_SOURCE 1
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define USER_LIMIT 5
#define BUFSIZE 64
#define FD_LIMIT 64
#define SERVER_PORT 8080

struct client_data
{
    struct sockaddr_in address;
    char *write_buf;
    char buf[BUFSIZE];
};

int setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

int main()
{
    int listenfd, ret;
    struct sockaddr_in address;

    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);
    address.sin_port = htons(SERVER_PORT);

    listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);
    assert(bind(listenfd, (struct sockaddr *)&address, sizeof(address)) == 0);
    assert(listen(listenfd, 5) == 0);

    struct client_data * users = (struct client_data *)calloc(FD_LIMIT, sizeof(struct client_data));
    struct pollfd fds[USER_LIMIT + 1];
    int user_count = 0;
    bzero(fds, sizeof(fds));
    for (int i = 1; i <= USER_LIMIT; ++i)
        fds[i].fd = -1;
    fds[0].fd = listenfd;
    fds[0].events = POLLIN | POLLERR;

    while (1)
    {
        ret = poll(fds, user_count + 1, -1);
        if (ret < 0)
        {
            fprintf(stderr, "poll failed\n");
            break;
        }

        for (int i = 0; i <= user_count; ++i)
        {
            if ((fds[i].fd == listenfd && fds[i].revents & POLLIN))
            {
                struct sockaddr_in client_address;
                socklen_t client_addrlen = sizeof(client_address);
                int connfd = accept(listenfd, (struct sockaddr *)&client_address, &client_addrlen);
                if (connfd < 0)
                {
                    perror("accept");
                    continue;
                }
                if (user_count >= USER_LIMIT)
                {
                    char * info = "too many users\n";
                    printf("%s", info);
                    send(connfd, info, strlen(info), 0);
                    close(connfd);
                    continue;
                }
                // 添加新的用户
                ++user_count;
                users[connfd].address = client_address;
                setnonblocking(connfd);
                fds[user_count].fd = connfd;
                fds[user_count].events= POLLIN | POLLRDHUP | POLLERR;
                fds[user_count].revents = 0;
                printf("comes a new user, now have %d users\n", user_count);
            }
            else if (fds[i].revents & POLLERR)
            {
                printf("get an error from %d\n", fds[i].fd);
                char errors[BUFSIZE];
                bzero(errors, sizeof(errors));
                socklen_t len = sizeof(errors);
                if (getsockopt(fds[i].fd, SOL_SOCKET, SO_ERROR, &errors, &len) < 0)
                    perror("getsockopt");
                continue;
            }
            else if (fds[i].revents & POLLRDHUP)
            {
                close(fds[i].fd);
                users[fds[i].fd] = users[fds[user_count].fd];
                fds[i] = fds[user_count--];
                --i;
                printf("a client left\n");
            }
            else if (fds[i].revents & POLLIN)
            {
                int connfd = fds[i].fd;
                bzero(users[connfd].buf, BUFSIZE);
                ret = recv(connfd, users[connfd].buf, BUFSIZE - 1, 0);
                printf("get %d bytes of client data %s from %d\n", ret, users[connfd].buf, connfd);
                if (ret < 0)
                {
                    if (errno != EAGAIN)
                    {
                        close(connfd);
                        users[fds[i].fd] = users[fds[user_count].fd];
                        fds[i] = fds[user_count--];
                        --i;
                    }
                }
                else if (ret == 0)
                    continue;
                for (int j = 1; j <= user_count; ++j)
                {
                    if (fds[j].fd == connfd)
                        continue;
                    fds[j].events &= ~POLLIN;
                    fds[j].events |= POLLOUT;
                    users[fds[j].fd].write_buf = users[connfd].buf;
                }

            }
            else if (fds[i].revents & POLLOUT)
            {
                int connfd = fds[i].fd;
                if (!users[connfd].write_buf)
                    continue;
                ret = send(connfd, users[connfd].write_buf, strlen(users[connfd].write_buf), 0);
                users[connfd].write_buf = NULL;
                fds[i].events &= ~POLLOUT;
                fds[i].events |= POLLIN;
            }
        }
    }
    close(listenfd);
    free(users);
    
    return 0;
}