#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/select.h>
#include <unistd.h>

#include "tlsp_hdr.h"

#define SERV_PORT 8080
#define MAXLINE 1024


int main(int argc, char **argv)
{
    int socket_fd;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, "localhost", &server_addr.sin_addr);

    socklen_t server_len = sizeof(server_addr);
    int connect_rt = connect(socket_fd, (struct sockaddr *)&server_addr, server_len);
    if (connect_rt < 0)
    {
        perror("connect failed");
    }

    char send_line[MAXLINE], recv_line[MAXLINE + 1];
    int n;

    fd_set readmask;
    fd_set allreads;

    FD_ZERO(&allreads);
    FD_SET(0, &allreads);
    FD_SET(socket_fd, &allreads);
    for (;;)
    {
        readmask = allreads;
        int rc = select(socket_fd + 1, &readmask, NULL, NULL, NULL);
        if (rc <= 0)
            perror("select failed");
        if (FD_ISSET(socket_fd, &readmask))
        {
            n = read(socket_fd, recv_line, MAXLINE);
            if (n < 0)
            {
                perror("read error");
            }
            else if (n == 0)
            {
                perror("server terminated \n");
                exit(0);
            }
            recv_line[n] = 0;
            fputs(recv_line, stdout);
            fputs("\n", stdout);
        }
        if (FD_ISSET(0, &readmask))
        {
            if (fgets(send_line, MAXLINE, stdin) != NULL)
            {
                if (strncmp(send_line, "shutdown", 8) == 0)
                {
                    FD_CLR(0, &allreads);
                    if (shutdown(socket_fd, 1))
                    {
                        perror("shutdown failed");
                    }
                }
                else if (strncmp(send_line, "close", 5) == 0)
                {
                    FD_CLR(0, &allreads);
                    if (close(socket_fd))
                    {
                        perror("close failed");
                    }
                    sleep(6);
                    exit(0);
                }
                else
                {
                    int i = strlen(send_line);
                    if (send_line[i - 1] == '\n')
                    {
                        send_line[i - 1] = 0;
                    }

                    printf("now sending %s\n", send_line);
                    size_t rt = write(socket_fd, send_line, strlen(send_line));
                    if (rt < 0)
                    {
                        perror("write failed ");
                    }
                    printf("send bytes: %zu \n", rt);
                }
            }
        }
    }
}