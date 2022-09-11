#include "socket.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>




int createSocket()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    { perror("socket"); return -1; }
    printf("套接字创建成功，fd=%d\n", fd);
    return fd;
}


int setListen(int lfd, unsigned short port)
{
    sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = INADDR_ANY;
    int ret = bind(lfd, (struct sockaddr*)&saddr, sizeof(saddr));
    if (ret == -1)
    { perror("bind"); return -1; }
    printf("套接字绑定成功！\n");

    ret = listen(lfd, 5);
    if (ret == -1)
    { perror("listen"); return -1; }
    printf("监听设置成功！\n");
    return ret;
}

int acceptConn(int lfd, sockaddr_in *addr)
{
    int cfd = -1;
    if (addr == NULL)
        cfd = accept(lfd, NULL, NULL);
    else
    {
        int addrlen = sizeof(sockaddr_in);
        cfd = accept(lfd, (struct sockaddr*)addr, &addrlen);
    }
    if (cfd == -1)
    { perror("accept"); return -1; }

    printf("成功和客户端建立连接...\n");
    return  cfd;
}

// 接受指定字节个数的字符串

int readn(int fd, char* buf, int size)
{
    char* pt = buf;
    int count = size; // 剩余的要接受的字节个数
    while (count > 0)
    {
        int len = recv(fd, pt, count, 0);
        if (len == -1)
            return -1;
        else if (len == 0)
            return size - count;
        pt += len; // 加入150字节先收到50个字节，那么下一个字节开始
        count -= len;
    }
    return size;
}


int recvMsg(int cfd, char**  msg)
{
    int len = 0;
    readn(cfd, (char*)&len,4);
    len = ntohl(len);
    printf("数据块的大小：%d\n",len);

    // 根据读出的长度分配内存+1 \0
    char* buf = (char*)malloc(len+1);
    int ret = readn(cfd, buf, len);
    if (ret != len)
    {
        printf("接收数据失败...\n");
        close(cfd);
        free(buf);
        return -1;
    }
    buf[len] = '\0';
    *msg = buf;

    return ret;
}

// 发送指定长度的字符串
int writen(int fd,const char* msg, int size)
{
   const char* buf = msg;
    int count = size;
    while (count > 0)
    {
        int len = send(fd, buf, count, 0);
        if (len == -1 )
        {
            return -1;
        }
        else if (len == 0)
        {
            continue;
        }
        
        buf += len;
        count -= len;
    }

    return size;
}


// 发送数据

int sendMsg(int cfd,  char* msg, int len)
{
    if (cfd < 0 || msg == NULL || len <= 0)
        return -1;
    // 申请内存空间：数据长度+包头4字节
    char* data = (char*)malloc(len+4);
    int biglen = htonl(len);
    memcpy(data, &biglen, 4);
    memcpy(data+4, msg, len);

    int ret = writen(cfd, data, len+4);
    if (ret == -1 )
    {  close(cfd); }
    return ret;
}


int connectToHost(int fd, const char* ip, unsigned short port)
{
    
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &saddr.sin_addr.s_addr);
    int ret = connect(fd, (struct sockaddr*)&saddr, sizeof(saddr));
    if (ret == -1)
    { perror("connect"); return -1; }
    printf("成功和服务器建立连接...\n");
    return ret;
}


int closeSocket(int fd)
{
    int ret = close(fd);
    if (ret == -1)
        perror("close");
    return ret;
}
