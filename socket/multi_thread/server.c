#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
//#include <sys/types.h>
//#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "threadPool.h"
struct SockInfo
{
    struct sockaddr_in addr;
    int fd;
};

typedef struct PoolInfo
{
    ThreadPool* p;
    int fd;
}PoolInfo;




void working(void* arg);

void acceptConn(void* arg);

int main(int argc, char* argv[])
{
    // 1.创建监听的套接字
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    { perror("socket"); return -1; }

    // 2. 绑定本地的IP port
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(5555); // 转换成网络端口大端形式
    saddr.sin_addr.s_addr = INADDR_ANY;  // 0 = 0.0.0.0 对于0来说大端和小端没有区别不需要转换
    // 绑定INADDR_ANY 会自动查询本地IP地址然后进行绑定
    int ret = bind(fd,(struct sockaddr*)&saddr, sizeof(saddr));
    if (ret == -1)
    { perror("bind"); return -1; }

    // 3. 设置监听
    ret = listen(fd, 5);
    if (ret == -1 )
    { perror("listen"); return -1; }

    // 初始化结构体数组
    ThreadPool* pool = threadPoolCreate(3,8,100);
    PoolInfo* info = (PoolInfo*)malloc(sizeof(PoolInfo));
    info->p = pool;
    info->fd = fd;
    threadPoolAdd(pool, acceptConn, info);

    pthread_exit(NULL);
    return 0;
}

void acceptConn(void* arg)
{

    PoolInfo* poolInfo = (PoolInfo*)arg;
    // 4. 阻塞并等待客户端的连接

    // 这里创建一个可以接受客户端的ip和断开的变量
    int addrlen = sizeof(struct sockaddr_in);
    while(1)
    {
        struct SockInfo* pinfo;
        pinfo = (struct SockInfo*)malloc(sizeof(struct SockInfo));
        pinfo->fd = accept(poolInfo->fd, (struct sockaddr*)&pinfo->addr, &addrlen);
        
        if (pinfo->fd == -1)
        { perror("accept"); break; }
        // 创建子线程
        
        threadPoolAdd(poolInfo->p, working,pinfo);
    }
    close(poolInfo->fd);


}


void working(void* arg)
{
    struct SockInfo* pinfo = (struct SockInfo*)arg;
    // 连接成功，打印客户端的IP端口信息。 这一步可以省略
    // 保存在caddr里的IP和端口都是大端 要转换成小端使用。
    char ip[32];
    printf("客户端的IP：%s， 端口：%d\n", inet_ntop(AF_INET, &pinfo->addr.sin_addr,ip,sizeof(ip)),
           ntohs(pinfo->addr.sin_port));


    // 5. 通信

    while(1)
    {
        // 接受数据  recv第一个参数是通信的文件描述符 也就是accept返回的数
        char buffer[1024];
        int len = recv(pinfo->fd, buffer, sizeof(buffer), 0);
        if (len>0)
        {
            printf("client say: %s\n", buffer);
            send(pinfo->fd, buffer, len, 0);
        }
        else if (len == 0)
        {
            printf("客户端断开连接...\n");
            break;
        }
        else
        {
            perror("recv");
            break;
        }
    }

    // 关闭文件描述符
    close(pinfo->fd);
}



