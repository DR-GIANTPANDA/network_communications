#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>

int main()
{

    int lfd = socket(AF_INET, SOCK_STREAM, 0);


    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5555);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(lfd,(struct sockaddr*)&addr, sizeof(addr));

    listen(lfd, 12);

    struct pollfd fds[1024];

    for (int i = 0; i < 1024; ++i)
    {
        fds[i].fd = -1;
        fds[i].events = POLLIN;
    }


    int maxfd = 0;
    while(1)
    {
        int  ret = poll(fds, maxfd+1, -1); 
        if(ret == -1)
        {
            perror("select");
            return -1;
        }

        // 检测的度缓冲区有变化
        // 有新连接
        if(fds[0].revents & POLLIN)
        {
            // 接收连接请求
            struct sockaddr_in sockcli;
            int len = sizeof(sockcli);
            // 这个accept是不会阻塞的
            int connfd = accept(lfd, (struct sockaddr*)&sockcli, &len);
            // 委托内核检测connfd的读缓冲区
            int i;
            for(i=0; i<1024; ++i)
            {
                if(fds[i].fd == -1)
                {
                    fds[i].fd = connfd;
                    break;
                }
            }
            maxfd = i > maxfd ? i : maxfd;
        }
        // 通信, 有客户端发送数据过来
        for(int i=1; i<=maxfd; ++i)
        {
            // 如果在集合中, 说明读缓冲区有数据
            if(fds[i].revents & POLLIN)
            {
                char buf[128];
                int ret = read(fds[i].fd, buf, sizeof(buf));
                if(ret == -1)
                {
                    perror("read");
                    exit(0);
                }
                else if(ret == 0)
                {
                    printf("对方已经关闭了连接...\n");
                    close(fds[i].fd);
                    fds[i].fd = -1;
                }
                else
                {
                    printf("客户端say: %s\n", buf);
                    write(fds[i].fd, buf, strlen(buf)+1);
                }
            }
        }
    }
    close(lfd);
    return 0;


}

