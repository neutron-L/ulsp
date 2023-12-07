#ifndef LST_TIMER
#define LST_TIMER


#include <arpa/inet.h>
#include <time.h>

#define BUFFER_SIZE 64

class util_timer;

struct client_data
{
    sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    util_timer * timer;
};

class util_timer
{
private:
    client_data * user_data{};
    util_timer * prev{},* next{};
    time_t expire{};

    void (*cb_func)(client_data *);
public:
    util_timer() : prev(nullptr), next(nullptr);
};

class sort_timer_lst
{
private:
    util_timer * head{}, *tail{};

    void add_timer(util_timer * timer, util_timer * lst_head)
    {}
public: 
    sort_timer_lst() : head(nullptr), tail(nullptr) {}
    ~sort_timer_lst() {}

    void add_timer(util_timer * timer)
    {}

    void adjust_timer(util_timer * timer)
    {}
    void del_timer(util_timer * timer)
    {}
    void tick(){}
};


#endif