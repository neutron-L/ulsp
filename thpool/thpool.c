


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

static int jobqueue_init(jobqueue_t *);
static void jobqueue_clear(jobqueue_t *);
static void jobqueue_push(jobqueue_t *);
static job_t * jobqueue_pop(jobqueue_t *);
static void jobqueue_destroy(jobqueue_t *);

