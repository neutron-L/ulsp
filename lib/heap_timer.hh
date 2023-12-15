#ifndef HEAP_TIMER_HH
#define HEAP_TIMER_HH

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
class timer_heap;

/// @brief 用于时间堆的计时器类
/// @tparam T
template <typename T>
class hp_timer : public base_timer<T>
{
    friend class timer_heap<T>;
public:
    hp_timer() = default;
    hp_timer(client_data<T> *data, time_t exp)
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
    int count{};

public:
    timer_wheel() = default;
    ~timer_wheel()
    {
        tw_timer<T> *head, *tmp;

        // 取消时钟信号
        alarm(0);
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

        static struct tm lt;
        localtime_r(&timer->expire, &lt);

        int timeout = timer->expire - time(NULL);
        int ticks = timeout / SI;
        if (ticks == 0)
            ticks = 1;


        timer->rotation = ticks / N;
        timer->slot_idx = (cur_slot + ticks) % N;
        // printf("add expire: %s %d\n", asctime(&lt), timer->slot_idx);

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
        ++count;

        resetTimer();
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
            cur = cur->next;
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
        --count;
        resetTimer();
    }
    void tick() override
    {
        static struct tm lt1, lt2;
        static char buf1[32], buf2[32];

        cur_slot = (cur_slot + 1) % N;
        tw_timer<T> *cur = slots[cur_slot], *tmp;

        // printf("slot: %d\n", cur_slot);
        while (cur)
        {
            if (cur->rotation > 0)
            {
                --cur->rotation;
                cur = cur->next;
            }
            else
            {
                time_t now = time(NULL);
                localtime_r(&now, &lt1);
                localtime_r(&cur->expire, &lt2);
                bzero(buf1, sizeof(buf1));
                bzero(buf2, sizeof(buf2));
                printf("Trigger time: %s Expire time: %s\n",
                       asctime_r(&lt1, buf1), asctime_r(&lt2, buf2));

                cur->user_data->data.cb_func();
                tmp = cur;
                if (cur == slots[cur_slot])
                {
                    cur = cur->next;
                    if (cur)
                        cur->prev = nullptr;
                    slots[cur_slot] = cur;
                }
                else
                {
                    cur = cur->next;
                    if (cur)
                        cur->prev = tmp->prev;
                    assert(tmp->prev);
                    tmp->prev->next = cur;
                }
                // cerr << tmp->slot_idx << endl;
                delete tmp;
                --count;
            }
        }
        resetTimer();
    }

    void resetTimer() const override
    {
        if (count)
            alarm(SI);
        else
            alarm(0);
    }
};

#endif