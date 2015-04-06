/*
 * mt-utils.hpp
 *
 *  Created on: Mar 31, 2015
 *      Author: masha
 */

#ifndef DEMO_MT_UTILS_HPP_
#define DEMO_MT_UTILS_HPP_

namespace demo
{
namespace sync
{
class mutex
{
public:
    mutex()
    {
        pthread_mutex_init(&m_mutex, nullptr);
    }
    ~mutex()
    {
        pthread_mutex_destroy(&m_mutex);
    }

    void lock() const
    {
        pthread_mutex_lock(&m_mutex);
    }
    void unlock() const
    {
        pthread_mutex_unlock(&m_mutex);
    }
private:
    mutex(const mutex&); // = delete;
    mutex& operator=(const mutex&); // = delete;
private:
    mutable pthread_mutex_t m_mutex;
};

class guard
{
public:
    guard(const mutex & m) :
            m_mutex(m)
    {
        m_mutex.lock();
    }
    ~guard()
    {
        m_mutex.unlock();
    }
private:
    guard(const guard&); // = delete;
    guard& operator=(const guard&); // = delete;
private:
    const mutex & m_mutex;
};

}
}

#endif /* DEMO_MT_UTILS_HPP_ */
