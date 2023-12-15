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
    hp_timer<T> *slots[N]{};
    int capacity{};
    int size{};

    void percolate_down(int i)
    {
    }

    void percolate_up(int i)
    {
    }

    void resize()
    {
    }

    int getIdx(hp_timer<T> *const t) const
    {
        int idx{};
        while (idx < size && slots[idx] != timer)
            ++idx;
        return idx;
    }

public:
    timer_heap() = default;
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
        if (size == capacity)
            resize();
        hp_timer<T> *timer = dynamic_cast<hp_timer<T> *>(bt);
        slots[size++] = timer;
        percolate_up(size - 1);

        resetTimer();
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
            if (slots[idx]->expire > slots[child]->expire)
                percolate_down(idx);
        }
        resetTimer();
    }

    void del_timer(base_timer<T> *bt) override
    {
        hp_timer<T> *timer = dynamic_cast<hp_timer<T> *>(bt);

        if (!timer)
            return;
        int idx = getIdx(timer);
        int i = idx;
        while (i < size)
            slots[i] = slots[++i];
        --size;

        for (i = size / 2; i >= 0; --i)
            percolate_down(i);

        resetTimer();
    }
    void tick() override
    {
        static struct tm lt1, lt2;
        static char buf1[32], buf2[32];

        cur_slot = (cur_slot + 1) % N;
        tw_timer<T> *cur = slots[0];

        time_t now = time(NULL);
        localtime_r(&now, &lt1);
        localtime_r(&cur->expire, &lt2);
        bzero(buf1, sizeof(buf1));
        bzero(buf2, sizeof(buf2));
        printf("Trigger time: %s Expire time: %s\n",
               asctime_r(&lt1, buf1), asctime_r(&lt2, buf2));
        delete cur;
        slots[0] = slots[--size];
        percolate_down(0);

        resetTimer();
    }

    void resetTimer() const override
    {
        if (size)
            alarm(slots[0]->exist - time(NULL));
        else
            alarm(0);
    }
};

#endif