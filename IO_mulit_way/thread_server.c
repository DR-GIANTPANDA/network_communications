#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ctype.h>
#include <pthread.h>
#include <sys/select.h>

pthread_mutex_t mutex;
typedef struct fdinfo
{
    int fd;
    int* maxfd;
    fd_set* rdset;
}FDInfo;

void* acceptConn(void* arg)
{
    printf("子线程线程ID：%ld\n", pthread_self());
    FDInfo* info = (FDInfo*)arg;

    struct sockaddr_in cliaddr;
    int cliLen = sizeof(cliaddr);
    int cfd = accept(info->fd, (struct sockaddr*)&cliaddr, &cliLen);

    // 把得到的通信文件描述符放到fd集合中，让通信的时候可以检测到
    
    pthread_mutex_lock(&mutex);
    FD_SET(cfd, info->rdset);

    *info->maxfd = cfd >*info->maxfd ? cfd :*info->maxfd;
    pthread_mutex_unlock(&mutex);
    free(info);
    return NULL;
}
void* communication(void* arg)
{
    
    printf("子线程线程ID：%ld\n", pthread_self());
    char buf[10] = {0};
    FDInfo* info = (FDInfo*)arg;
    int  len = recv(info->fd, buf, sizeof(buf),0);
    if (len == 0)
    {
        printf("客户端关闭连接...\n");
        
        pthread_mutex_lock(&mutex);
        FD_CLR(info->fd, info->rdset);
        pthread_mutex_unlock(&mutex);
        close(info->fd);
        free(info);
        return NULL;
    }
    else if (len > 0)
    {
        printf("read buf = %s\n",buf);
        for (int i = 0; i < len; i++)
            buf[i] = toupper(buf[i]);
        printf("after buf = %s\n", buf);
        int  ret = send(info->fd, buf, strlen(buf)+1, 0);
        if (ret == -1)
        { perror("send"); return NULL; }
    }
    else
    {
        perror("recv");
        free(info);
        return NULL;
    }
    free(info);
    return NULL;
}


int main()
{
    pthread_mutex_init(&mutex, NULL);

    int lfd = socket(AF_INET, SOCK_STREAM, 0);


    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5555);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(lfd,(struct sockaddr*)&addr, sizeof(addr));

    listen(lfd, 12);

    int maxfd = lfd;
    fd_set rdset; // rdset用于保存原始数据，这个变量不能作为参数传递给select函数
    // 因为在函数内部会被修改
    fd_set rdtemp;

    FD_ZERO(&rdset);

    FD_SET(lfd, &rdset);

    while(1)
    {
        pthread_mutex_lock(&mutex);
        rdtemp = rdset;
        pthread_mutex_unlock(&mutex);
        // select的最后一个参数超时检测，如果指定为NULL 则代表一直检测直到有就绪的fd
        int num = select(maxfd+1, &rdtemp, NULL, NULL, NULL);
        // 判断是不是监听的fd
        if (FD_ISSET(lfd,&rdtemp))
        {


            pthread_t tid;
            FDInfo* info = (FDInfo*)malloc(sizeof(FDInfo));
            info->fd = lfd;
            info->maxfd = &maxfd;
            info->rdset = &rdset;
            pthread_create(&tid, NULL, acceptConn, info);
            pthread_detach(tid);
        }
        // 判断是不是通信的描述符，因为通信可以有很多个所以这里用循环检测
        for (int i = 0; i < maxfd+1; i++)
        {
            // 如果i不是监听的文件描述符并且在fd集合中就绪 则是一个通信的fd
            if (i != lfd && FD_ISSET(i, &rdtemp))
            {

                pthread_t tid;
                FDInfo* info = (FDInfo*)malloc(sizeof(FDInfo));
                info->fd = i;
                info->rdset = &rdset;
                pthread_create(&tid, NULL, communication, info);
                pthread_detach(tid);
            }
        }
    }
    close(lfd);
    pthread_mutex_destroy(&mutex);
    return 0;
}

