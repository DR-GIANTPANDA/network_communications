#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// u:unsigned
// 16: 16位, 32:32位
// h: host, 主机字节序
// n: net, 网络字节序
// s: short
// l: int

// 这套api主要用于 网络通信过程中 IP 和 端口 的 转换
// 将一个短整形从主机字节序 -> 网络字节序
uint16_t htons(uint16_t hostshort);

// 将一个短整形从网络字节序 -> 主机字节序
uint16_t ntohs(uint16_t netshort);

// 主机字节序的IP地址转换为网络字节序
// 主机字节序的IP地址是字符串, 网络字节序IP地址是整形
int inet_pton(int af, const char *src, void *dst); 
// 将大端的整形数, 转换为小端的点分十进制的IP地址        
const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);



int main(int argc, char* argv[])
{
    // 1.创建监听的套接字
    // 一个文件描述符对应2块内存，一块内存是读缓冲区，一块是写缓冲区
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

    // 4. 阻塞并等待客户端的连接
    
    struct sockaddr_in caddr;  // 这里创建一个可以接受客户端的ip和端口的变量
    int addrlen = sizeof(caddr);
    // cfd是accept返回的一个通信的文件描述符
    // 这是一个阻塞函数，如果没有客户端的请求 每次只能用于和一个客户端连接
    // 如果需要和多个客户端连接则需要调用N次
    int cfd = accept(fd, (struct sockaddr*)&caddr, &addrlen);

    if (cfd == -1)
    { perror("accept"); return -1; }

    // 连接成功，打印客户端的IP端口信息。 这一步可以省略
    // 保存在caddr里的IP和端口都是大端 要转换成小端使用。
    char ip[32];
    printf("客户端的IP：%s， 端口：%d\n", inet_ntop(AF_INET, &caddr.sin_addr, ip, sizeof(ip)),
                                                    ntohs(caddr.sin_port));


    // 5. 通信
    
    while(1)
    {
        // 接受数据  recv第一个参数是通信的文件描述符 也就是accept返回的数
        char buffer[1024];
        int len = recv(cfd, buffer, sizeof(buffer), 0);
        if (len>0)
        {
            printf("client say: %s\n", buffer);
            send(cfd, buffer, len, 0);
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
    close(fd);
    close(cfd);

    return 0;
}



