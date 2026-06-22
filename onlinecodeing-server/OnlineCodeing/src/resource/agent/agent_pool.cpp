#include "agent/agent_pool.h"

bool AgentPool::init(const AgentPoolConfig& pool_config)
{

    if (pool_config.ai_host.empty())
    {
        std::cerr << "[error]AgentPool::init: invalid pool config." << std::endl;
        return false;
    }

    if (pool_config.ai_port < 1000 || pool_config.ai_port >= 10000 || pool_config.min_size < 0 || pool_config.timeout_ms < 0 || pool_config.max_size < pool_config.min_size)
    {
        std::cerr << "[error]AgentPool::init: invalid pool size." << std::endl;
        return false;
    }

    this->m_config = pool_config;
    this->BasePool<Agent>::m_config.min_size = pool_config.min_size;
    this->BasePool<Agent>::m_config.max_size = pool_config.max_size;
    this->BasePool<Agent>::m_config.timeout_ms = pool_config.timeout_ms;

    std::lock_guard<std::mutex> lock(this->m_queue_mutex);
    for (int i = 0; i < this->m_config.min_size; i++)
    {
        auto conn = createConnection();
        if (conn)
        {
            this->m_idle_queue.push(std::move(conn));
        }
        else
        {
            std::cerr << "[warn]ConnectionPool::init: create " << i << "th connection failed." << std::endl;
        }
    }

	return true;
}

std::unique_ptr<PooledAgent> AgentPool::getExecutor(int timeout_ms)
{
    std::unique_lock<std::mutex> lock(this->m_queue_mutex);
    std::unique_ptr<Agent> executor;

    std::chrono::steady_clock::time_point wait_start = std::chrono::steady_clock::now();
    while (true)
    {
        while (!this->m_idle_queue.empty())
        {
            executor = std::move(this->m_idle_queue.front());
            this->m_idle_queue.pop();

            if (this->validateConnection(executor.get()))
            {
                this->m_used_count++;
                return std::unique_ptr<PooledAgent>(new PooledAgent(std::move(executor), this));
            }
            else
            {
                executor.reset();
            }
        }
        if (this->m_used_count < this->m_config.max_size)
        {
            executor = this->createConnection();
            if (executor)
            {
                this->m_used_count++;
                return std::unique_ptr<PooledAgent>(new PooledAgent(std::move(executor), this));
            }
        }

        std::chrono::milliseconds wait_duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - wait_start);
        if (wait_duration.count() >= timeout_ms)
        {
            lock.unlock();
            std::cerr << "[error]AgentPool::getExecutor: get executor timeout " << timeout_ms << " ms." << std::endl;
            return nullptr;
        }

        this->m_cv.wait_for(lock, std::chrono::milliseconds(timeout_ms - wait_duration.count()), [this]()
            {
                return !this->m_idle_queue.empty();
            });
    }
}

std::unique_ptr<Agent> AgentPool::createConnection()
{
    try
    {
        std::unique_ptr<Agent> executor = std::make_unique<Agent>();
        return executor;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[error]AgentPool::createConnection: failed to create Agent: " << e.what() << std::endl;
        return nullptr;
    }
}

bool AgentPool::validateConnection(Agent* conn)
{
    if (conn == nullptr)
    {
        std::cerr << "[error]AgentPool::validateConnection: Agent is null." << std::endl;
        return false;
    }

    return true;
}

void AgentPool::returnConnection(std::unique_ptr<Agent> conn)
{
    if (!conn)
    {
        std::cerr << "[warn]AgentPool::returnConnection: trying to return null Agent." << std::endl;
        return;
    }
    std::lock_guard<std::mutex> lock(this->m_queue_mutex);

    if (this->m_is_destroyed)
    {
        std::cout << "[info]AgentPool::returnConnection: pool is destroyed, releasing Agent." << std::endl;
        return;
    }

    if (this->m_idle_queue.size() + this->m_used_count >= this->m_config.max_size)
    {
        std::cout << "[info]AgentPool::returnConnection: pool is full, releasing Agent." << std::endl;
        return;
    }

    this->m_idle_queue.push(std::move(conn));
    this->m_used_count--;

    this->m_cv.notify_one();
}

PooledAgent::PooledAgent(std::unique_ptr<Agent> conn, AgentPool* pool)
    : m_connection(std::move(conn)), m_parent_pool(pool)
{
}

void PooledAgent::releaseConnection()
{
    if (this->m_connection)
    {
        this->m_parent_pool->returnConnection(std::move(this->m_connection));
    }
}

PooledAgent::~PooledAgent()
{
    this->releaseConnection();
}
