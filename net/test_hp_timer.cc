#include <iostream>
#include <cassert>
#include <signal.h>
#include "processpool.hh"

using namespace std;

const int timeslot = 1;
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

timer_heap<TestConn> timer;

void handler(int sig)
{
    cout << "Handler\n";
    timer.tick();
}

int main()
{
    static struct tm lt;

    client_data<TestConn> *users = new client_data<TestConn>[max_process_number];
    base_timer<TestConn> **timers = new base_timer<TestConn> *[max_process_number];
    struct sigaction sa;
    bzero((void *)&sa, sizeof(sa));
    sa.sa_handler = handler;
    sa.sa_flags |= SA_RESTART;

    sigfillset(&sa.sa_mask);
    assert(sigaction(SIGALRM, &sa, NULL) != -1);

    for (int i = 0; i < max_process_number; ++i)
    {
        timers[i] = new hp_timer<TestConn>(&users[i], time(NULL) + (max_process_number - i) * timeslot);
        timer.add_timer(timers[i]);

        localtime_r(&timers[i]->expire, &lt);
        // printf("init expire: %s\n", asctime(&lt));
    }

    while (n > 0)
    {
        if (n % 2) // 主动移除下标为奇数的定时器
        {
            // 需要判断待删除的定时器不能是已经过期的，因为过期的定时器已经被删除
            // 而时间堆的操作中，使用到了dynamic_cast，对一个已经delete的指针指向该操作
            // 结果是未定义的
            if (timers[max_process_number - n]->expire > time(NULL)) 
            {
                cout << "Delete " << timers[max_process_number - n]->expire << ' ' << time(NULL) << endl;
                localtime_r(&timers[max_process_number - n]->expire, &lt);
                timer.del_timer(timers[max_process_number - n]);
                delete timers[max_process_number - n];
                --n;
            }
        }
        continue;
    }


    delete[] users;
    delete[] timers;

    return 0;
}