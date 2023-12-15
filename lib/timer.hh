#ifndef TIMER_HH_
#define TIMER_HH_

#include <stdio.h>
#include <signal.h>
#include <time.h>

// 前置声明定时器基类
template <typename T>
class base_timer;

/// @brief 用户数据结构体
/// @tparam T 一般是一个继承自Conn的任务类
template <typename T>
struct client_data
{
    T data;               // 具体的用户数据
    base_timer<T> *timer; // 计时器指针
};

/* *
 * 定义定时器的基类和定时器容器基类
 * 所有定时器子类都需要继承自定时器基类
 * 所有定时器容器类都需要继承自定时器容器基类
 * */

/// @brief 定时器基类，所有定时器容器的定时器都要继承自该类
/// @tparam T 一般是一个继承自Conn的任务类
template <typename T>
class base_timer
{
public:
    client_data<T> *user_data{};
    time_t expire{}; // 定时器超时过期期限
public:
    base_timer() = default;
    base_timer(client_data<T> *data, time_t exp)
        : user_data(data), expire(exp)
    {
    }
    virtual void setExpire(time_t expire)
    {
        this->expire = expire;
    }

    virtual ~base_timer()
    {
       
    }
};

/// @brief 定时器容器基类，所有定时器容器都要继承自该类
/// @tparam T 一般是一个继承自Conn的任务类
template <typename T>
class timer_container
{
protected:
    static void block_alarm() 
    {
        sigset_t block_set;  
        sigemptyset(&block_set);  
        sigaddset(&block_set, SIGALRM);  
        if (sigprocmask(SIG_BLOCK, &block_set, NULL) == -1) {  
            perror("sigprocmask");  
        }  
    }

    static void unblock_alarm() 
    {
        sigset_t block_set;  
        sigemptyset(&block_set);  
        sigaddset(&block_set, SIGALRM);  
        if (sigprocmask(SIG_UNBLOCK, &block_set, NULL) == -1) {  
            perror("sigprocmask");  
        }  
    }
public:
    virtual void add_timer(base_timer<T> *) = 0;
    virtual void del_timer(base_timer<T> *) = 0;
    virtual void adjust_timer(base_timer<T> *) = 0;
    virtual void tick() = 0;

    // 定时器类建议使用者调用alarm函数时设置的值
    virtual void resetTimer() const = 0;
};

#endif