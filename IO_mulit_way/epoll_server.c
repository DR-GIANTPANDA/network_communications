#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
int main(int argc, char* argv[])
{
    int lfd = socket(AF_INET, SOCK_STREAM, 0);

    if(lfd == -1)
    {
        perror("socket error");
        exit(1);
    }

    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(5555);
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int ret = bind(lfd, (struct sockaddr*)&saddr, sizeof(saddr));
    if(ret == -1)
    {
        perror("bind error");
        exit(1);
    }

    ret = listen(lfd, 12);
    if(ret == -1)
    {
        perror("listen error");
        exit(1);
    }
    // 创建epoll实例
    int epfd = epoll_create(100);
    if(epfd == -1)
    {
        perror("epoll_create");
        exit(0);
    }
    // 
    struct epoll_event ev;
    //  ev.events = EPOLLIN;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = lfd;
    // 将lfd描述符添加到epfd树上，以及lfd是什么事件
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);
    if(ret == -1)
    {
        perror("epoll_ctl");
        exit(0);
    }

    struct epoll_event evs[1024];
    int size = sizeof(evs)/sizeof(evs[0]);

    // 检测
    while (1)
    {
        // 指定超时时长-1， 一直检测直到有描述符就绪
        int num = epoll_wait(epfd, evs, size, -1);
        printf("num = %d\n", num);
        for (int i = 0; i < num; ++i)
        {
            int curfd = evs[i].data.fd;

            if (curfd == lfd)
            {
                int cfd = accept(curfd, NULL, NULL);
                // 设置非阻塞属性
                int flag = fcntl(cfd, F_GETFL);
                flag |= O_NONBLOCK;
                fcntl(cfd, F_SETFL, flag);

                //  ev.events = EPOLLIN;
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = cfd;
                ret = epoll_ctl(epfd, EPOLL_CTL_ADD,cfd,  &ev);

                if(ret == -1)
                {
                    perror("epoll_ctl-accept");
                    exit(0);
                }
            }
            else
            {

                char buf[5];
                memset(buf, 0, sizeof(buf));
                while (1)
                {
                    int len = recv(curfd, buf, sizeof(buf), 0);
                    if (len == 0)
                    {
                        printf("客户端已经断开连接\n");

                        epoll_ctl(epfd, EPOLL_CTL_DEL, curfd, NULL);
                        close(curfd);
                        break;
                    }
                    else if (len > 0)
                    {
                        printf("客户端say: %s\n", buf);
                        send(curfd, buf, len, 0);
                    }
                    else
                    {
                        if (errno == EAGAIN)
                        {
                            printf("数据已经接受完毕...\n");
                            break;
                        }
                        perror("recv");
                        exit(0);
                    }
                }
            }
        }
    }
    return 0;
}

