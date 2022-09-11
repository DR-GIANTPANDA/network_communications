#include "ConnectionPool.h"
#include <json/json.h>
#include <fstream>
#include <thread>
using namespace Json;

ConnectionPool* ConnectionPool::getConnectPool()
{
    // 静态的局部对象只会有一个
    static ConnectionPool pool;
    return &pool;
}
// 对外的接口 给外面需要连接数据库的一个接口 消费者 因为一直用数据库池里的连接数
shared_ptr<MysqlConn> ConnectionPool::getConnection()
{
    // 阻塞线程。如果连接的队列是空的
    unique_lock<mutex> locker(m_mutexQ);
    while (m_connectionQ.empty())
    {
        // 调用wait_for阻塞的毫秒数，保存在m_timeout，
        if (cv_status::timeout == m_cond.wait_for(locker, chrono::milliseconds(m_timeout)))
        {
            if (m_connectionQ.empty())
            {
                //return nullptr;
                continue;
            }
        }
    }
    // 如果存储的队列不为空就从队列头取出一个可用的连接。
    // 当用完之后把这个连接还给数据库池
    // 如何实现用完还给，用智能指针，共享智能指针 当智能指针析构的时候还给数据池
    shared_ptr<MysqlConn> connptr(m_connectionQ.front(), [this](MysqlConn* conn) {
        lock_guard<mutex> locker(m_mutexQ); // 需要加锁 因为是共享资源
        conn->refreshAliveTime(); // 更新时间戳
        m_connectionQ.push(conn);
        });
    m_connectionQ.pop();// 从队列里弹出
    m_cond.notify_all();// 唤醒生产数据库连接数的生产者，当消费完之后
    return connptr;
}

bool ConnectionPool::parseJsonFile()
{
    ifstream ifs("dbconf.json");
    Reader rd;
    Value root;
    rd.parse(ifs,root);
    if (root.isObject())
    {
        m_ip = root["ip"].asString();
        m_port = root["port"].asInt();
        m_user = root["userName"].asString();
        m_passwd = root["password"].asString();
        m_dbName = root["dbName"].asString();
        m_minSize = root["minSize"].asInt();
        m_maxSize = root["maxSize"].asInt();
        m_maxIdleTime = root["maxIdleTime"].asInt();
        m_timeout = root["timeout"].asInt();
        return true;
    }
    return false;
}
// 生产数据库池的连接数，一直生产
void ConnectionPool::produceConnection()
{
    while (true)
    {
        // 这个互斥锁可以被locker对象管理，析构的时候解锁
        unique_lock<mutex> locker(m_mutexQ);
        while(m_connectionQ.size() >= m_minSize && m_connectionQ.size() <= m_maxSize)
        {
            // 如果连接池的数量多阻塞
            m_cond.wait(locker);
        }
        addConnection();
        m_cond.notify_all();  // 生产者和消费者使用的是同一个条件变量
    }
}

void ConnectionPool::recycleConnection()
{
    while (true)
    {

        // 周期性检测 ，指定一个时间长度休眠，比如1秒
        this_thread::sleep_for(chrono::seconds(1));// 每隔一秒钟检测一次
        lock_guard<mutex> locker(m_mutexQ);
        while(m_connectionQ.size() > m_minSize && m_connectionQ.size() < m_maxSize)
        {
            // 队头的元素肯定是空闲时长最长的。因为先进先出
            MysqlConn* conn = m_connectionQ.front();
            // 计算队头存活的时长是否大于空闲的时长；
            if (conn->getAliveTime() >= m_maxIdleTime)
            {
                // 如果满足则从队列里弹出来删除
                m_connectionQ.pop();
                delete conn;
            }
            else
            {
                break;
            }
        }
    }
    
}


void ConnectionPool::addConnection()
{
        
        MysqlConn* conn = new MysqlConn;
        conn->connect(m_user, m_passwd, m_dbName, m_ip, m_port);
        conn->refreshAliveTime();
        // 添加一个新的数据库连接需要记录时间戳
        m_connectionQ.push(conn);// 添加到队列
}
ConnectionPool::ConnectionPool()
{
    // 解析json文件
    if(!parseJsonFile())
    {
        return;
    }
    // 默认创建多少个有效的数据库连接
    for (int i = 0; i < m_minSize; ++i)
    {
        addConnection();
    }
    // 为了维护数据库连接池
    // 创建2个子线程去检测需要创建和销毁的连接池的数量
    // 指定任务函数
    thread producer(&ConnectionPool::produceConnection, this);
    thread recycler(&ConnectionPool::recycleConnection, this);
    producer.detach(); // 线程分离
    recycler.detach();

}

ConnectionPool::~ConnectionPool()
{
    // 如果队里不为空，释放资源
    while (!m_connectionQ.empty())
    {
        MysqlConn* conn = m_connectionQ.front();
        m_connectionQ.pop();
        delete conn;
    }
}

