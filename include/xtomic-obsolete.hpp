/*
 * old_async.hpp
 *
 *  Created on: Mar 31, 2015
 *      Author: masha
 */

#ifndef INCLUDE_XTOMIC_OBSOLETE_HPP_
#define INCLUDE_XTOMIC_OBSOLETE_HPP_

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


template<class T>
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
        return __sync_fetch_and_add(&m_val, add);
    }
    T fetch_add(T add, barriers::erelease) volatile
    {
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
        return __sync_fetch_and_sub(&m_val, add);
    }
    T fetch_sub(T add, barriers::erelease) volatile
    {
        return __sync_fetch_and_sub(&m_val, add);
    }

    bool cas(const T e, const T n)
    {
        return __sync_bool_compare_and_swap(&m_val, e, n);
    }
    bool cas(const this_type & e, const this_type & n)
    {
        return cas(e.m_val, n.m_val);
    }

    // pre
    T operator++()
    {
        return __sync_add_and_fetch(&m_val, static_cast<T>(1));
    }
    T operator++() volatile
    {
        return __sync_add_and_fetch(&m_val, static_cast<T>(1));
    }
    T operator--()
    {
        return __sync_sub_and_fetch(&m_val, static_cast<T>(1));
    }
    T operator--() volatile
    {
        return __sync_sub_and_fetch(&m_val, static_cast<T>(1));
    }
    // post
    T operator++(int)
    {
        return __sync_fetch_and_add(&m_val, static_cast<T>(1));
    }
    T operator++(int) volatile
    {
        return __sync_fetch_and_add(&m_val, static_cast<T>(1));
    }
    T operator--(int)
    {
        return __sync_fetch_and_sub(&m_val, static_cast<T>(1));
    }
    T operator--(int) volatile
    {
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