/*
 * old_async.hpp
 *
 *  Created on: Mar 31, 2015
 *      Author: masha
 */

#ifndef INCLUDE_XTOMIC_OBSOLETE_HPP_
#define INCLUDE_XTOMIC_OBSOLETE_HPP_

#include "cas.hpp"

namespace lfds
{

inline void thread_fence(barriers::erelease)
{
    asm volatile("" : : : "memory");
}

inline void thread_fence(barriers::eacquire)
{
    asm volatile("" : : : "memory");
}

inline void atomic_prologue()
{
    thread_fence(barriers::acquire);
}

template<typename T>
class xtomic
{
public:
    typedef xtomic<T> this_type;

public:
    xtomic() :
            m_val()
    {

    }
    explicit xtomic(T val) :
            m_val(val)
    {

    }

    void store(T val, barriers::erelaxed)
    {
        m_val = val;
    }
    void store(T val, barriers::erelaxed) volatile
    {
        m_val = val;
    }
    void store(T val, barriers::erelease)
    {
        m_val = val;
        thread_fence(barriers::release);
    }
    void store(T val, barriers::erelease) volatile
    {
        m_val = val;
        thread_fence(barriers::release);
    }

    T load(barriers::erelaxed) const
    {
        return m_val;
    }
    T load(barriers::erelaxed) const volatile
    {
        return m_val;
    }
    T load(barriers::eacquire) const
    {
        thread_fence(barriers::acquire);
        return m_val;
    }
    T load(barriers::eacquire) const volatile
    {
        thread_fence(barriers::acquire);
        return m_val;
    }

    T fetch_add(T add, barriers::erelaxed)
    {
        T res = m_val;
        m_val += add;
        return res;
    }
    T fetch_add(T add, barriers::erelaxed) volatile
    {
        T res = m_val;
        m_val += add;
        return res;
    }

    T fetch_add(T add, barriers::erelease)
    {
        atomic_prologue();
        return __sync_fetch_and_add(&m_val, add);
    }
    T fetch_add(T add, barriers::erelease) volatile
    {
        atomic_prologue();
        return __sync_fetch_and_add(&m_val, add);
    }

    T fetch_sub(T add, barriers::erelaxed)
    {
        T res = m_val;
        m_val -= add;
        return res;
    }
    T fetch_sub(T add, barriers::erelaxed) volatile
    {
        T res = m_val;
        m_val -= add;
        return res;
    }

    T fetch_sub(T add, barriers::erelease)
    {
        atomic_prologue();
        return __sync_fetch_and_sub(&m_val, add);
    }
    T fetch_sub(T add, barriers::erelease) volatile
    {
        atomic_prologue();
        return __sync_fetch_and_sub(&m_val, add);
    }

    bool atomic_cas(const T e, const T n)
    {
        return lfds::atomic_cas(m_val, e, n);
    }
    bool atomic_cas(const this_type & e, const this_type & n)
    {
        return atomic_cas(e.m_val, n.m_val);
    }

    // pre
    T operator++()
    {
        atomic_prologue();
        return __sync_add_and_fetch(&m_val, static_cast<T>(1));
    }
    T operator++() volatile
    {
        atomic_prologue();
        return __sync_add_and_fetch(&m_val, static_cast<T>(1));
    }
    T operator--()
    {
        atomic_prologue();
        return __sync_sub_and_fetch(&m_val, static_cast<T>(1));
    }
    T operator--() volatile
    {
        atomic_prologue();
        return __sync_sub_and_fetch(&m_val, static_cast<T>(1));
    }
    // post
    T operator++(int)
    {
        atomic_prologue();
        return __sync_fetch_and_add(&m_val, static_cast<T>(1));
    }
    T operator++(int) volatile
    {
        atomic_prologue();
        return __sync_fetch_and_add(&m_val, static_cast<T>(1));
    }
    T operator--(int)
    {
        atomic_prologue();
        return __sync_fetch_and_sub(&m_val, static_cast<T>(1));
    }
    T operator--(int) volatile
    {
        atomic_prologue();
        return __sync_fetch_and_sub(&m_val, static_cast<T>(1));
    }
private:
    xtomic(const this_type&); // = delete;
    this_type& operator=(const this_type&); // = delete;

private:
    volatile T m_val;
};

}

#endif /* INCLUDE_XTOMIC_OBSOLETE_HPP_ */
