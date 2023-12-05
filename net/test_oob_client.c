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


    int nodelay = 1;
    if (setsockopt(connfd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(int)) != 0)
        perror("setsockopt");

    const char * oob = "abcd";
    const char * normal ="123";
    send(connfd, normal, strlen(normal), 0);
    send(connfd, oob, strlen(oob), MSG_OOB);
    send(connfd, normal, strlen(normal), 0);
    // send(connfd, oob, strlen(oob), MSG_OOB);

    close(connfd);
    return 0;
}
