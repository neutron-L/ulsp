#ifndef LST_TIMER
#define LST_TIMER

#include <iostream>
#include <arpa/inet.h>
#include <time.h>

using std::cerr;
using std::endl;

#define BUFFER_SIZE 64

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
    virtual base_timer<T> *add_timer(base_timer<T> *) = 0;
    virtual void del_timer(base_timer<T> *) = 0;
    virtual void adjust_timer(base_timer<T> *) = 0;
    virtual void tick() = 0;
};

/* 定义基于升序链表的定时器容器 */

// 前置声明sort timer lst
template <typename T>
class sort_timer_lst;

/// @brief 用于排序链表的计时器类
/// @tparam T
template <typename T>
class util_timer : public base_timer<T>
{
    friend class sort_timer_lst<T>;

private:
    util_timer *prev{}, *next{};

public:
    util_timer() = default;
    util_timer(client_data<T> *data, time_t exp)
        : base_timer<T>(data, exp)
    {
    }
};

template <typename T>
class sort_timer_lst : public timer_container<T>
{
private:
    util_timer<T> *head{}, *tail{};

    void add_timer(util_timer<T> *timer, util_timer<T> *lst_head)
    {
        auto cur = lst_head;
        while (cur->next && cur->next->expire < timer->expire)
            cur = cur->next;
        if (cur->next)
        {
            timer->next = cur->next;
            cur->next->prev = timer;
            timer->prev = cur;
            cur->next = timer;
        }
        else
        {
            timer->prev = cur;
            timer->next = nullptr;
            tail = cur->next = timer;
        }
    }

public:
    sort_timer_lst() = default;
    ~sort_timer_lst()
    {
        while (head)
        {
            /* code */
            auto tmp = head;
            head = head->next;
            delete tmp;
        }
    }

    void add_timer(base_timer<T> *timer) override
    {
        if (!head)
        {
            head = tail = timer;
        }
        else if (head->expire <= timer->expire)
        {
            timer->next = head;
            head->prev = timer;
            head = timer;
        }
        else
            add_timer(timer, head);
    }

    void adjust_timer(base_timer<T> *timer) override
    {
        if (!timer)
            return;
        // 链表仍然是有序的
        if ((!timer->prev || timer->expire >= timer->prev->expire) || (!timer->next || timer->expire <= timer->next->expire))
            return;
        del_timer(timer);
        add_timer(timer);
    }
    void del_timer(util_timer<T> *timer) override
    {
        if (!timer)
            return;
        if (timer != head)
            timer->prev->next = timer->next;
        if (timer != tail)
            timer->next->prev = timer->prev;
        if (head == timer)
        {
            head = head->next;
            if (!head)
                tail = nullptr;
        }
    }
    void tick() override
    {
        time_t cur = time(NULL);
        while (head && head->expire <= cur)
        {
            head->user_data->data.cb_func();
            auto tmp = head;
            head = head->next;
            delete tmp;
        }

        if (head)
            head->prev = nullptr;
    }
};

/* 定义时间轮 */

// 前置声明timer wheel
template <typename T>
class timer_wheel;

/// @brief 用于排序链表的计时器类
/// @tparam T
template <typename T>
class tw_timer : public base_timer<T>
{
    friend class timer_wheel<T>;

private:
    int rotation{};
    int slot_idx{};

    tw_timer *prev{}, *next{};

public:
    tw_timer() = default;
    tw_timer(client_data<T> *data, time_t exp)
        : base_timer<T>(data, exp)
    {
    }
};

template <typename T>
class timer_wheel : public timer_container<T>
{
private:
    static const int N = 60;
    static const int SI = 1;
    tw_timer *slots[N]{};
    int cur_slot{};

    

public:
    timer_wheel() = default;
    ~timer_wheel()
    {
        tw_timer<T> *head, *tmp;

        for (int i = 0; i < N; ++i)
        {
            head = slots[i];
            while (head)
            {
                *tmp = head;
                head = head->next;
                delete tmp;
            }
        }
    }

    void add_timer(base_timer<T> *bt) override
    {
        tw_timer<T> *timer = dynamic_cast<tw_timer<T> *>(bt);
        int ticks = min(1, timer->expire / SI);

        timer->rotation = ticks / N;
        timer->slot_idx = (cur_slot + ticks) % N;

        if (!timer)
        {
            cerr << "timer should be tw_timer class" << endl;
            return;
        }
        if (!slots[timer->slot_idx])
            slots[timer->slot_idx] = timer;
        else
        {
            timer->next = slots[timer->slot_idx];
            slots[timer->slot_idx]->prev = timer;
            slots[timer->slot_idx] = timer;
        }
    }

    void adjust_timer(base_timer<T> *bt) override
    {
        del_timer(bt);
        add_timer(bt);
    }

    void del_timer(util_timer<T> *bt) override
    {
        tw_timer<T> *timer = dynamic_cast<tw_timer<T> *>(bt);
        if (!timer)
        {
            cerr << "timer should be tw_timer class" << endl;
            return;
        }

        tw_timer<T> *cur = slots[timer->slot_idx];

        while (cur != timer)
            cur = cur->prev;
        if (!cur)
        {
            cerr << "timer does not exist" << endl;
            return;
        }
        if (cur->prev)
            cur->prev->next = cur->next;
        else
            slots[timer->slot_idx] = cur->next;
        if (cur->next)
            cur->next->prev = cur->prev;
    }
    void tick() override
    {
        tw_timer<T> *cur = slots[timer->slot_idx], *tmp;
        while (cur)
        {
            if (--cur->rotation > 0)
                cur = cur->next;
            else
            {
                cur->user_data.data.cb_func();
                tmp = cur;
                if (cur == slots[cur->slot_idx])
                {
                    cur = cur->next;
                    slots[cur->slot_idx] = cur;
                }
                else
                {
                    cur = cur->next;
                    if (cur)
                        cur->prev = tmp->prev;
                    assert (tmp->prev);
                    tmp->prev->next=cur;
                }
                delete tmp;
            }
        }
        cur_slot = (cur_slot + 1) % N;
    }
};

#endif