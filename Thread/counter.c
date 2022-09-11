#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#define MAX 50

int number;


pthread_mutex_t mutex;


void* funcA_num(void* arg)
{
    for (int i = 0; i < MAX; ++i)
    {
        pthread_mutex_lock(&mutex);
        int cur = number;
        cur++;
        usleep(10);
        number = cur;
        printf("Thread A, id = %lu, number = %d\n", pthread_self(), number);
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

void* funcB_num(void* arg)
{
    for(int i=0; i<MAX; ++i)
    {
        pthread_mutex_lock(&mutex);
        int cur = number;
        cur++;
        number = cur;
        printf("Thread B, id = %lu, number = %d\n", pthread_self(), number);
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main(int argc, char* argv[])
{
    pthread_t p1, p2;
// 在线程创建之前对互斥锁初始化。
    pthread_mutex_init(&mutex, NULL);

    pthread_create(&p1,NULL,funcA_num,NULL);
    pthread_create(&p2,NULL,funcB_num,NULL);

    pthread_join(p1,NULL);
    pthread_join(p2,NULL);
    pthread_mutex_destroy(&mutex);
    return 0;
}
// 多少个共享资源就有多少个锁
