#include "runtime/code_pool.h"

bool CodePool::init(const CodePoolConfig& pool_config)
{

    if (pool_config.min_size < 0 || pool_config.max_size < pool_config.min_size)
    {
        std::cerr << "[error]CodePool::init: invalid pool size." << std::endl;
        return false;
    }

    this->m_config = pool_config;
    this->BasePool<CodeExecutor>::m_config.min_size = pool_config.min_size;
    this->BasePool<CodeExecutor>::m_config.max_size = pool_config.max_size;
    this->BasePool<CodeExecutor>::m_config.timeout_ms = pool_config.timeout_ms;

    std::filesystem::create_directories(this->m_config.temp_dir);
    
    std::lock_guard<std::mutex> lock(this->m_queue_mutex);
    for (int i = 0; i < this->m_config.min_size; i++)
    {
        std::unique_ptr<CodeExecutor> conn = createConnection();
        if (conn)
        {
            this->m_idle_queue.push(std::move(conn));
        }
        else
        {
            std::cerr << "[error]CodePool::init: create CodeExecutor failed." << std::endl;
            return false;
        }
    }

    std::cout << "[info]CodePool::init: code pool initialized successfully with " << this->m_config.min_size << " executors." << std::endl;
    return true;
}

std::unique_ptr<PooledCode> CodePool::getExecutor(int timeout_ms)
{
    std::unique_lock<std::mutex> lock(this->m_queue_mutex);
    std::unique_ptr<CodeExecutor> executor;

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
                return std::unique_ptr<PooledCode>(new PooledCode(std::move(executor), this));
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
                return std::unique_ptr<PooledCode>(new PooledCode(std::move(executor), this));
            }
        }

        std::chrono::milliseconds wait_duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - wait_start);
        if (wait_duration.count() >= timeout_ms)
        {
            lock.unlock();
            std::cerr << "[error]CodePool::getExecutor: get executor timeout " << timeout_ms << " ms." << std::endl;
            return nullptr;
        }

        this->m_cv.wait_for(lock, std::chrono::milliseconds(timeout_ms - wait_duration.count()), [this]()
            {
                return !this->m_idle_queue.empty();
            });
    }
}

std::unique_ptr<CodeExecutor> CodePool::createConnection()
{
    try
    {
        std::unique_ptr<CodeExecutor> executor = std::make_unique<CodeExecutor>();
        executor->setTempDir(this->m_config.temp_dir);
        executor->setCompileTimeLimit(this->m_config.compile_limit_ms);
        return executor;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[error]CodePool::createConnection: failed to create CodeExecutor: " << e.what() << std::endl;
        return nullptr;
    }
}

bool CodePool::validateConnection(CodeExecutor* conn)
{
    if (conn == nullptr)
    {
        std::cerr << "[error]CodePool::validateConnection: CodeExecutor is null." << std::endl;
        return false;
    }

    std::cout << "[info]CodePool::validateConnection: CodeExecutor is valid." << std::endl;
    return true;
}

void CodePool::returnConnection(std::unique_ptr<CodeExecutor> conn)
{
    if (!conn)
    {
        std::cerr << "[warn]CodePool::returnConnection: trying to return null CodeExecutor." << std::endl;
        return;
    }
    std::lock_guard<std::mutex> lock(this->m_queue_mutex);

    if (this->m_is_destroyed)
    {
        std::cout << "[info]CodePool::returnConnection: pool is destroyed, releasing CodeExecutor." << std::endl;
        return;
    }

    if (this->m_idle_queue.size() + this->m_used_count >= this->m_config.max_size)
    {
        std::cout << "[info]CodePool::returnConnection: pool is full, releasing CodeExecutor." << std::endl;
        return;
    }

    this->m_idle_queue.push(std::move(conn));
    this->m_used_count--;

    this->m_cv.notify_one();

    std::cout << "[info]CodePool::returnConnection: CodeExecutor returned to pool." << std::endl;
}

void PooledCode::releaseConnection()
{
    if (this->m_connection)
    {
        this->m_parent_pool->returnConnection(std::move(this->m_connection));
    }
}

PooledCode::PooledCode(std::unique_ptr<CodeExecutor> conn, CodePool* pool)
    : m_connection(std::move(conn)), m_parent_pool(pool)
{
}

PooledCode::~PooledCode()
{
    this->releaseConnection();
}

