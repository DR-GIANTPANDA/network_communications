#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ctype.h>
int main()
{

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
        rdtemp = rdset;
        // select的最后一个参数超时检测，如果指定为NULL 则代表一直检测直到有就绪的fd
        int num = select(maxfd+1, &rdtemp, NULL, NULL, NULL);
        // 判断是不是监听的fd
        if (FD_ISSET(lfd,&rdtemp))
        {

            struct sockaddr_in cliaddr;
            int cliLen = sizeof(cliaddr);
            int cfd = accept(lfd, (struct sockaddr*)&cliaddr, &cliLen);

            // 把得到的通信文件描述符放到fd集合中，让通信的时候可以检测到
            FD_SET(cfd, &rdset);

            maxfd = cfd > maxfd ? cfd : maxfd;
        }
        // 判断是不是通信的描述符，因为通信可以有很多个所以这里用循环检测
        for (int i = 0; i < maxfd+1; i++)
        {
            // 如果i不是监听的文件描述符并且在fd集合中就绪 则是一个通信的fd
            if (i != lfd && FD_ISSET(i, &rdtemp))
            {

                char buf[10] = {0};

                int  len = recv(i, buf, sizeof(buf),0);
                if (len == 0)
                {
                    printf("客户端关闭连接...\n");

                    FD_CLR(i, &rdset);
                    close(i);
                }
                else if (len > 0)
                {
                    printf("read buf = %s\n",buf);
                    for (int i = 0; i < len; i++)
                        buf[i] = toupper(buf[i]);
                    printf("after buf = %s\n", buf);
                    int  ret = send(i, buf, strlen(buf)+1, 0);
                    if (ret == -1)
                    { perror("send"); return -1; }
                }
                else
                    perror("recv");
            }
        }
    }
    close(lfd);
    return 0;
}

