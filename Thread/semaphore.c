#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>

// 信号量 semaphore 主要阻塞线程，需要和互斥锁一起使用保证线程同步
// 信号量不一定是锁定某一个资源而是流程上的概念，比如A,B两个线程，B线程需要等到A线程完成才能进行下面的工作
// 信号量有一个信号灯，灯亮意味着资源可以用，灯灭不可用


struct Node {
    int number;
    struct Node* next;
};


// 生产者线程信号量
sem_t psem;

// 消费者线程信号量
sem_t csem;


pthread_mutex_t mutex;
struct Node* head = NULL;


void* producer(void* arg)
{
    while(1)
    {

        sem_wait(&psem);
        
        pthread_mutex_lock(&mutex);
        // 创建一个链表的新节点
        struct Node* pnew = (struct Node*)malloc(sizeof(struct Node));
        // 节点初始化
        pnew->number = rand() % 1000;
        // 节点的连接, 添加到链表的头部, 新节点就新的头结点
        pnew->next = head;
        // head指针前移
        head = pnew;
        printf("+++producer, number = %d, tid = %ld\n", pnew->number, pthread_self());
        pthread_mutex_unlock(&mutex);

        sem_post(&csem);
        sleep(rand()%3);
    }
    return NULL;
}


void* consumer(void* arg)
{
    while (1)
    {
        sem_wait(&csem);

        pthread_mutex_lock(&mutex);
        struct Node* pnode = head;
        printf("--consumer: number = %d, tid = %ld\n", pnode->number, pthread_self());
        head = pnode->next;
        free(pnode);
        pthread_mutex_unlock(&mutex);

        sem_post(&psem);

        sleep(rand()%3);
    }

    return NULL;
}





int main(int argc, char* argv[])
{

    sem_init(&psem, 0, 5);
    sem_init(&csem, 0, 0);

    pthread_mutex_init(&mutex,NULL);
    
    pthread_t ptid[5];
    pthread_t ctid[5];

    for(int i=0; i<5; ++i)
    {
        pthread_create(&ptid[i], NULL, producer, NULL);
    }

    for(int i=0; i<5; ++i)
    {
        pthread_create(&ctid[i], NULL, consumer, NULL);
    }

    // 释放资源
    for(int i=0; i<5; ++i)
    {
        pthread_join(ptid[i], NULL);
    }

    for(int i=0; i<5; ++i)
    {
        pthread_join(ctid[i], NULL);
    }

    sem_destroy(&psem);
    sem_destroy(&csem);
    pthread_mutex_destroy(&mutex);
    return 0;
}

