#ifndef MEM_H_
#define MEM_H_

#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>

#include "tlsp_hdr.h"

#define BUFSIZE 5

struct Mem
{
    sem_t lock;
    int head, tail;
    int nums[BUFSIZE];
};

void init(struct Mem *pm)
{
    sem_init(&pm->lock, 1, 1);
    pm->head = pm->tail = 0;
}

void mem_read(struct Mem *pm)
{
    sem_wait(&pm->lock);
    printf("%ld read %d\n", getpid(), pm->nums[pm->tail]);
    pm->tail = (pm->tail + 1) % BUFSIZE;
    sem_post(&pm->lock);
}

void mem_write(struct Mem *pm, int val)
{
    sem_wait(&pm->lock);
    pm->nums[pm->head] = val;
    printf("%d write %d\n", getpid(), val);
    pm->head = (pm->head + 1) % BUFSIZE;
    sem_post(&pm->lock);
}

void destroy(struct Mem * pm)
{
    sem_destroy(&pm->lock);
}
#endif