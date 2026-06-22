#ifndef _CONNECTION_POOL_H_
#define _CONNECTION_POOL_H_

#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/connection.h>
#include <jdbc/cppconn/statement.h>

#include <memory>
#include <queue>
#include <mutex>
#include <string>
#include <condition_variable>
#include <chrono>
#include <iostream>

#include "utils/pool.h"
#include "utils/singleton.h"

//连接池配置
struct ConnectionPoolConfig
{
    std::string host;       //链接主机
    std::string port;       //链接端口
    std::string username;   //数据库用户名
    std::string password;   //数据库密码
    std::string dbname;     //数据库名称
    int min_size = 5;        //池最小大小
    int max_size = 20;       //池最大大小
    int timeout_ms = 1000;  //连接超时时长
};

class PooledConnection;

//连接池主类
class ConnectionPool : public BasePool<sql::Connection>, public Singleton<ConnectionPool>
{
    friend class Singleton<ConnectionPool>;
    friend class PooledConnection;

public:
    //初始化连接池
    bool init(const ConnectionPoolConfig& pool_config);

    //获取一个包装对象，离开作用域自动归还
    std::unique_ptr<PooledConnection> getConnection(int timeout_ms = 1000);

    //空闲连接数
    size_t getIdleCount() const { return BasePool<sql::Connection>::getIdleCount(); }

    //析构公有保证单例释放能够访问
    ~ConnectionPool() override = default;

private:
    //私有构造
    ConnectionPool() = default;

    //重写创建新连接
    std::unique_ptr<sql::Connection> createResource() override { return createConnection(); }

    //重写验证连接
    bool validateResource(sql::Connection* conn) override { return validateConnection(conn); }

    //重写init
    bool init(const BasePool<sql::Connection>::BasePoolConfig& config) override { return false; }

    //创建连接函数
    std::unique_ptr<sql::Connection> createConnection();

    //验证连接函数
    bool validateConnection(sql::Connection* conn);

    //归还连接到池
    void returnConnection(std::unique_ptr<sql::Connection> conn);

private:
    ConnectionPoolConfig m_config;                    //配置对象
    sql::mysql::MySQL_Driver* m_driver = nullptr;     //MySQL驱动

};

//连接包装类
class PooledConnection
{
    friend class ConnectionPool;

private:
    //构造只能由连接池创建
    PooledConnection(std::unique_ptr<sql::Connection> conn, ConnectionPool* pool);

    //归还连接到池
    void releaseConnection();

public:
    //释放自动归还对象池
    ~PooledConnection();

    //禁止拷贝避免重复归还
    PooledConnection(const PooledConnection&) = delete;
    PooledConnection& operator=(const PooledConnection&) = delete;
    PooledConnection(PooledConnection&&) = default;
    PooledConnection& operator=(PooledConnection&&) = default;

    //是否链接生效
    bool isValid() const { return m_connection && m_connection->isValid(); }

    //供业务代码使用获取原始指针
    sql::Connection* get() const { return this->m_connection.get(); }

    //手动释放提前归还
    void release() { if (this->m_connection) { this->releaseConnection(); this->m_connection.reset(); } }

private:
    std::unique_ptr<sql::Connection> m_connection;      //数据库连接
    ConnectionPool* m_parent_pool;                      //父对象池
};

//事务守卫类
class TransactionGuard
{
public:
    //构造关闭自动提交开启事务
    explicit TransactionGuard(sql::Connection* conn)
        : m_conn(conn)
    {
        if (this->m_conn)
        {
            this->m_auto_commit = this->m_conn->getAutoCommit();
            this->m_conn->setAutoCommit(false);
        }
    }

    //析构恢复原始状态
    ~TransactionGuard()
    {
        if (this->m_conn)
        {
            this->m_conn->setAutoCommit(this->m_auto_commit);
        }
    }

    //禁用拷贝与移动
    TransactionGuard(const TransactionGuard&) = delete;
    TransactionGuard& operator=(const TransactionGuard&) = delete;

private:
    sql::Connection* m_conn;    //连接对象
    bool m_auto_commit;         //自动提交事务
};

// ResultSet 包装类，用于管理 ResultSet、PreparedStatement 和连接的生命周期
class PooledResultSet
{
public:
    PooledResultSet(std::unique_ptr<PooledConnection> conn, 
                   std::unique_ptr<sql::PreparedStatement> stmt, 
                   std::unique_ptr<sql::ResultSet> res)
        : m_connection(std::move(conn)), 
          m_statement(std::move(stmt)), 
          m_result_set(std::move(res))
    {
    }

    // 禁用拷贝
    PooledResultSet(const PooledResultSet&) = delete;
    PooledResultSet& operator=(const PooledResultSet&) = delete;

    // 允许移动
    PooledResultSet(PooledResultSet&&) = default;
    PooledResultSet& operator=(PooledResultSet&&) = default;

    ~PooledResultSet() = default;

    // ResultSet 接口代理
    bool next() { return m_result_set ? m_result_set->next() : false; }
    
    bool first() { return m_result_set ? m_result_set->first() : false; }
    
    bool last() { return m_result_set ? m_result_set->last() : false; }
    
    bool isBeforeFirst() { return m_result_set ? m_result_set->isBeforeFirst() : false; }
    
    bool isAfterLast() { return m_result_set ? m_result_set->isAfterLast() : false; }
    
    bool isFirst() { return m_result_set ? m_result_set->isFirst() : false; }
    
    bool isLast() { return m_result_set ? m_result_set->isLast() : false; }
    
    void beforeFirst() { if (m_result_set) m_result_set->beforeFirst(); }
    
    void afterLast() { if (m_result_set) m_result_set->afterLast(); }

    std::string getString(const std::string& columnLabel) 
    { 
        return m_result_set ? m_result_set->getString(columnLabel) : ""; 
    }
    
    std::string getString(unsigned int columnIndex) 
    { 
        return m_result_set ? m_result_set->getString(columnIndex) : ""; 
    }

    int getInt(const std::string& columnLabel) 
    { 
        return m_result_set ? m_result_set->getInt(columnLabel) : 0; 
    }
    
    int getInt(unsigned int columnIndex) 
    { 
        return m_result_set ? m_result_set->getInt(columnIndex) : 0; 
    }

    long long getInt64(const std::string& columnLabel) 
    { 
        return m_result_set ? m_result_set->getInt64(columnLabel) : 0; 
    }
    
    long long getInt64(unsigned int columnIndex) 
    { 
        return m_result_set ? m_result_set->getInt64(columnIndex) : 0; 
    }

    double getDouble(const std::string& columnLabel) 
    { 
        return m_result_set ? m_result_set->getDouble(columnLabel) : 0.0; 
    }
    
    double getDouble(unsigned int columnIndex) 
    { 
        return m_result_set ? m_result_set->getDouble(columnIndex) : 0.0; 
    }

    bool getBoolean(const std::string& columnLabel) 
    { 
        return m_result_set ? m_result_set->getBoolean(columnLabel) : false; 
    }
    
    bool getBoolean(unsigned int columnIndex) 
    { 
        return m_result_set ? m_result_set->getBoolean(columnIndex) : false; 
    }

    bool isNull(const std::string& columnLabel) 
    { 
        return m_result_set ? m_result_set->isNull(columnLabel) : true; 
    }
    
    bool isNull(unsigned int columnIndex) 
    { 
        return m_result_set ? m_result_set->isNull(columnIndex) : true; 
    }

    sql::ResultSet* get() { return m_result_set.get(); }
    
    operator bool() const { return m_result_set != nullptr; }

private:
    std::unique_ptr<PooledConnection> m_connection;
    std::unique_ptr<sql::PreparedStatement> m_statement;
    std::unique_ptr<sql::ResultSet> m_result_set;
};

#endif // !_CONNECTION_POOL_H_
