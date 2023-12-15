#ifndef LST_TIMER_HH_
#define LST_TIMER_HH_

#include <iostream>
#include <algorithm>
#include <arpa/inet.h>
#include <time.h>
#include "timer.hh"

using std::cerr;
using std::endl;

#define BUFFER_SIZE 64

/* 定义基于升序链表的定时器容器 */
/*
 * 一般在SIGALRM信号处理函数中调用定时器容器的tick方法
 * 处理过期的事件
 * */

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
        // 取消时钟信号
        alarm(0);

        while (head)
        {
            /* code */
            auto tmp = head;
            head = head->next;
            delete tmp;
        }
    }

    /// @brief
    ///       操作链表时被新的SIGALRM中断，链表可能处于不一致状态？
    ///       Linux默认阻塞当前正在处理的信号
    /// @param bt
    void add_timer(base_timer<T> *bt) override
    {
        util_timer<T> *timer = dynamic_cast<util_timer<T> *>(bt);

        if (!head)
        {
            head = tail = timer;
            resetTimer();
        }
        else if (head->expire >= timer->expire)
        {
            timer->next = head;
            head->prev = timer;
            head = timer;
            resetTimer();
        }
        else
            add_timer(timer, head);
        printf("add expire: %d\n", timer->expire);
    }

    void adjust_timer(base_timer<T> *bt) override
    {
        util_timer<T> *timer = dynamic_cast<util_timer<T> *>(bt);

        if (!timer)
            return;
        // 链表仍然是有序的
        if ((!timer->prev || timer->expire >= timer->prev->expire) || (!timer->next || timer->expire <= timer->next->expire))
            return;
        del_timer(timer);
        add_timer(timer);
    }
    void del_timer(base_timer<T> *bt) override
    {
        util_timer<T> *timer = dynamic_cast<util_timer<T> *>(bt);

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
            resetTimer();
        }
    }
    void tick() override
    {
        time_t cur = time(NULL);
        while (head && head->expire <= cur)
        {
            printf("Expire...\n");
            head->user_data->data.cb_func();
            auto tmp = head;
            head = head->next;
            delete tmp;
        }

        if (head)
            head->prev = nullptr;
        resetTimer();
    }

    // 重置时钟信号为链表头部节点的过期时间间隔
    void resetTimer() const override
    {
        if (head)
        {
            int tics = head->expire - time(NULL);
            // 可能当前定时器最早过期的事件已经过期，但不能设置alarm 0
            //  否则会取消定时器而永远不会触发alarm信号
            if (tics == 0)
                ++tics;
            printf("reset alarm %d\n", tics);
            alarm(tics);
        }
        else
            alarm(0);
    }
};

#endif