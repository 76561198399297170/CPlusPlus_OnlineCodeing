#ifndef _SINGLETON_H_
#define _SINGLETON_H_

#include <mutex>

template<typename T>
class Singleton
{
public:
    //禁用拷贝构造
    Singleton(const Singleton&) = delete;
    //禁用赋值运算符
    Singleton& operator=(const Singleton&) = delete;
    //获取单例实例类
    static std::shared_ptr<T> getInstance()
    {
        std::call_once(initFlag, &Singleton<T>::init);
        return instance;
    }

protected:
    Singleton() {}

private:
    static void init()
    {
        instance = std::shared_ptr<T>(new T());
    }

    static std::shared_ptr<T> instance;
    static std::once_flag initFlag;
};

template<typename T>
std::shared_ptr<T> Singleton<T>::instance = nullptr;

template<typename T>
std::once_flag Singleton<T>::initFlag;

#endif // !_SINGLETON_H_
