#ifndef TIMER_HH_
#define TIMER_HH_

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
protected:
    client_data<T> *user_data{};
    time_t expire{}; // 定时器期限（过期时间）
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
public:
    virtual void add_timer(base_timer<T> *) = 0;
    virtual void del_timer(base_timer<T> *) = 0;
    virtual void adjust_timer(base_timer<T> *) = 0;
    virtual void tick() = 0;

    // 定时器类建议使用者调用alarm函数时设置的值
    virtual int getTimeSlot() const=0;
};

#endif