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

    void add_timer(base_timer<T> *bt) override
    {
        util_timer<T> * timer = dynamic_cast<util_timer<T> *>(bt);

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

    void adjust_timer(base_timer<T> *bt) override
    {
        util_timer<T> * timer = dynamic_cast<util_timer<T> *>(bt);

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
        util_timer<T> * timer = dynamic_cast<util_timer<T> *>(bt);

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

    int getTimeSlot() const override
    {
        if (head)
            return head->expire - time(NULL);
        return 5;
    }
};

#endif