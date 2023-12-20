


#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include "thpool.h"

#define SIG_STOP_EXECUTE SIGUSR1

// 前置声明
typedef struct ThreadPool thpool_t;

/* ========================== Static Functions ============================ */
static int thread_init(thpool_t *, int id);
static void *thread_do(void *);
static void thread_pause(int sig);

/* ========================== Classes ============================ */

/// @brief 线程包装类
typedef struct
{
    int id{-1};
    pthread_t tid{-1};
    thpool_t * thpool{};
} thread_t;

/// @brief 任务包装类
typedef struct Job
{
    void *(*fun)(void *){}; // 执行任务的函数
    void *arg{};            // 函数所需要的参数
    job_t * next{};
} job_t;

typedef struct 
{
    /* data */
    job_t * head{}, *tail{};
    pthread_mutex_t lock{};
    unsigned int len{};
} jobqueue_t;


struct ThreadPool
{
    /* 线程池的基本参数 */
    int thread_num{};
    int thread_alive{};
    int thread_working{};
    bool done{}; // 结束执行

    /* 线程池管理的结构，包括线程和工作队列 */
    std::queue<Job> jobs{};
    Thread* threads{};

    /* 线程池使用的同步数据成员 */
    pthread_mutex_t count_locker{}; // 线程修改thread_alive/thread_working/thread_stop的互斥锁
    pthread_cond_t isIdle{};        // thread_working为0的条件变量，需要与count_locker配合使用


    pthread_mutex_t job_locker{};            // 存取工作队列的互斥锁
    sem_t hasJobs{};            // 工作队列中有待处理工作的信号量

public:
    ThreadPool(int nb_thread = 5) : thread_num(nb_thread), threads(nb_thread)
    {
        // 初始化同步数据
        if (pthread_mutex_init(&count_locker, NULL))
        {
            perror("pthread_mutex_init");
            throw std::exception();
        }
        if (pthread_cond_init(&isIdle, NULL))
        {
            perror("pthread_cond_init");
            throw std::exception();
        }

        // 创建线程

        for (int i = 0; i < nb_thread; ++i)
            if (thread_init(this, i))
            {
                throw std::exception();
            }

        // 等待所有线程进入执行状态
        while (thread_alive < thread_num)
        {
        }
    }

    void addTask(void *(*fun)(void *), void *arg)
    {
        job_locker.acquire();
        jobs.push({fun, arg});
        job_locker.release();
        hasJobs.post();
    }

    /// @brief 等待线程执行完所有任务
    void wait()
    {
        pthread_mutex_lock(&count_locker);
        while (jobs.size() || thread_working)
            pthread_cond_wait(&isIdle, &count_locker);
        pthread_mutex_unlock(&count_locker);
    }

    /// @brief 暂停所有线程，通过发送指定信号通知线程自行阻塞
    void pause()
    {
        stop = true;
        for (int i = 0; i < thread_num; ++i)
            pthread_kill(threads[i].tid, SIG_STOP_EXECUTE);
    }

    /// @brief 通过条件变量通知所有阻塞的线程继续执行
    void resume()
    {
        stop = false;
    }
    ~ThreadPool()
    {
    }
};

bool ThreadPool::stop = false;

/* ========================== Static Functions Implementation ============================ */
static int thread_init(ThreadPool * thpool, int id)
{
    Thread * self = &thpool->threads[id];
    
}

static void *thread_do(void *arg)
{
    ThreadPool *thpool = static_cast<ThreadPool *>(arg);

    return NULL;
}

static void thread_pause(int sig)
{
    while (ThreadPool::pause)
    {}
}
