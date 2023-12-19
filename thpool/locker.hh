#ifndef THREADPOOL_HH_
#define THREADPOOL_HH_

#include <iostream>
#include <exception>
#include <semaphore.h>
#include <pthread.h>
#include <queue>

using std::cerr;
using std::cout;
using std::endl;

class Sem
{
private:
    sem_t sem;

public:
    Sem() : Sem(0)
    {
    }
    Sem(unsigned int val)
    {
        if (sem_init(&sem, 0, val) != 0)
        {
            perror("sem_init");
            throw std::exception();
        }
    }

    ~Sem()
    {
        sem_destroy(&sem);
    }

    bool wait()
    {
        return sem_wait(&sem) == 0;
    }

    bool post()
    {
        return sem_post(&sem) == 0;
    }
};

class Locker
{
private:
    pthread_mutex_t lock;

public:
    Locker()
    {
        if (pthread_mutex_init(&lock, NULL) != 0)
        {
            perror("pthread_mutex_init");
            throw std::exception();
        }
    }

    ~Locker()
    {
        pthread_mutex_destroy(&lock);
    }

    bool acquire()
    {
        return pthread_mutex_lock(&lock) == 0;
    }

    bool release()
    {
        return pthread_mutex_unlock(&lock) == 0;
    }
};

class Condition
{
private:
    pthread_mutex_t mutex;
    pthread_cond_t cond;

public:
    Condition()
    {
        if (pthread_mutex_init(&mutex, NULL) != 0)
        {
            perror("pthread_mutex_init");
            throw std::exception();
        }
        if (pthread_cond_init(&cond, NULL) != 0)
        {
            perror("pthread_cond_init");
            throw std::exception();
        }
    }
    ~Condition()
    {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }

    bool wait()
    {
        int ret;
        pthread_mutex_lock(&mutex);
        ret = pthread_cond_wait(&cond, &mutex);
        pthread_mutex_unlock(&mutex);
        return ret==0;
    }

    bool signal()
    {
        return pthread_cond_signal(&cond) == 0;
    }

    bool signalall()
    {
        return pthread_cond_broadcast(&cond) == 0;
    }
};

#endif