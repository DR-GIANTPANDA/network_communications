#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
//#include <sys/types.h>
//#include <sys/socket.h>
#include <arpa/inet.h>


int main(int argc, char* argv[])
{
    // 1.创建通信的套接字
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    { perror("socket"); return -1; }

    // 2. 连接服务器IP port 
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(5555); // 转换成网络端口大端形式
    inet_pton(AF_INET, "192.168.226.1",&saddr.sin_addr.s_addr);
    int ret = connect(fd,(struct sockaddr*)&saddr, sizeof(saddr));
    if (ret == -1)
    { perror("connect"); return -1; }



    // 3. 通信
    int number = 0;
    while(1)
    {
        // 发送数据
        char buffer[1024];
        sprintf(buffer, "你好，hello world，%d...\n", number++);
        send(fd, buffer, strlen(buffer)+1, 0);

        //接收数据
        memset(buffer, 0, sizeof(buffer));
        int len = recv(fd, buffer, sizeof(buffer), 0);
        if (len>0)
        {
            printf("server say: %s\n", buffer);
        }
        else if (len == 0)
        {
            printf("服务端断开连接...\n");
            break;
        }
        else
        {
            perror("recv");
            break;
        }
        sleep(1);
    }

    // 关闭文件描述符
    close(fd);

    return 0;
}



