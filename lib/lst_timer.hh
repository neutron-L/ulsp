#ifndef LST_TIMER
#define LST_TIMER

#include <arpa/inet.h>
#include <time.h>

#define BUFFER_SIZE 64

template<typename T>
class base_timer
{
protected:
    time_t expire{};
public:
    base_timer()=default;
    base_timer(time_t exp) : expire(exp)
    {}
    void setExpire(time_t expire)
    {
        this->expire = expire;
    }
};


/// @brief 用户数据结构体
/// @tparam T 一般是一个继承自Conn的任务类
template<typename T>
struct client_data
{
    T data;                // 具体的用户数据
    base_timer<T> *timer;  // 计时器指针
};

// 前置声明sort timer lst
template<typename T>
class sort_timer_lst;

/// @brief 用于排序链表的计时器类
/// @tparam T 
template<typename T>
class util_timer : public base_timer<T>
{
    friend class sort_timer_lst<T>;

private:
    client_data<T> *user_data{};
    util_timer *prev{}, *next{};


public:
    util_timer()=default;
    util_timer(client_data<T> * data, time_t exp)
    : base_timer<T>(exp), user_data(data)
    {}
    // void setExpire(time_t expire)
    // {
    //     this->expire = expire;
    // }
};


template<typename T>
class sort_timer_lst
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
    sort_timer_lst() : head(nullptr), tail(nullptr) {}
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

    void add_timer(util_timer<T> *timer)
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

    void adjust_timer(util_timer<T> *timer)
    {
        if (!timer)
            return;
        // 链表仍然是有序的
        if ((!timer->prev || timer->expire >= timer->prev->expire) || (!timer->next || timer->expire <= timer->next->expire))
            return;
        del_timer(timer);
        add_timer(timer);
    }
    void del_timer(util_timer<T> *timer)
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
    void tick()
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

#endif