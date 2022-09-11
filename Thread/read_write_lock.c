#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>

// 读写锁是一把锁 ，但是有2种功能，可以写锁定，也可以读锁定，
// 写锁定的优先级比读锁定高，读锁是共享的，写锁是独占的。
// 如果程序中所有线程既有读的操作又有写的操作，则读写锁有优势

// 锁定的状态：锁顶/打开
// 锁定的是什么操作，读操作/写操作，使用读写锁锁定了读操作，需要先解锁才能锁定去写的操作
// 这样的原因是保证读的时候数据不混乱。
// 那个线程把这个锁上了

#define MAX 50

int number = 0;

pthread_rwlock_t rwlock;

void* writeNum(void* arg)
{
    for (int i = 0; i<MAX;i++)
    {
        pthread_rwlock_wrlock(&rwlock);
        int cur = number;
        cur++;
        number = cur;
        printf("++写操作完毕，number: %d, tid = %ld\n", number, pthread_self());
        pthread_rwlock_unlock(&rwlock);

        sleep(1);
    }
    return NULL;
}

void* readNum(void* arg)
{
    while(1)
    {
        pthread_rwlock_rdlock(&rwlock);
        printf("--全局变量number = %d, tid = %ld\n", number, pthread_self());
        pthread_rwlock_unlock(&rwlock);
        sleep(1);
    }
    return NULL;
}

int main(int argc, char* argv[])
{
    pthread_rwlock_init(&rwlock, NULL);

    pthread_t wtid[3];
    pthread_t rtid[5]; // 5个读线程 意味着每次可以读取5次共享内存的数据互不干扰

    for (int i = 0; i < 3; i++)
    {
        pthread_create(&wtid[i], NULL,writeNum, NULL);
    }

    for (int i = 0; i < 5; i++)
    {
        pthread_create(&rtid[i], NULL, readNum, NULL);
    }

    for (int i = 0; i < 3; i++)
    {
        pthread_join(wtid[i], NULL);
    }


    for (int i = 0; i < 5; i++)
    {
        pthread_join(rtid[i], NULL);
    }
    
    pthread_rwlock_destroy(&rwlock);

    return 0;
}


