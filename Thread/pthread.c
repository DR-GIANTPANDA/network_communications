#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

// 比如现在定义一个person类 要子线程处理
struct Person
{
    int id;
    char name[36];
    int age;
};

// struct Person p;

void* working(void* arg)
{
     struct Person* p = (struct Person*)arg;
   //  如果使用主线上的传入参数，则需要类型转换
    for (int i = 0; i < 5; i++)
    {
    
        printf("子线程：i = %d\n", i);
        if (i == 3)
        {
           // p.age = 18;
           // strcpy(p.name, "lucy");
           // p.id = 100;
           // 
           // // 只要调用该函数则立马退出；
           // pthread_exit(&p);
           
            p->age = 12;
            strcpy(p->name, "tom");
            p->id = 100;
            pthread_exit(p);
        }
    }

    printf("子线程的ID，%ld\n",pthread_self());

    return NULL;
}


int main(int argc, char* argv[])
{
    struct Person p;

    pthread_t tid;
    pthread_create(&tid,NULL,working,&p);
    printf("主线程的ID，%ld\n",pthread_self());
    //  sleep(3);   第一种解决方案 让主线程睡一会儿
    
//    void* ptr = NULL;
    // 这个线程退出的函数携带数据，当前子线程的主线程得到该数据。如果不需要使用指定为NULL
   //  pthread_exit(NULL);   // 第二种解决， 线程退出，让主线程退出不影响子线程的执行。
    
    
//    pthread_join(tid,&ptr);    // 第三种，线程回收函数，这是一个阻塞函数，如果还有子线程在运行就被阻塞
    //，直到子线程退出函数接触阻塞，函数被调用一次，只能回收一个子线程，多个则需要多次调用
    
//    struct Person* pp = (struct Person*)ptr;
//    printf("子线程返回的数据：name: %s, age: %d, id: %d\n", pp->name, pp->age, pp->id);
    
       
//    printf("子线程返回的数据：name: %s, age: %d, id: %d\n", p.name,p.age, p.id);
 
   // 线程分离， 但是必须加pthread_exit 不然主线程退出地址空间会被释放
   // 线程分离子线程的内核资源会被系统其他进程接管
    pthread_detach(tid);
    pthread_exit(NULL);
    return 0;
}
