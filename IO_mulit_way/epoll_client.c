#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

int main()
{

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    { perror("socket"); return -1;}


    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5555);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);
    int ret = connect(fd,(struct sockaddr*)&addr, sizeof(addr));

    if (ret == -1)
    { perror("connenct"); return -1; }

    int num = 0;
    while(1)
    {
        char recvBuf[1024] = {0};
        //       sprintf(recvBuf, "hello, world, %d\n...",num++);
        fgets(recvBuf, sizeof(recvBuf), stdin);
        write(fd, recvBuf, strlen(recvBuf)+1);

        int len =  read(fd, recvBuf, sizeof(recvBuf));
        if (len == -1)
        { perror("read"); return -1; }
        printf("recv buf: %s\n", recvBuf);
        sleep(1);
    }

    close(fd);
    return 0;
}

