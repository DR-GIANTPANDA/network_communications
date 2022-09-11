#include "MysqlConn.h"

MysqlConn::MysqlConn()
{
    // 初始化环境，然后设置字符类型utf8防止中午乱码
    m_conn = mysql_init(nullptr);
    mysql_set_character_set(m_conn, "utf8");
}

MysqlConn::~MysqlConn()
{
    // 释放掉mysql连接
    if (m_conn!=nullptr)
        mysql_close(m_conn);
    freeResult();//析构
}

bool MysqlConn::connect(string user, string passwd, string dbName, string ip, unsigned short port )
{
    // 建立建立 c_str 转换成char*类型
    MYSQL* ptr = mysql_real_connect(m_conn, ip.c_str(), user.c_str(), passwd.c_str(), dbName.c_str(), port, nullptr, 0);
    return ptr != nullptr;
}

bool MysqlConn::update(string sql)
{
    // 执行sql语句char*类型返回0说明成功
    if (mysql_query(m_conn, sql.c_str()))
        return false;
    return true;
}

bool MysqlConn::query(string sql)
{
    freeResult();// 清空上次的查询
    if (mysql_query(m_conn, sql.c_str()))
        return false;
    // 把查询到的结果从服务器端保存到本地
    // m_result是一块地址，类型是MYSQL_RES
    m_result =  mysql_store_result(m_conn);
    return true;
}

bool MysqlConn::next()
{
    if (m_result != nullptr)
    {
        // 返回一个二级指针
        // 保存的是这个字段值所有数据
        m_row =  mysql_fetch_row(m_result);
        if (m_row != nullptr)
        {
            return true;
        }
    }
    return false;
}

string MysqlConn::value(int index)
{
    // 首先得到该字段的数量也就是有多少列
    int colCount = mysql_num_fields(m_result);
    // 如果传的index有问题返回一个空字符串
    if (index >= colCount || index < 0)
        return string();
    // m_row是二级指针
    char* val = m_row[index];
    // 得到该字段值的长度
    unsigned long lenght =  mysql_fetch_lengths(m_result)[index];
    return string(val,lenght);
}
// 事务相关的3个函数
bool MysqlConn::transaction()
{
    // 设置手动提交
    return mysql_autocommit(m_conn, false);
}

bool MysqlConn::commit()
{
    return mysql_commit(m_conn);
}

bool MysqlConn::rollback()
{   
    // 数据回滚 等于没有做任务操作
    return mysql_rollback(m_conn);
}

// 计算这个连接被创建出的起始时间点
void MysqlConn::refreshAliveTime()
{
    m_alivetime = steady_clock::now();
}

long long MysqlConn::getAliveTime()
{
    // 得到新的时间段 也就是没有被使用的时候的时间长短
    // 纳秒转换成毫秒
    nanoseconds res = steady_clock::now() - m_alivetime;
    milliseconds millsec = duration_cast<milliseconds>(res);
    return millsec.count();
}

void MysqlConn::freeResult()
{
    // 如果结果集的数据读完了
    // 该内存地址指向的地址就被释放
    if (m_result)
    {
        mysql_free_result(m_result);
        m_result = nullptr;
    }
}

