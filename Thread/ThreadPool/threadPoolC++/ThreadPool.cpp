#include "ThreadPool.h"
#include <stdio.h>

template <typename T>
ThreadPool<T>::ThreadPool(int min, int max)
{
    do
    {

        taskQ = new TaskQueue<T>;
        if (taskQ == nullptr)
        { printf("malloc taskQ fail...\n"); break; }
        // 创建工作线程的个数，最大
        threadIDs = new pthread_t[max];
        if (threadIDs == nullptr)
        { printf("malloc threadIDs fail...\n"); break; }
        // 初始化工作线程的ID
        memset(threadIDs, 0, sizeof(pthread_t)*max);

        minNum = min;
        maxNum = max;
        busyNum = 0;
        liveNum = min;
        exitNum = 0;

        if (pthread_mutex_init(&mutexPool, NULL) != 0 ||
            pthread_cond_init(&notEmpty,NULL) != 0 )
        {
            printf("mutex or condition init fail...\n");
            break;
        }

        // 任务队列 


        shutdown = false;

        // 创建线程
        // 静态成员函数访问类内静态成员变量，但是如果传入一个实例化对象则可以访问类内的非静态变量
        pthread_create(&managerID, NULL, manager, this);
        for (int i = 0; i < min; ++i)
        {
            pthread_create(&threadIDs[i], NULL, worker, this);
        }

        return ;
    }while(0);

    // 如果没有创建成功释放资源
    if (threadIDs) delete[] threadIDs;
    if (taskQ) delete taskQ;

}

template <typename T>
ThreadPool<T>::~ThreadPool()
{
    // 关闭线程池
    shutdown = true;
    // 阻塞回收管理者线程
    pthread_join(managerID, nullptr);

    // 唤醒阻塞的消费者线程 让子线程全部退出
    for (int i = 0; i < liveNum; i++)
    {
        pthread_cond_signal(&notEmpty);
    }

    if (taskQ)
        delete taskQ;
    if (threadIDs)
        delete[] threadIDs;
    pthread_mutex_destroy(&mutexPool);
    pthread_cond_destroy(&notEmpty);

}

template <typename T>
void ThreadPool<T>::addTask(Task<T>  task)
{
    if (shutdown)
    {
        return ;
    }
    // 添加任务
    taskQ->addTask(task);
    // 唤醒阻塞在工作变量上的条件变量
    pthread_cond_signal(&notEmpty);
}


template <typename T>
int ThreadPool<T>::getBusyNum()
{
    pthread_mutex_lock(&mutexPool);
    int busyNum = this->busyNum;
    pthread_mutex_unlock(&mutexPool);
    return busyNum;
}

template <typename T>
int ThreadPool<T>::getAliveNum()
{
    pthread_mutex_lock(&mutexPool);
    int aliveNum = this->liveNum;
    pthread_mutex_unlock(&mutexPool);
    return aliveNum;
}

template <typename T>
void* ThreadPool<T>::worker(void* arg)
{
    ThreadPool<T>* pool = static_cast<ThreadPool<T>*>(arg);
    while(true)
    {
        pthread_mutex_lock(&pool->mutexPool);
        // 当前任务队列是否为空

        while (pool->taskQ->taskNumber() == 0 && !pool->shutdown)
        {
            // 阻塞工作线程
            pthread_cond_wait(&pool->notEmpty, &pool->mutexPool);

            // 判断是不是要销毁线程
            if (pool->exitNum > 0)
            {
                pool->exitNum--;
                if (pool->liveNum > pool->minNum)
                {
                    pool->liveNum--;
                    pthread_mutex_unlock(&pool->mutexPool);
                    pool->threadExit();
                }
            }
        }
        // 如果条件阻塞被唤醒
        // 判断线程是否关闭了
        if (pool->shutdown)
        {
            pthread_mutex_unlock(&pool->mutexPool);
            pool->threadExit();
        }

        // 从任务队列抽取任务工作
        Task<T> task = pool->taskQ->takeTask();

        // 有线程开始干活了所以忙的线程加1  因为是共享资源所以加锁
        pool->busyNum++;
        // 解锁 唤醒生产者
        pthread_mutex_unlock(&pool->mutexPool);

        printf("thread %ld start working...\n",pthread_self());


        // 调用函数来执行该任务；传递过来的arg是堆内存。
        task.function(task.arg);
        delete task.arg;
        task.arg=nullptr;

        printf("thread %ld end working...\n", pthread_self());

        // 线程执行完毕后 忙的线程数目就减少1 随时记录线程池中的线程工作情况
        pthread_mutex_lock(&pool->mutexPool);
        pool->busyNum--;
        pthread_mutex_unlock(&pool->mutexPool);
    }
    return nullptr;
}



template <typename T>
void* ThreadPool<T>::manager(void* arg)
{
    ThreadPool<T>* pool = static_cast<ThreadPool<T>*>(arg);
    while (!pool->shutdown)
    {
        // 每隔3秒检测一次
        sleep(3);

        // 取出线程池中任务数量和当前线程的数量 因为是共享资源所以加锁
        // 如果当前任务数量多，线程的数量较少则增加线程的个数
        pthread_mutex_lock(&pool->mutexPool);
        int queueSize = pool->taskQ->taskNumber();
        int liveNum = pool->liveNum;
        int busyNum = pool->busyNum;
        pthread_mutex_unlock(&pool->mutexPool);


        // 添加线程 任务个数>存活的线程个数 && 存活的个数 < 最大线程数
        // 一次只添加2个线程
        if (queueSize > liveNum && liveNum < pool->maxNum)
        {
            pthread_mutex_lock(&pool->mutexPool);
            int counter = 0;
            for (int i = 0; i < pool->maxNum && counter < NUMBER && pool->liveNum < pool->maxNum; ++i)
            {
                // 线程数组里面该线程的ID是0的情况下，因为销毁的线程都必须置为0  所以可以创建出来
                if (pool->threadIDs[i] == 0)
                { 
                    pthread_create(&pool->threadIDs[i], nullptr, worker, pool);
                    counter++;
                    pool->liveNum++;
                }
            }
            pthread_mutex_unlock(&pool->mutexPool);
        }

        // 销毁线程
        // 忙的线程*2 < 存活的线程数  && 存活的线程 > 最小的线程数
        if (busyNum * 2 < liveNum && liveNum > pool->minNum)
        {

            pthread_mutex_lock(&pool->mutexPool);
            pool->exitNum = NUMBER;
            pthread_mutex_unlock(&pool->mutexPool);

            // 让工作的线程自销毁。
            for (int i = 0; i < NUMBER; i++)
            {
                // 没事干的线程都被阻塞在条件变量wait处，把它们唤醒让向下执行自销毁即可
                // 工作的线程都被阻塞在notempty处。唤醒
                pthread_cond_signal(&pool->notEmpty);
            }
        }
    }
    return nullptr;
}


template <typename T>
void ThreadPool<T>::threadExit()
{
    // 先找到该线程ID
    pthread_t tid = pthread_self();
    for (int i = 0; i < maxNum; i++)
    {
        if (threadIDs[i] == tid)
        {
            threadIDs[i] = 0;
            printf("threadExit() called, %ld exiting...\n", tid);
            break;
        }
    }
    // 调用线程退出的函数
    pthread_exit(nullptr);
}


