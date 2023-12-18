/*
 *
 * */
#include <stdio.h>
#include <pthread.h>

#define N 5


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void *task(void *arg)
{
    printf("entry\n");

    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&cond, &mutex);
    pthread_mutex_unlock(&mutex);
    printf("exit\n");
    return NULL;
}

int main()
{
    pthread_t threads[N];
    for (int i = 0; i < N; ++i)
        if (pthread_create(&threads[i], NULL, task, NULL))
            perror("pthread_create");
    sleep(5);
    pthread_cond_broadcast(&cond);
    for (int i = 0; i < N; ++i)
        pthread_join(threads[i], NULL);
    return 0;
}