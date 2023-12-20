


#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include "thpool.h"

#define SIG_STOP_EXECUTE SIGUSR1

// 前置声明
typedef struct ThreadPool thpool_t;


/* ============================= Structures =============================== */

/// @brief 线程包装类
typedef struct
{
    int id;
    pthread_t tid;
    thpool_t * thpool;
} thread_t;

/// @brief 任务包装类
typedef struct Job
{
    void *(*fun)(void *); // 执行任务的函数
    void *arg;            // 函数所需要的参数
    job_t * next;
} job_t;

typedef struct 
{
    /* data */
    job_t * head, *tail;
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
    jobqueue_t* jobs;
    thread_t** threads;

    /* 线程池使用的同步数据成员 */
    pthread_mutex_t count_locker; // 线程修改thread_alive/thread_working/thread_stop的互斥锁
    pthread_cond_t all_idle;        // thread_working为0的条件变量，需要与count_locker配合使用
};



/* ========================== Static Functions ============================ */
static int thread_init(thpool_t *, int id);
static void *thread_do(thread_t *);
static void thread_pause(int sig);
static void thread_destroy(thread_t *);

static jobqueue_t* jobqueue_init();
static void jobqueue_clear(jobqueue_t *);
static void jobqueue_push(jobqueue_t *);
static job_t * jobqueue_pop(jobqueue_t *);
static void jobqueue_destroy(jobqueue_t *);

/* =============================== THREADPOOL ============================== */

thpool_t * thpool_init(int nb_threads)
{
    thpool_t * thpool = (thpool_t *)malloc(sizeof(thpool_t));
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
    {}

    return thpool;


fail_init_thread:
    while (i >= 0)
    {
        thread_destroy(thpool->threads[i]);
        --i;
    }

fail_alloc_threads:
    free(thpool->threads);
    pthread_cond_destroy(&thpool->all_idle);
fail_init_cond:
    pthread_cond_destroy(&thpool->count_locker);
fail_init_locker:
    free(thpool);
fail_alloc_thpool:
    return NULL;
}

void thpool_add_job(thpool_t *, void (*func)(void *), void *arg);

void thpool_wait(thpool_t *);
void thpool_pause(thpool_t *);
void thpool_resume(thpool_t *);
void thpool_destroy(thpool_t *);
int thpool_num_threads_working(thpool_t *);
