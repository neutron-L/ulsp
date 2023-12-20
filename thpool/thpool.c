

#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include "thpool.h"

#define SIG_STOP_EXECUTE SIGUSR1

// 前置声明
typedef struct ThreadPool thpool_t;

static volatile int pause;

/* ============================= Structures =============================== */

/// @brief 线程包装类
typedef struct
{
    int id;
    pthread_t tid;
    thpool_t *thpool;
} thread_t;

/// @brief 任务包装类
typedef struct Job
{
    void *(*fun)(void *); // 执行任务的函数
    void *arg;            // 函数所需要的参数
    job_t *next;
} job_t;

typedef struct
{
    /* data */
    job_t *head, *tail;
    pthread_mutex_t lock;
    unsigned int len;
} jobqueue_t;

struct ThreadPool
{
    /* 线程池的基本参数 */
    int thread_num;
    volatile int thread_alive;
    volatile int thread_working;
    int running; // 执行状态

    /* 线程池管理的结构，包括线程和工作队列 */
    jobqueue_t *jobs;
    thread_t **threads;

    /* 线程池使用的同步数据成员 */
    pthread_mutex_t count_locker; // 线程修改thread_alive/thread_working/thread_stop的互斥锁
    pthread_cond_t all_idle;      // thread_working为0的条件变量，需要与count_locker配合使用
    sem_t has_job;                // 信号量，表示工作队列中是否有任务待处理
};

/* ========================== Static Functions ============================ */
static int thread_init(thpool_t *, int id);
static void *thread_do(void *);
static void thread_pause(int sig);
static void thread_destroy(thread_t *);

static jobqueue_t *jobqueue_init();
static void jobqueue_clear(jobqueue_t *);
static void jobqueue_push(jobqueue_t *, job_t *);
static job_t *jobqueue_pop(jobqueue_t *);
static void jobqueue_destroy(jobqueue_t *);

/* =============================== THREADPOOL ============================== */

thpool_t *thpool_init(int nb_threads)
{
    thpool_t *thpool = (thpool_t *)malloc(sizeof(thpool_t));
    int i;

    if (!thpool)
    {
        perror("malloc");
        goto fail_alloc_thpool;
    }

    /* 初始化统计数据 */
    thpool->thread_num = nb_threads;
    thpool->thread_alive = 0;
    thpool->thread_working = 0;
    thpool->running = 1;

    /* 初始化同步数据 */
    if (pthread_mutex_init(&thpool->count_locker, NULL))
    {
        perror("pthread_mutex_init");
        goto fail_init_locker;
    }

    if (pthread_cond_init(&thpool->all_idle, NULL))
    {
        perror("pthread_cond_init");
        goto fail_init_cond;
    }

    if (sem_init(&thpool->has_job, 0, 0))
    {
        perror("sem_init");
        goto fail_init_sem;
    }

    /* 创建并初始化线程 */
    thpool->threads = (thread_t *)malloc(nb_threads * sizeof(thread_t));
    if (!thpool->threads)
    {
        perror("malloc");
        goto fail_alloc_threads;
    }
    thpool->thread_num = nb_threads;
    for (i = 0; i < nb_threads; ++i)
    {
        if (thread_init(thpool, i))
        {
            perror("thread_init");
            goto fail_init_thread;
        }
    }

    /* 初始化任务队列 */
    thpool->jobs = jobqueue_init();
    if (!thpool->jobs)
        goto fail_init_thread;

    while (thpool->thread_alive < thpool->thread_num)
    {
    }

    return thpool;

fail_init_thread:
    while (i >= 0)
    {
        thread_destroy(thpool->threads[i]);
        --i;
    }

fail_alloc_threads:
    free(thpool->threads);
fail_init_sem:
    pthread_cond_destroy(&thpool->all_idle);

fail_init_cond:
    pthread_cond_destroy(&thpool->count_locker);
fail_init_locker:
    free(thpool);
fail_alloc_thpool:
    return NULL;
}

void thpool_add_job(thpool_t *thpool, void (*func)(void *), void *arg)
{
    job_t *job = (job_t *)malloc(sizeof(job_t));
    jobqueue_push(thpool->jobs, job);
}

void thpool_wait(thpool_t *thpool)
{
    pthread_mutex_lock(&thpool->count_locker);
    while (thpool->jobs->len || thpool->thread_working)
        pthread_cond_wait(&thpool->all_idle, &thpool->count_locker);
    pthread_mutex_unlock(&thpool->count_locker);
}

void thpool_pause(thpool_t *thpool)
{
    pause = 1;
    for (int i = 0; i < thpool->thread_num; ++i)
        pthread_kill(thpool->threads[i], SIG_STOP_EXECUTE);
}

void thpool_resume(thpool_t *thpool)
{
    pause = 0;
}
void thpool_destroy(thpool_t *thpool)
{
}
int thpool_num_threads_working(thpool_t *thpool)
{
    return thpool->thread_working;
}

/* ===================== Helper function implementation ======================== */
static int thread_init(thpool_t *thpool, int id)
{
    thread_t *self = thpool->threads[id];
    self->id = id;
    self->thpool = thpool;

    if (pthread_create(&self->tid, NULL, thread_do, (void *)self))
        return -1;
    return 0;
}

static void *thread_do(void *arg)
{
    thread_t *self = (thread_t *)arg;
    thpool_t *thpool = self->thpool;

    // 递增alive线程数量
    pthread_mutex_lock(&thpool->count_locker);
    ++thpool->thread_alive;
    pthread_mutex_unlock(&thpool->count_locker);

    while (thpool->running)
    {
        // 等待任务
        sem_wait(&thpool->has_job);

        // 检查线程池是否仍在运行
        if (thpool->running)
        {
            // 递增工作线程数量
            pthread_mutex_lock(&thpool->count_locker);
            ++thpool->thread_working;
            pthread_mutex_unlock(&thpool->count_locker);

            // working
            job_t *job = jobqueue_pop(thpool->jobs);
            assert(job);
            assert(job->fun);
            job->fun(job->arg);

            // 递减工作线程数量
            pthread_mutex_lock(&thpool->count_locker);
            --thpool->thread_working;
            pthread_mutex_unlock(&thpool->count_locker);
        }
        else // 唤醒其他阻塞在等待任务处理的线程，可以理解为wait是“取出”一个任务（其实是任务对应的信号量），这里将任务“放回”
            sem_post(&thpool->has_job);
    }

    // 递减alive线程数量
    pthread_mutex_lock(&thpool->count_locker);
    --thpool->thread_alive;
    pthread_mutex_unlock(&thpool->count_locker);
    return NULL;
}

static void thread_pause(int sig)
{
    (void)sig;
    while (pause)
    {
    }
}

static void thread_destroy(thread_t *thread)
{
    free(thread);
}

static jobqueue_t *jobqueue_init()
{
    jobqueue_t *jobqueue = (jobqueue_t *)malloc(sizeof(jobqueue_t));
    if (!jobqueue || pthread_mutex_init(&jobqueue->lock, NULL))
        return NULL;
    jobqueue->head = jobqueue->tail = NULL;
    jobqueue->len = 0;

    return jobqueue;
}

static void jobqueue_clear(jobqueue_t *jobqueue)
{
    pthread_mutex_lock(&jobqueue->lock);
    while (jobqueue->head)
    {
        job_t *job = jobqueue->head;
        free(job);
        jobqueue->head = jobqueue->head->next;
        --jobqueue->len;
    }
    jobqueue->head = jobqueue->tail = NULL;
    pthread_mutex_unlock(&jobqueue->lock);
}
static void jobqueue_push(jobqueue_t *jobqueue, job_t *job)
{
    pthread_mutex_lock(&jobqueue->lock);
    if (jobqueue->tail)

        jobqueue->tail->next = job;
    else
        jobqueue->head = job;
    jobqueue->tail = job;
    ++jobqueue->len;
    pthread_mutex_unlock(&jobqueue->lock);
}

static job_t *jobqueue_pop(jobqueue_t *jobqueue)
{
    job_t *job;
    pthread_mutex_lock(&jobqueue->lock);
    if (!jobqueue->len)
        job = NULL;
    else
    {
        job = jobqueue->head;
        jobqueue->head = job->next;
        if (!jobqueue->head)
            jobqueue->tail = NULL;
        --jobqueue->len;
    }
    pthread_mutex_unlock(&jobqueue->lock);

    return NULL;
}

static void jobqueue_destroy(jobqueue_t *jobqueue)
{
    jobqueue_clear(jobqueue);
    pthread_mutex_destroy(&jobqueue->lock);
    free(jobqueue);
}