#include "socket.h"
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
int main()
{
    int fd = createSocket();
    if (fd == -1)
        return -1;

    int ret = connectToHost(fd, "192.168.1.106",5555);
    if (ret == -1)
        return -1;


    int fdl = open("english.txt", O_RDONLY);
    int length = 0;
    char tmp[1000];
    while ((length = read(fdl, tmp, rand()%1000)) > 0)
    {
        sendMsg(fd, tmp, length);

        memset(tmp, 0, sizeof(tmp));
        usleep(300);
    }
    
    sleep(10);

    closeSocket(fd);

    return 0;
}


