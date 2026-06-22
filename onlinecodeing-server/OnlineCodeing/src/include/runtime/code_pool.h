#ifndef _CODE_POOL_H_
#define _CODE_POOL_H_

#include "utils/pool.h"
#include "utils/singleton.h"
#include "runtime/executor.h"

struct CodePoolConfig
{
    int min_size = 5;                   //池最小大小
    int max_size = 20;                  //池最大大小
    int timeout_ms = 1000;              //连接超时时长
    int compile_limit_ms = 3000;        //编译时长限制
    std::string temp_dir = "C:\\Users\\Administrator\\Desktop\\OnlineCodeing\\onlinecodeing-server\\OnlineCodeing\\res\\temp";      //临时代码创建根目录
};

class PooledCode;

//代码池主类
class CodePool : public BasePool<CodeExecutor>, public Singleton<CodePool>
{
	friend class Singleton<CodePool>;
	friend class PooledCode;

public:
    //初始化连接池
    bool init(const CodePoolConfig& pool_config);

    //获取一个包装对象，离开作用域自动归还
    std::unique_ptr<PooledCode> getExecutor(int timeout_ms = 1000);

    //空闲连接数
    size_t getIdleCount() const { return BasePool<CodeExecutor>::getIdleCount(); }

    //析构公有保证单例释放能够访问
    ~CodePool() override = default;

private:
    //私有构造
    CodePool() = default;

    //重写创建新连接
    std::unique_ptr<CodeExecutor> createResource() override { return this->createConnection(); }

    //重写验证连接
    bool validateResource(CodeExecutor* conn) override { return this->validateConnection(conn); }

    //重写init
    bool init(const BasePool<CodeExecutor>::BasePoolConfig& config) override { return false; }

    //创建连接函数
    std::unique_ptr<CodeExecutor> createConnection();

    //验证连接函数
    bool validateConnection(CodeExecutor* conn);

    //归还连接到池
    void returnConnection(std::unique_ptr<CodeExecutor> conn);

private:
    CodePoolConfig m_config;                //配置对象

};

//代码运行包装类
class PooledCode
{
    friend class CodePool;

private:
    //构造只能由连接池创建
    PooledCode(std::unique_ptr<CodeExecutor> conn, CodePool* pool);

    //归还连接到池
    void releaseConnection();

public:
    //释放自动归还对象池
    ~PooledCode();

    //禁止拷贝避免重复归还
    PooledCode(const PooledCode&) = delete;
    PooledCode& operator=(const PooledCode&) = delete;
    PooledCode(PooledCode&&) = default;
    PooledCode& operator=(PooledCode&&) = default;

    //供业务代码使用获取原始指针
    CodeExecutor* get() const { return this->m_connection.get(); }

    //手动释放提前归还
    void release() { if (this->m_connection) { this->releaseConnection(); this->m_connection.reset(); } }

private:
    std::unique_ptr<CodeExecutor> m_connection;     //数据库连接
    CodePool* m_parent_pool;                        //父对象池
};

#endif // !_CODE_POOL_H_
