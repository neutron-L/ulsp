#ifndef WHELL_TIMER_HH
#define WHELL_TIMER_HH

#include <iostream>
#include <algorithm>
#include <arpa/inet.h>
#include <time.h>
#include "timer.hh"

using std::cerr;
using std::endl;

#define BUFFER_SIZE 64

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
    tw_timer<T> *slots[N]{};
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
                tmp = head;
                head = head->next;
                delete tmp;
            }
        }
    }

    void add_timer(base_timer<T> *bt) override
    {
        tw_timer<T> *timer = dynamic_cast<tw_timer<T> *>(bt);
        int ticks = timer->expire / SI;
        if (ticks == 0)
            ticks = 1;

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

    void del_timer(base_timer<T> *bt) override
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
        tw_timer<T> *cur = slots[cur_slot], *tmp;
        while (cur)
        {
            if (--cur->rotation > 0)
                cur = cur->next;
            else
            {
                cur->user_data->data.cb_func();
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

    int getTimeSlot() const override
    {
        return SI;
    }
};

#endif