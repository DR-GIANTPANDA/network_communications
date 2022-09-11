#ifndef _MYSQLCONN_H
#define _MYSQLCONN_H
#include <iostream>
#include <mysql/mysql.h>
#include <chrono>
using namespace std::chrono;
using namespace std;
// 封装mysql的C函数类
class MysqlConn 
{
public:
// 初始化数据库连接
    MysqlConn();
// 释放数据库连接
    ~MysqlConn();
// 连接数据库
    bool connect(string user, string passwd, string dbName, string ip, unsigned short port = 3306);
// 更新数据库：insert，update，delete，
    bool update(string sql);
// 查询数据库
    bool query(string sql);
// 遍历查询得到的结果集
    bool next();

    // 得到结果集中的字段
    string value(int index);

    // 事务操作
    bool transaction();

    // 事务操作 提交事务
    bool commit();
    // 事务回滚
    bool rollback();

    // 刷新起始的空闲时间点
    void refreshAliveTime();
    // 计算连接存活的总时长
    long long getAliveTime();
private:
    void freeResult();
    // 创建一个变量得到mysql的地址
    MYSQL* m_conn = nullptr;
    MYSQL_RES* m_result = nullptr;
    MYSQL_ROW m_row = nullptr;
    std::chrono:: steady_clock::time_point m_alivetime;
};


#endif

