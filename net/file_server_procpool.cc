/*
 * 利用进程池实现文件服务器
 * */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include "processpool.hh"

#define SERVER_PORT 8088

class FileConn : public Conn
{
private:
    static const int bufsize = 1024;
    static const char *const errinfo;
    char inbuf[bufsize];

public:
    FileConn() {}
    ~FileConn() {}
    
    bool process() const
    {
        int ret;
        struct stat st;

        ret = recv(sockfd, (void *)inbuf, sizeof(inbuf), 0);
        if (ret <= 0)
            return false;
        int fd = open(inbuf, O_RDONLY);
        if (fd < 0)
        {
            perror("open");
            send(sockfd, errinfo, strlen(errinfo), 0);
        }
        else if (fstat(fd, &st) < 0)
        {
            perror("fstat");
        }
        else
            sendfile(sockfd, fd, 0, st.st_size);
        close(fd);
        // 注释该行，使用定时器完成关闭
        // removefd(epollfd, sockfd); 
        return true;
    }

    void cb_func() const
    {
        send(sockfd, "Bye\n", 4, 0);
        removefd(epollfd, sockfd); 
    }
};

const char *const FileConn::errinfo = "No such file\n";


int main()
{
    int ret, listenfd;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);
    address.sin_port = htons(SERVER_PORT);

    listenfd = socket(PF_INET, SOCK_STREAM, 0);
    int reuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    assert(listenfd >= 0);
    if (bind(listenfd, (struct sockaddr *)&address, sizeof(address)) != 0)
        perror("bind");
    assert(listen(listenfd, 5) != -1);

    ProcessPool<FileConn> *processPool = ProcessPool<FileConn>::create(listenfd, 3);

    if (processPool)
    {
        processPool->run();
        delete processPool;
    }
    close(listenfd);
    return 0;
}