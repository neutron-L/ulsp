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



/// @brief 等待当前正在执行的以及排队的任务全部执行完毕
/// @example
///
///     int main()
///     {
///         ...
///         thpool_t * thpool = thpool_init(5);
///         // 添加若干任务
///         thpool_wait(thpool);
///         // 任务全部执行完毕
///         ...
///     }
/// @param  thpool_t *  线程池指针
void thpool_wait(thpool_t *);




/// @brief 立即暂停所有线程
///        所有线程，无论是处于执行状态还是空闲状态
/// @example
///
///     int main()
///     {
///         ...
///         thpool_t * thpool = thpool_init(5);
///         // 添加若干任务
///         ...
///         // 暂停线程池
///         thpool_pause(thpool);
///         ...
///     }
/// @param  thpool_t *  线程池指针
void thpool_pause(thpool_t *);



/// @brief 恢复所有线程（解除暂停状态）
///        用来撤销thpool_pause(thpool_t *)的影响
/// @example
///
///     int main()
///     {
///         ...
///         thpool_t * thpool = thpool_init(5);
///         // 添加若干任务
///         ...
///         // 暂停线程池
///         thpool_pause(thpool);
///         ...
///         // 恢复执行
///         thpool_resume(thpool);
///     }
/// @param  thpool_t *  线程池指针
void thpool_resume(thpool_t *);



/// @brief 销毁线程池
///        会等待正在执行任务的线程完成任务，然后杀死所有线程，
///        最后销毁线程池所有资源，包括线程池指针指向的内存本身
///        排队的任务会被销毁
/// @example
///
///     int main()
///     {
///         ...
///         thpool_t * thpool = thpool_init(5);
///         // 添加若干任务
///         ...
///         // 销毁线程池
///         thpool_destroy(thpool);
///     }
/// @param  thpool_t *  线程池指针
void thpool_destroy(thpool_t *);


/// @brief 返回线程池中执行任务的线程数量
/// @example
///
///     int main()
///     {
///         ...
///         thpool_t * thpool = thpool_init(5);
///         ...
///         printf("Working threads: %d\n", thpool_num_threads_working(thpool1));
///     }
/// @param  thpool_t *  线程池指针
/// @return integer     工作中线程数量
int thpool_num_threads_working(thpool_t *);


#ifdef __cplusplus
}
#endif

#endif
