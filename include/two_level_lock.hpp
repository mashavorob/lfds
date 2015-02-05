/*
 * two_level_lock.hpp
 *
 *  Created on: Jan 31, 2015
 *      Author: masha
 */

#ifndef INCLUDE_TWO_LEVEL_LOCK_HPP_
#define INCLUDE_TWO_LEVEL_LOCK_HPP_

#include <atomic>

namespace lfds
{
namespace
{
class two_level_lock
{
public:
    two_level_lock() :
            m_acc_count(0), m_del_count(0)
    {
    }

    // weak lock, allows access from multiple clients
    void lock_weak()
    {
        bool success = false;
        do
        {
            if (m_del_count.load(std::memory_order_relaxed) == 0)
            {
                m_acc_count.fetch_add(1, std::memory_order_acquire);
                if (m_del_count.load(std::memory_order_relaxed) == 0)
                {
                    success = true;
                }
                else
                {
                    m_acc_count.fetch_sub(1, std::memory_order_relaxed);
                }
            }
        } while (!success);
    }

    // strong lock, allows just exclusive access
    void unlock_weak()
    {
        m_acc_count.fetch_sub(1, std::memory_order_relaxed);
    }

    void lock_exclusive()
    {
        int count = m_del_count.fetch_add(1, std::memory_order_relaxed);

        while (count)
        {
            m_del_count.fetch_sub(1, std::memory_order_relaxed);
            count = m_del_count.fetch_add(1, std::memory_order_relaxed);
        }
        while (m_acc_count.load(std::memory_order_relaxed))
            ;
    }

    void unlock_exclusive()
    {
        m_del_count.fetch_sub(1, std::memory_order_relaxed);
    }

private:
    std::atomic<int> m_acc_count;
    std::atomic<int> m_del_count;
};

template<class Lock>
class scoped_guard
{
private:
    typedef scoped_guard<Lock> this_class;
private:
    scoped_guard(const this_class&);
    this_class& operator=(const this_class&);
public:
    scoped_guard(this_class&& other) :
            m_lock(other.m_lock), m_locked(other.m_locked)
    {
        other.m_locked = false;
    }
    scoped_guard(Lock & lock) :
            m_lock(lock), m_locked(false)
    {

    }
    ~scoped_guard()
    {
        unlock();
    }
    void lock()
    {
        if (!m_locked)
        {
            m_lock.lock();
            m_locked = true;
        }
    }
    void unlock()
    {
        if (m_locked)
        {
            m_lock.unlock();
            m_locked = true;
        }
    }
private:
    Lock & m_lock;
    bool m_locked;
};

class weak_lock: private two_level_lock
{
public:
    typedef scoped_guard<weak_lock> guard_type;

    void lock()
    {
        two_level_lock::lock_weak();
    }
    void unlock()
    {
        two_level_lock::unlock_weak();
    }

    static guard_type create(two_level_lock& lock)
    {
        return guard_type(reinterpret_cast<weak_lock&>(lock));
    }
};

class exclusive_lock: private two_level_lock
{
public:
    typedef scoped_guard<exclusive_lock> guard_type;

    void lock()
    {
        two_level_lock::lock_exclusive();
    }
    void unlock()
    {
        two_level_lock::unlock_exclusive();
    }
    static guard_type create(two_level_lock& lock)
    {
        return guard_type(reinterpret_cast<exclusive_lock&>(lock));
    }
};

}
}

#endif /* INCLUDE_TWO_LEVEL_LOCK_HPP_ */
