#ifndef LST_TIMER
#define LST_TIMER

#include <arpa/inet.h>
#include <time.h>

#define BUFFER_SIZE 64

class util_timer;
class sort_timer_lst;

struct client_data
{
    sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    util_timer *timer;
};

class util_timer
{
    friend class sort_timer_lst;

private:
    client_data *user_data{};
    util_timer *prev{}, *next{};
    time_t expire{};

    void (*cb_func)(client_data *){nullptr};

public:
    util_timer()=default;
    util_timer(client_data * data, time_t exp, void (*handler)(client_data *))
    : user_data(data), expire(exp), cb_func(handler)
    {}
};

class sort_timer_lst
{
private:
    util_timer *head{}, *tail{};

    void add_timer(util_timer *timer, util_timer *lst_head)
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

    void add_timer(util_timer *timer)
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

    void adjust_timer(util_timer *timer)
    {
        if (!timer)
            return;
        // 链表仍然是有序的
        if ((!timer->prev || timer->expire >= timer->prev->expire) || (!timer->next || timer->expire <= timer->next->expire))
            return;
        del_timer(timer);
        add_timer(timer);
    }
    void del_timer(util_timer *timer)
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
            head->cb_func(head->user_data);
            auto tmp = head;
            head = head->next;
            delete tmp;
        }

        if (head)
            head->prev = nullptr;
    }
};

#endif