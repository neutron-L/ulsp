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
    int fd = shm_open("/rw", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (ftruncate(fd, sizeof(struct Mem)) < 0)
        errExit("ftruncate");
    addr = mmap(NULL, sizeof(struct Mem), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    pm = (struct Monitor *)addr;
    init(pm);

    sem_t * full = sem_open(argv[1], O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 0);
    sem_t * empty = sem_open(argv[2], O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 5);

   

    for (int i = 0; i < 10; ++i)
    {
        sem_wait(empty);

        mem_write(pm, i);
        sem_post(full);

        sleep(1);
    }
    destroy(pm);
    munmap(addr, sizeof(struct Mem));
    printf("close\n");
    sem_close(full);
    sem_close(empty);
    printf("unlink\n");
    
 if (sem_unlink(argv[1]) == -1)
        perror("sem_unlink");
    if (sem_unlink(argv[2]) == -1)
        perror("sem_unlink");
    return 0;
}