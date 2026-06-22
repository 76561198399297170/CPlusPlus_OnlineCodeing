#ifndef _POOL_H_
#define _POOL_H_

#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <stdexcept>
#include <string>
#include <iostream>

template<typename T>
class BasePool
{
public:
    struct BasePoolConfig
    {
        int min_size = 5;
        int max_size = 20;
        int timeout_ms = 1000;
    };

    virtual ~BasePool()
    {
        std::lock_guard<std::mutex> lock(m_queue_mutex);
        m_is_destroyed = true;
        m_cv.notify_all();

        while (!m_idle_queue.empty())
        {
            m_idle_queue.pop();
        }
        std::cout << "[info]BasePool::~BasePool: all idle resources released." << std::endl;
    }

    virtual bool init(const BasePoolConfig& config) = 0;

    std::unique_ptr<T> getResource(int timeout_ms = -1)
    {
        if (timeout_ms < 0)
        {
            timeout_ms = m_config.timeout_ms;
        }

        std::unique_lock<std::mutex> lock(m_queue_mutex);
        auto wait_start = std::chrono::steady_clock::now();
        std::unique_ptr<T> resource;

        while (true)
        {

            if (m_is_destroyed)
            {
                throw std::runtime_error("BasePool::getResource: pool has been destroyed");
            }

            while (!m_idle_queue.empty())
            {
                resource = std::move(m_idle_queue.front());
                m_idle_queue.pop();

                if (validateResource(resource.get()))
                {
                    m_used_count++;
                    return resource;
                }
                else
                {
                    std::cerr << "[warn]BasePool::getResource: invalid idle resource, discard." << std::endl;
                    resource.reset();
                }
            }

            if (m_used_count < m_config.max_size)
            {
                resource = createResource();
                if (resource)
                {
                    m_used_count++;
                    return resource;
                }
                else
                {
                    std::cerr << "[warn]BasePool::getResource: create new resource failed." << std::endl;
                }
            }

            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - wait_start);
            int remaining_ms = timeout_ms - static_cast<int>(elapsed.count());

            if (remaining_ms <= 0)
            {
                std::string err_msg = "BasePool::getResource: timeout (" + std::to_string(timeout_ms) + "ms)";
                throw std::runtime_error(err_msg);
            }

            if (!m_cv.wait_for(lock, std::chrono::milliseconds(remaining_ms), [this]()
                {
                    return !m_idle_queue.empty() || m_is_destroyed;
                }))
            {
                std::string err_msg = "BasePool::getResource: wait timeout (" + std::to_string(remaining_ms) + "ms remaining)";
                throw std::runtime_error(err_msg);
            }
        }
    }

    void returnResource(std::unique_ptr<T> resource)
    {
        if (!resource) return;

        std::lock_guard<std::mutex> lock(m_queue_mutex);

        if (m_is_destroyed)
        {
            std::cerr << "[warn]BasePool::returnResource: pool destroyed, discard resource." << std::endl;
            return;
        }

        if (m_used_count <= 0)
        {
            std::cerr << "[warn]BasePool::returnResource: used_count=" << m_used_count << ", invalid state." << std::endl;
            return;
        }

        if (validateResource(resource.get()))
        {
            m_idle_queue.push(std::move(resource));
            m_cv.notify_one();
        }
        else
        {
            std::cerr << "[warn]BasePool::returnResource: invalid resource, discard." << std::endl;
        }

        m_used_count--;
    }

    //获取空闲资源数
    size_t getIdleCount() const { std::lock_guard<std::mutex> lock(m_queue_mutex); return m_idle_queue.size(); }

    //使用中资源数
    int getUsedCount() const { std::lock_guard<std::mutex> lock(m_queue_mutex); return m_used_count; }

protected:
    //禁止移动拷贝，仅子类可构造
    BasePool() = default;
    BasePool(const BasePool&) = delete;
    BasePool& operator=(const BasePool&) = delete;
    BasePool(BasePool&&) = delete;
    BasePool& operator=(BasePool&&) = delete;

    //创建单个资源
    virtual std::unique_ptr<T> createResource() = 0;

    //资源有效性验证
    virtual bool validateResource(T* resource) = 0;

    BasePoolConfig m_config;                        //池配置
    mutable std::mutex m_queue_mutex;               //队列互斥锁
    std::queue<std::unique_ptr<T>> m_idle_queue;    //空闲资源队列
    int m_used_count = 0;                           //使用中资源数
    bool m_is_destroyed = false;                    //池是否已销毁
    std::condition_variable m_cv;                   //等待条件变量
};

#endif // !_POOL_H_
