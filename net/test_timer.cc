#include <iostream>
#include <cassert>
#include <signal.h>
#include "processpool.hh"

using namespace std;


const int timeslot = 2;
const int max_process_number = 10;
int n = max_process_number;


class TestConn : public Conn
{
public:
    TestConn() {}
    ~TestConn() {}
    
    bool process() 
    {
        printf("Process");
        return true;
    }

    void cb_func() const
    {
        --n;
    }
};
sort_timer_lst<TestConn> timer;

void handler(int sig)
{
    timer.tick();
}

int main()
{
    client_data<TestConn> *users = new client_data<TestConn>[max_process_number];
    base_timer<TestConn> ** timers = new base_timer<TestConn> *[max_process_number];
    struct sigaction sa;
    bzero((void *)&sa, sizeof(sa));
    sa.sa_handler = handler;
        sa.sa_flags |= SA_RESTART;

    sigfillset(&sa.sa_mask);
    assert(sigaction(SIGALRM, &sa, NULL) != -1);

    for (int i = 0; i < max_process_number; ++i)
    {
        timers[i]= new util_timer<TestConn>(&users[i], time(NULL) + (i + 1) * timeslot);
        timer.add_timer(timers[i]);
    }
    while (n)
    {
        if (n % 2) // 主动移除下标为奇数的定时器
        {
            timer.del_timer(timers[max_process_number - n]);
            delete timers[max_process_number - n];
            --n;
        }
        continue;
    }

    delete [] users;
    delete [] timers;

    return 0;
}