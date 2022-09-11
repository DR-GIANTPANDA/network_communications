#ifndef TASKQUEUE_H
#define TASKQUEUE_H 
#include <queue>
#include <pthread.h>

using callback = void (*)(void* arg);

template <typename T>
struct Task
{
    Task()
    {
        function = nullptr;
        arg = nullptr;
    }
    Task(callback f, void* arg)
    {
        this->arg = (T*) arg;
        function = f;
    }
    callback function;
    T* arg;
};

template <typename T>
class TaskQueue
{
public:
    TaskQueue();
    ~TaskQueue();

    // 添加任务
    void addTask(callback func, void* arg);
    void addTask(Task<T>& task);

    // 取出一个任务
    Task<T> takeTask();

    inline int taskNumber()
    {
        return m_taskQ.size();
    }

private:
    std::queue<Task<T>> m_taskQ;  // 任务队列
    pthread_mutex_t m_mutex;  // 互斥锁 
};
#endif 
