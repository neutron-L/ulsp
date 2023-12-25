
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <signal.h>
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

static int count;

static void sig_int(int signo) {
    printf("\nreceived %d datagrams\n", count);
    exit(0);
}

int main(int argc, char **argv) {
    int listenfd;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    int reuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERV_PORT);

    int rt1 = bind(listenfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (rt1 < 0) {
        perror("bind failed ");
    }

    int rt2 = listen(listenfd, 5);
    if (rt2 < 0) {
        perror("listen failed ");
    }

    signal(SIGINT, sig_int);
    signal(SIGPIPE, SIG_IGN);

    int connfd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    if ((connfd = accept(listenfd, (struct sockaddr *) &client_addr, &client_len)) < 0) {
        perror("bind failed ");
    }

    char message[MAXLINE];
    count = 0;

    for (;;) {
        int n = read(connfd, message, MAXLINE);
        if (n < 0) {
            perror("error read");
        } else if (n == 0) {
            perror("client closed \n");
        }
        message[n] = 0;
        printf("received %d bytes: %s\n", n, message);
        count++;

        char send_line[MAXLINE];
        sprintf(send_line, "Hi, %s", message);

        sleep(5);

        int write_nc = send(connfd, send_line, strlen(send_line), 0);
        printf("send bytes: %zu \n", write_nc);
        if (write_nc < 0) {
            perror("error write");
        }
    }
}