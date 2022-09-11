#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <string.h>

// 线程同步是让一个线程处理完共享内存写会物理内存中，然后下一个线程才有资格去使用
// 不然就会数据混乱
// 常用的线程同步有四种方式，互斥锁，读写锁，条件变量，信号量
// 所谓共享资源就是多个线程共同访问的变量， 这些变量通常为全局区或者堆区的变量
//  找到临界资源上下加锁和解锁
//  保证这部分代码同时只能由一个线程访问，这样并行访问就变成串行访问








