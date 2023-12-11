#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "mem.h"

int main(int argc, char **argv)
{
    char *addr;
    struct Mem *pm;
    /* 初始化 */
    // 创建共享内存
    int fd = shm_open("/rw", O_RDWR, S_IRUSR | S_IWUSR);

    addr = mmap(NULL, sizeof(struct Mem), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    pm = (struct Mem *)addr;
    sem_t *full = sem_open(argv[1], O_RDWR);
    sem_t *empty = sem_open(argv[2], O_RDWR);

    for (int i = 0; i < 5; ++i)
    {
        sem_wait(full);
        mem_read(pm);
        sem_post(empty);

        sleep(1);
    }
    munmap(addr, sizeof(struct Mem));
    sem_close(full);
    sem_close(empty);

    return 0;
}