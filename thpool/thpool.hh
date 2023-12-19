#ifndef THPOOL_HH_
#define THPOOL_HH_

#include <queue>
#include <vector>
#include <pthread.h>

#include "locker.hh"

/* ========================== Static Functions ============================ */
static void * thread_do(void *);

/* ========================== Classes ============================ */

/// @brief 线程包装类
class Thread
{
private:
    int id{};
    pthread_t tid{};
};


/// @brief 任务包装类
class Task
{
    void *(*fun)(void *) {}; // 执行任务的函数
    void *arg{};            // 函数所需要的参数
};

class ThreadPool
{
private:
    /* 线程池的基本参数 */
    int thread_num{};
    int thread_alive{};
    int thread_working{};
    bool done{};  // 结束执行
    /* 线程池管理的结构，包括线程和工作队列 */
    std::queue<Task> tasks{};
    std::vector<Thread> threads{};

    /* 线程池使用的同步数据成员 */
    Locker count_locker{}; // 线程修改thread_alive和thread_working的互斥锁
    Semaphore hasJobs{};   // 工作队列中有待处理工作的信号量
    Condition isIdle{};    // 工作线程为0的条件变量
    Condition beContinue{};  // 继续执行的条件变量，用于暂停和开启线程池

public:
    ThreadPool(int nb_thread = 5) :
    thread_num(nb_thread)
    {}

    void addTask(void *( *fun)(void *), void * arg)
    {}
    
    void wait()
    {}

    void pause()
    {}
    void resume()
    {}
    ~ThreadPool()
    {}
};  

#endif