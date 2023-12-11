#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "tlsp_hdr.h"

int main(int argc, char **argv)
{
    struct stat sb;
    int file = open(argv[2], O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    int fd = shm_open(argv[1], O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd < 0)
        perror("shm_open\n");
    printf("%d %d\n", file, fd);
    pid_t p = fork();
    if (p < 0)
        errExit("fork");
    else if (p == 0)
    {
        if (ftruncate(fd, 200) < 0)
            perror("ftruncate");
        if (fstat(fd, &sb) < 0)
            perror("fstat");
        printf("%d %d  %d\n", fd, sb.st_ino, sb.st_size);
        printf("child exit\n");
        exit(0);
        // execlp("./test_close_on_exec", "test_close_on_exec", NULL);
        // perror("execlp");
    }
    else
    {
        wait(NULL);
        char buf[6];
        printf("read\n");
        int n;
        lseek(file, 0, SEEK_SET);

        if ((n = read(file, buf, 6)) < 0)
            perror("read");
        printf("%d %s\n", n, buf);
        if (fstat(fd, &sb) < 0)
            errExit("fstat");
        printf("%d %d  %d\n", fd, sb.st_ino, sb.st_size);
        if (ftruncate(fd, 100) < 0)
            perror("ftruncate");
        if (fstat(fd, &sb) < 0)
            errExit("fstat");
        printf("%d %d  %d\n", fd, sb.st_ino, sb.st_size);
        close(file);
        shm_unlink(argv[1]);
    }

    return 0;
}