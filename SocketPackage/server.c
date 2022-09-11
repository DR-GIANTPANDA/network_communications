#include "socket.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
struct SockInfo
{
    int fd;
    sockaddr_in addr;
};

struct SockInfo infos[250];

void* working(void* arg)
{
    struct SockInfo* pinfo = (struct SockInfo*)arg;

    char ip[32];
    printf("客户端的IP，%s, 端口： %d\n", inet_ntop(AF_INET, &pinfo->addr.sin_addr, ip, sizeof(ip)),
           ntohs(pinfo->addr.sin_port));

    while (1)
    {
        char* buf;
        int len = recvMsg(pinfo->fd, &buf);
        printf("接收数据,%d：.....\n",len);
        if (len > 0)
        {
            printf("%s\n\n\n",buf);
            free(buf);
        }
        else
            break;

        sleep(1);
    }
    return NULL;
}




int main()
{
    int fd = createSocket();
    if (fd == -1)
    { return -1; }


    int ret = setListen(fd, 5555);
    if(ret == -1)
        return -1;

    int max = sizeof(infos) / sizeof(infos[0]);
    for (int i = 0; i < max;++i)
    {
        memset(&infos[i], 0, sizeof(infos[0]));
        infos[i].fd = -1;
    }
    while(1)
    {
        struct SockInfo* pinfo;
        printf("max: %d\n", max);
        for (int i = 0; i < max; i++)
        {
            if (infos[i].fd == -1)
            {
                pinfo = &infos[i];
                break;
            }
        }
        pinfo->fd = acceptConn(fd, &pinfo->addr);

        pthread_t tid;
        pthread_create(&tid, NULL, working, pinfo);
        pthread_detach(tid);
    }
    close(fd);
}

