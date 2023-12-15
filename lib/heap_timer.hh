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

/* 定义时间堆 */

// 前置声明timer heap
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

/// @brief 时间堆类
/// @tparam T
template <typename T>
class timer_heap : public timer_container<T>
{
private:
    hp_timer<T> **slots{};
    int capacity{5};
    int size{};

    void percolate_down(int i)
    {
        int child;

        while ((child = i * 2) < size)
        {
            if (child + 1 < size && slots[child + 1]->expire < slots[child]->expire)
                ++child;
            if (slots[child]->expire < slots[i]->expire)
            {
                std::swap(slots[i], slots[child]);
                i = child;
            }
            else
                break;
        }
    }

    void percolate_up(int i)
    {
        int parent;

        while (i > 0)
        {
            parent = i / 2;
            if (slots[i]->expire < slots[parent]->expire)
            {
                std::swap(slots[i], slots[parent]);
                i = parent;
            }
            else
                break;
        }
    }

    void resize()
    {
        capacity *= 2;
        hp_timer<T> **new_slots = new hp_timer<T> *[capacity] {};
        for (int i = 0; i < size; ++i)
            new_slots[i] = slots[i];
        delete[] slots;
        slots = new_slots;
    }

    int getIdx(hp_timer<T> *const t) const
    {
        int idx{};
        while (idx < size && slots[idx] != t)
            ++idx;

        return idx;
    }

public:
    timer_heap()
    {
        slots = new hp_timer<T> *[capacity] {};
    }
    ~timer_heap()
    {
        // 取消时钟信号
        alarm(0);
        for (int i = 0; i < size; ++i)
            delete slots[i];
        delete[] slots;
    }

    void add_timer(base_timer<T> *bt) override
    {
        block_alarm();
        if (size == capacity)
            resize();
        hp_timer<T> *timer = dynamic_cast<hp_timer<T> *>(bt);
        if (!timer)
        {
            cerr << "timer is not hp_timer]\n";
            return;
        }
        slots[size++] = timer;
        percolate_up(size - 1);

        resetTimer();
        unblock_alarm();
    }

    void adjust_timer(base_timer<T> *bt) override
    {
        hp_timer<T> *timer = dynamic_cast<hp_timer<T> *>(bt);
        if (!timer)
            return;
        int idx = getIdx(timer);

        if (idx == size)
        {
            cerr << "not found the timer\n";
            return;
        }

        int parent = idx / 2;
        int child = idx * 2;
        if (slots[idx]->expire < slots[parent]->expire)
            percolate_up(idx);
        else
        {
            if (child + 1 < size && slots[child + 1]->expire < slots[child]->expire)
                ++child;
            if (child < size && slots[idx]->expire > slots[child]->expire)
                percolate_down(idx);
        }
        resetTimer();
    }

    void del_timer(base_timer<T> *bt) override
    {
        hp_timer<T> *timer = dynamic_cast<hp_timer<T> *>(bt);
        if (!timer || !size)
            return;
        block_alarm();
        int idx = getIdx(timer);
        cerr << idx << endl;

        if (idx == size)
        {
            cerr << "not found the timer\n";
            return;
        }
        slots[idx] = slots[--size];
        slots[size] = nullptr;
        if (slots[idx])
        {
            if (slots[idx]->expire < timer->expire)
                percolate_up(idx);
            else
                percolate_down(idx);
        }

        resetTimer();
        unblock_alarm();
        cerr << "Delete " << timer->expire << endl ;

    }
    void tick() override
    {
        static struct tm lt1, lt2;
        static char buf1[32], buf2[32];

        if (!size)
            return;
        time_t now = time(NULL);
        hp_timer<T> *cur;
        while (size && (cur = slots[0])->expire <= now)
        {
            localtime_r(&now, &lt1);
            localtime_r(&cur->expire, &lt2);
            bzero(buf1, sizeof(buf1));
            bzero(buf2, sizeof(buf2));
            printf("Trigger time: %s Expire time: %s\n",
                   asctime_r(&lt1, buf1), asctime_r(&lt2, buf2));
            cur->user_data->data.cb_func();
            delete cur;
            if (size == 1)
            {
                --size;
                break;
            }
            slots[0] = slots[--size];
            slots[size] = nullptr;
            percolate_down(0);
        }

        resetTimer();
    }

    void resetTimer() const override
    {
        if (size)
        {
            int tics = slots[0]->expire - time(NULL);
            // 可能当前定时器最早过期的事件已经过期，但不能设置alarm 0
            //  否则会取消定时器而永远不会触发alarm信号
            if (tics == 0)
                ++tics;
            alarm(tics);
        }
        else
            alarm(0);
    }
};

#endif