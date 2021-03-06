/*
 * sync.hpp
 *
 *  Created on: Apr 1, 2015
 *      Author: masha
 */

#ifndef UTILS_UTILS_SYNC_HPP_
#define UTILS_UTILS_SYNC_HPP_

#include <xtomic/cppbasics.hpp>

#include <pthread.h>

namespace xtomic {
//
// All content of 'my' namespace can be used only inside xtomic library
// do not try to reuse it outside of xtomic
//
namespace my {
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



#endif /* UTILS_UTILS_SYNC_HPP_ */
