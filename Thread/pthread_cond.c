#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

// 条件变量在生产者和消费者模型中使用，区别于互斥锁
// 条件变量是一种阻塞，当生产者生产的数量满的时候就会被阻塞，反之消费者亦然
// 当生产者阻塞的时候需要消费者唤醒，
// 当消费者阻塞的时候需要生产者唤醒
// 条件变量是阻塞线程的时候需要一个互斥锁，互斥锁的主要作用是进行线程同步 让线程顺序进行 
// 

struct Node
{
    int number;
    struct Node* next;
};

struct Node* head = NULL;

pthread_cond_t cond;
pthread_mutex_t mutex;

void* producer(void* arg)
{
    // 一直生产
    while(1)
    {
        pthread_mutex_lock(&mutex);
// 生产节点肯定是一个接一个所以需要上锁，防止数据混乱
        struct Node* pnew = (struct Node*)malloc(sizeof(struct Node));
        pnew->number = rand()%1000;
        pnew->next = head;
        head = pnew;
        printf("+++producer, number = %d, tid = %ld\n", pnew->number, pthread_self());
        pthread_mutex_unlock(&mutex);

        // 唤醒阻塞在条件变量上的线程。
        pthread_cond_broadcast(&cond);

        sleep(rand()%3);
    }
    return NULL;
}


// 这一个消费者模型函数，5个消费者都可以进入这个函数
void* consumer(void* arg)
{
    while(1)
    {
        pthread_mutex_lock(&mutex);

        while(head==NULL)
        {
            //  当任务队列为空线程就会被阻塞。
            //  那个线程调用这个函数，那个线程就会被阻塞
            //  如果该线程已经对互斥锁上锁，则会打开该互斥锁
            //  让其他的线程包括生产者线程再去抢互斥锁。
            //  当线程被唤醒接触阻塞的时候， 函数会帮这个线程抢该锁，谁抢到了谁上锁继续往下执行
            pthread_cond_wait(&cond, &mutex);
        }

        struct Node* pnode = head;
        printf("--consumer: number: %d, tid = %ld\n", pnode->number, pthread_self());
        head = pnode->next;
        free(pnode);
        pthread_mutex_unlock(&mutex);

        sleep(rand()%3);
    }
    return NULL;
}

        




int main()
{

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond,NULL);

    pthread_t ptid[5];
    pthread_t ctid[5];

    for (int i = 0; i < 5; i++)
    {
        pthread_create(&ptid[i], NULL, producer, NULL);
    }

    for (int i = 0; i < 5; i++)
    {
        pthread_create(&ctid[i], NULL, consumer, NULL);
    }

    // 等待子线程退出;
    for (int i = 0; i < 5; ++i)
    {
        pthread_join(ptid[i],NULL);
    }

    for (int i = 0; i < 5; ++i)
    {
        pthread_join(ctid[i],NULL);
    }

    // 销毁条件变量

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);


//    time_t mytim = time(NULL);

//    printf("time: %ld\n",mytim);

    return 0;
}

