#include <stdio.h>
#include <mysql/mysql.h>


int main()
{

    // 初始化连接环境
    MYSQL* mysql = mysql_init(nullptr);
    if (mysql == nullptr)
    {
        printf("mysql_init() error\n");
        return -1;
    }

    mysql = mysql_real_connect(mysql, "localhost", "root", "st19216811", "testdb", 0, NULL, 0);

    if (mysql == nullptr)
    {
        printf("mysql_real_connect() error\n");
        return -1;
    }

    printf("mysql api使用的默认编码：%s\n", mysql_character_set_name(mysql));

    // 设置编码为utf8
    printf("mysql api使用修改之后的编码：%s\n", mysql_character_set_name(mysql));

    printf("恭喜， 连接数据库服务器成功了...\n");

    // 3. 执行一个sql语句
    // 查询testdb数据库下的dept部门表
    const char* sql = "select * from dept";

    // 执行这个sql语句
    int ret = mysql_query(mysql, sql);
    if (ret != 0)
    {
        printf("mysql_query 失败了， 原因: %s\n", mysql_error(mysql));
        return -1;
    }

    // 将结果集从mysql对象中取出来，MYSQL_RES对应一块内存，里边保存着这个查询之后得到的结果
    // 从服务器端保存到本地端
    MYSQL_RES* res = mysql_store_result(mysql);
    if(res == NULL)
    {
        printf("mysql_store_result() 失败了, 原因: %s\n", mysql_error(mysql));
        return -1;
    }
// 查询返回结果集中列的个数
    int num = mysql_num_fields(res);
// 得到存储头信息的数组的地址
    MYSQL_FIELD* fields = mysql_fetch_fields(res);
    for (int i = 0; i < num; ++i)
    {
        printf("%s\t\t", fields[i].name);
    }
    printf("\n");

    MYSQL_ROW row;
    while( (row = mysql_fetch_row(res)) != nullptr)
    {

        for (int i = 0; i < num; ++i)
        {
            printf("%s\t\t", row[i]);
        }
        printf("\n");
    }

    mysql_free_result(res);
// 事务操作，保证数据库里的数据正确性
    mysql_autocommit(mysql, 0);
    int ret1 = mysql_query(mysql, "insert into dept values(61, '海军', '圣地玛丽乔亚')");
    int ret2 = mysql_query(mysql, "insert into dept values(62, '七武海', '世界各地')");
    int ret3 = mysql_query(mysql, "insert into dept values(63, '四皇', '新世界')");
    printf("ret1 = %d, ret2 = %d, ret3 = %d\n", ret1, ret2, ret3);

    if (ret1 == 0 && ret2 == 0 && ret3 == 0)
    {
        mysql_commit(mysql);
    }
    else
    {
        mysql_rollback(mysql);
    }

    mysql_close(mysql);

    return 0;
}

