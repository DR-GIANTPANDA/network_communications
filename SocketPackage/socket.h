#ifndef _SOCKET_H
#define _SOCKET_H

typedef struct sockaddr_in sockaddr_in;
// 服务器
// 绑定 + 监听
int setListen(int lfd, unsigned short port);
int acceptConn(int lfd,sockaddr_in* addr );


// 客户端
int connectToHost(int fd, const char* ip, unsigned short port);



// 共用
int createSocket();
int sendMsg(int fd, char* msg, int len);
int recvMsg(int fd, char** msg);
int closeSocket(int fd);

#endif





