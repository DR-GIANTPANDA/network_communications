#ifndef _CONNECTIONPOOL_H
#define _CONNECTIONPOOL_H 

#include <queue>
#include "MysqlConn.h"
#include <mutex>
#include <condition_variable>
using namespace std;
// 单例模式，因为数据库池有一个对象就够了
// 懒汉模式，饿汉模式
class ConnectionPool
{
public:
    // 添加一个静态方法
    static ConnectionPool* getConnectPool();
    ConnectionPool(const ConnectionPool& obj) = delete;
    ConnectionPool& operator=(const ConnectionPool& obj) = delete;
    shared_ptr<MysqlConn> getConnection();
    ~ConnectionPool();
private:
    // 构造函数是私有的
    ConnectionPool();
    // 写一个配置文件处理要连接的数据库的信息
    bool parseJsonFile();    
    
    void produceConnection();
    void recycleConnection();
    void addConnection();


    // 通过以下属性保证和数据库连接
    string m_ip;
    string m_user;
    string m_passwd;
    string m_dbName;
    unsigned short m_port;
    int m_minSize;
    int m_maxSize;
    int m_timeout;
    int m_maxIdleTime;// 给线程指定一个超时
    // 时长等待连接，空闲时长如果太多释放
    // 在这个队列存储若干个有效的数据库连接
    queue<MysqlConn*> m_connectionQ;
    mutex m_mutexQ;// 保证数据库连接池不混乱，因为要多个线程访问
    condition_variable m_cond; // 阻塞
};

#endif

