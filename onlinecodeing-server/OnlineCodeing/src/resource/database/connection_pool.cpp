#include "database/connection_pool.h"
#include <iostream>
#include <memory>

bool ConnectionPool::init(const ConnectionPoolConfig& pool_config)
{
    if (pool_config.host.empty() || pool_config.port.empty() || pool_config.username.empty() || pool_config.dbname.empty())
    {
        std::cerr << "[error]ConnectionPool::init: invalid pool config." << std::endl;
        return false;
    }

    if (pool_config.min_size < 0 || pool_config.timeout_ms < 0 || pool_config.max_size < pool_config.min_size)
    {
        std::cerr << "[error]ConnectionPool::init: invalid pool size." << std::endl;
        return false;
    }

    this->m_config = pool_config;
    this->BasePool<sql::Connection>::m_config.min_size = pool_config.min_size;
    this->BasePool<sql::Connection>::m_config.max_size = pool_config.max_size;
    this->BasePool<sql::Connection>::m_config.timeout_ms = pool_config.timeout_ms;

    this->m_driver = sql::mysql::get_mysql_driver_instance();
    if (!this->m_driver)
    {
        std::cerr << "[error]ConnectionPool::init: get mysql driver failed." << std::endl;
        return false;
    }

    std::lock_guard<std::mutex> lock(this->m_queue_mutex);
    for (int i = 0; i < this->m_config.min_size; i++)
    {
        auto conn = this->createConnection();
        if (conn)
        {
            this->m_idle_queue.push(std::move(conn));
        }
        else
        {
            std::cerr << "[warn]ConnectionPool::init: create " << i << "th connection failed." << std::endl;
        }
    }
    std::cout << "[info]ConnectionPool::init: initialized, " <<
        "Host: " << this->m_config.host << ":" << this->m_config.port <<
        "Idle: " << this->m_idle_queue.size() << ", " <<
        "Min: " << this->m_config.min_size << ", " <<
        "Max: " << this->m_config.max_size << "." << std::endl;

    return true;
}

std::unique_ptr<PooledConnection> ConnectionPool::getConnection(int timeout_ms)
{
    std::unique_lock<std::mutex> lock(this->m_queue_mutex);
    std::unique_ptr<sql::Connection> conn;

    std::chrono::steady_clock::time_point wait_start = std::chrono::steady_clock::now();
    while (true)
    {
        while (!this->m_idle_queue.empty())
        {
            conn = std::move(this->m_idle_queue.front());
            this->m_idle_queue.pop();

            if (this->validateConnection(conn.get()))
            {
                this->m_used_count++;
                return std::unique_ptr<PooledConnection>(new PooledConnection(std::move(conn), this));
            }
            else
            {
                conn.reset();
            }
        }
        if (this->m_used_count < this->m_config.max_size)
        {
            conn = this->createConnection();
            if (conn)
            {
                this->m_used_count++;
                return std::unique_ptr<PooledConnection>(new PooledConnection(std::move(conn), this));
            }
        }

        std::chrono::milliseconds wait_duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - wait_start);
        if (wait_duration.count() >= timeout_ms)
        {
            lock.unlock();
            std::cerr << "[error]ConnectionPool::getConnection: get connection timeout " << timeout_ms << " ms." << std::endl;
            return nullptr;
        }

        this->m_cv.wait_for(lock, std::chrono::milliseconds(timeout_ms - wait_duration.count()), [this]()
            {
                return !this->m_idle_queue.empty();
            });
    }
}

std::unique_ptr<sql::Connection> ConnectionPool::createConnection()
{
    try
    {
        std::unique_ptr<sql::Connection> conn(this->m_driver->connect("tcp://" + this->m_config.host + ":" + this->m_config.port, this->m_config.username, this->m_config.password));
        conn->setSchema(this->m_config.dbname);
        conn->setAutoCommit(true);
        conn->setClientOption("charset", "utf8mb4");

        return conn;
    }
    catch (sql::SQLException& e)
    {
        std::cerr << "[error]ConnectionPool::createConnection: "
            << "Code: " << e.getErrorCode() << ", "
            << "SQLState: " << e.getSQLState() << ", "
            << "Msg: " << e.what() << "." << std::endl;
        return nullptr;
    }
}

bool ConnectionPool::validateConnection(sql::Connection* conn)
{
    if (!conn) return false;

    try
    {
        std::unique_ptr<sql::Statement> stmt(conn->createStatement());
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT 1"));

        return res && res->next();
    }
    catch (sql::SQLException& e)
    {
        (void)e;
        return false;
    }
}

void ConnectionPool::returnConnection(std::unique_ptr<sql::Connection> conn)
{
    if (!conn) return;

    std::lock_guard<std::mutex> lock(this->m_queue_mutex);
    if (this->m_used_count <= 0)
    {
        std::cerr << "[warn]ConnectionPool::returnConnection: used_count is 0, skip decrement." << std::endl;
        return;
    }

    if (this->validateConnection(conn.get()))
    {
        this->m_idle_queue.push(std::move(conn));
        this->m_cv.notify_one();
    }
    else
    {
        std::cerr << "[warn]ConnectionPool::returnConnection: invalid connection, discard." << std::endl;
    }

    this->m_used_count--;
}

PooledConnection::PooledConnection(std::unique_ptr<sql::Connection> conn, ConnectionPool* pool)
    : m_connection(std::move(conn)), m_parent_pool(pool)
{}

PooledConnection::~PooledConnection()
{
    this->releaseConnection();
    return;
}

void PooledConnection::releaseConnection()
{
    if (!this->m_connection) return;

    if (!this->m_parent_pool)
    {
        std::cerr << "[warn]PooledConnection::returnConnection: pool has been destroyed." << std::endl;
        return;
    }

    this->m_parent_pool->returnConnection(std::move(this->m_connection));
}