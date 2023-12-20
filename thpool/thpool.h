//
// Create on 2023/12/19 by lilin
//

#ifndef THPOOL_H_
#define THPOOL_H_

#ifdef __cplusplus
extern "C"
{
#endif

/* =================================== API ======================================= */
typedef struct ThreadPool thpool_t;

/// @brief 初始化线程池
///        初始化并返回一个线程池，函数等待所有线程创建完毕才返回
/// @example
///     ...
///     thpool_t * thp = thpool_init(5);
///     ...
/// @param nb_threads   线程池中的线程数量
/// @return thpool_t*   成功 线程池指针，
///                     失败 NULL
thpool_t * thpool_init(int nb_threads);



/// @brief  添加任务到工作队列
///         添加一个函数指针以及其执行需要的参数指针，作为一个任务到线程池的工作队列中
/// @example    
///     void do(int num)
///     { ... }
///
///     int main()
///     {
///         ...
///         int num = 10;
///         thpool_add_job(thpool, do, (void *)a);
///         ...
///     }
/// @param thpool_t *   线程池指针 
/// @param func         待添加的任务函数指针
/// @param arg          任务函数需要的参数
void thpool_add_job(thpool_t *, void (*func)(void *), void *arg);




#ifdef __cplusplus
}
#endif

#endif
