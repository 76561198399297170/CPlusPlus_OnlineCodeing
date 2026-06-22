#ifndef _AGENT_POOL_H_
#define _AGENT_POOL_H_

#include "utils/pool.h"
#include "utils/singleton.h"
#include "agent/agent.h"

struct AgentPoolConfig
{
	std::string ai_host = "172.23.207.64";
	int ai_port = 8000;
	int min_size = 5;
	int max_size = 20;
	int timeout_ms = 12500;
};

class PooledAgent;

class AgentPool : public BasePool<Agent>, public Singleton<AgentPool>
{
	friend class Singleton<AgentPool>;
	friend class PooledAgent;

public:
	//初始化连接池
	bool init(const AgentPoolConfig& pool_config);

	//获取一个包装对象，离开作用域自动归还
	std::unique_ptr<PooledAgent> getExecutor(int timeout_ms = 1000);

	//空闲连接数
	size_t getIdleCount() const { return BasePool<Agent>::getIdleCount(); }

	//析构公有保证单例释放能够访问
	~AgentPool() override = default;

private:
	//私有构造
	AgentPool() = default;

	//重写init
	bool init(const BasePool<Agent>::BasePoolConfig& config) override { return false; }

	//创建连接函数
	std::unique_ptr<Agent> createResource() override { return this->createConnection(); }

	//重写验证连接
	bool validateResource(Agent* conn) override { return this->validateConnection(conn); }

	//创建连接函数
	std::unique_ptr<Agent> createConnection();

	//验证连接函数
	bool validateConnection(Agent* conn);

	//归还连接到池
	void returnConnection(std::unique_ptr<Agent> conn);

private:
	AgentPoolConfig m_config;

};

class PooledAgent
{
	friend class AgentPool;

private:
	//构造只能由连接池创建
	PooledAgent(std::unique_ptr<Agent> conn, AgentPool* pool);

	//归还连接到池
	void releaseConnection();

public:
    //释放自动归还对象池
    ~PooledAgent();

    //禁止拷贝避免重复归还
	PooledAgent(const PooledAgent&) = delete;
	PooledAgent& operator=(const PooledAgent&) = delete;
	PooledAgent(PooledAgent&&) = default;
	PooledAgent& operator=(PooledAgent&&) = default;

    //供业务代码使用获取原始指针
    Agent* get() const { return this->m_connection.get(); }

    //手动释放提前归还
    void release() { if (this->m_connection) { this->releaseConnection(); this->m_connection.reset(); } }

private:
    std::unique_ptr<Agent> m_connection;     //数据库连接
    AgentPool* m_parent_pool;                //父对象池

};

#endif // !_AGENT_POOL_H_
