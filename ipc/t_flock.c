/*
 * 程序清单55-1：使用flock
 * */

#include <sys/file.h>
#include <fcntl.h>
#include "curr_time.h"
#include "tlsp_hdr.h"

int main(int argc, char **argv)
{
    // int fd, lock;
    // char *lname;

    // if (argc < 3 || !strcmp(argv[1], "--help") || strchr("sx", argv[2][0]) == NULL)
    // {
    //     usageErr("%s file lock [sleep-time]\n"
    //              "'lock' is 's' or 'x'\n"
    //              "optionally followed by 'n'\m"
    //              "'secs' specifies time to hold lock\n",
    //              argv[0]);
    // }
    // lock = (argv[2][0] == 's') ? LOCK_SH : LOCK_EX;
    // if (argv[2][1] == 'n')
    //     lock |= LOCK_NB;

    // fd = open(argv[1], O_RDONLY);
    // if (fd == -1)
    //     errExit("open");
    // lname = (lock & LOCK_SH) ? "LOCK_SH" : "LOCK_EX";

    // printf("PID %ld: requesting %s at %s\n", (long)getpid(), lname, currTime("%T"));
    // if (flock(fd, lock) == -1)
    // {
    //     if (errno == EWOULDBLOCK)
    //         fatal("PID %ld: already locked - bye!", (long)getpid());
    //     else
    //         errExit("flock (PID=%ld)", (long)getpid());
    // }

    // printf("PID %ld: granted %s at %s\n", (long)getpid(), lname, currTime("%T"));
    // sleep((argc > 3) ? getInt(argv[3], GN_NONNEG, "sleep-time") : 10);
    // printf("PID %ld: releasing %s at %s\n", (long)getpid(), lname, currTime("%T"));
    // if (flock(fd, LOCK_UN) == -1)
        // errExit("flock");
    int fd1 = open("temp", O_RDONLY);
    int fd2 = open("temp", O_RDWR);

    flock(fd1, LOCK_EX);
    printf("lock fd1\n");
    flock(fd2, LOCK_SH);
    printf("lock fd2\n"); // 阻塞

    return 0;
}