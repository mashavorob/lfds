/*
 * testsync.hpp
 *
 *  Created on: Mar 26, 2015
 *      Author: masha
 */

#ifndef PERFTEST_TESTSYNC_HPP_
#define PERFTEST_TESTSYNC_HPP_

#include <pthread.h>

namespace lfds
{
namespace perftest
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
}

#endif /* PERFTEST_TESTSYNC_HPP_ */
