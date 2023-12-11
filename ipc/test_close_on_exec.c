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
    printf("I am child\n");
    int filefd = 3, fd = 4;

    write(filefd, "hello\n", 6);

    char buf[6];
    lseek(filefd, 0, SEEK_SET);
    int r = read(filefd, buf, 6);
    printf("%d %s", r, buf);

    
    struct stat sb;
    if (fstat(fd, &sb) < 0)
        perror("fstat");
    else
        printf("%d %d  %d\n", fd, sb.st_ino, sb.st_size);
    close(filefd);
    close(fd);
    printf("end\n");
    return 0;
}