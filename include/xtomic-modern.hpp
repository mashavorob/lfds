/*
 * modern_async.hpp
 *
 *  Created on: Mar 31, 2015
 *      Author: masha
 */

#ifndef INCLUDE_XTOMIC_MODERN_HPP_
#define INCLUDE_XTOMIC_MODERN_HPP_

namespace lfds
{

inline void thread_fence(barriers::erelease)
{
    __atomic_thread_fence(__ATOMIC_RELEASE);
}

inline void thread_fence(barriers::eacquire)
{
    __atomic_thread_fence(__ATOMIC_ACQUIRE);
}

inline void atomic_prologue()
{
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
    xtomic(T val) :
            m_val(val)
    {

    }

    void store(T val, barriers::erelaxed)
    {
        __atomic_store_n(&m_val, val, __ATOMIC_RELAXED);
    }
    void store(T val, barriers::erelaxed) volatile
    {
        __atomic_store_n(&m_val, val, __ATOMIC_RELAXED);
    }
    void store(T val, barriers::erelease)
    {
        __atomic_store_n(&m_val, val, __ATOMIC_RELEASE);
    }
    void store(T val, barriers::erelease) volatile
    {
        __atomic_store_n(&m_val, val, __ATOMIC_RELEASE);
    }

    T load(barriers::erelaxed) const
    {
        return __atomic_load_n(&m_val, __ATOMIC_RELAXED);
    }
    T load(barriers::erelaxed) const volatile
    {
        return __atomic_load_n(&m_val, __ATOMIC_RELAXED);
    }
    T load(barriers::eacquire) const
    {
        return __atomic_load_n(&m_val, __ATOMIC_ACQUIRE);
    }
    T load(barriers::eacquire) const volatile
    {
        return __atomic_load_n(&m_val, __ATOMIC_ACQUIRE);
    }

    T fetch_add(T val, barriers::erelaxed)
    {
        return __atomic_fetch_add(&m_val, val, __ATOMIC_RELAXED);
    }
    T fetch_add(T val, barriers::erelaxed) volatile
    {
        return __atomic_fetch_add(&m_val, val, __ATOMIC_RELAXED);
    }
    T fetch_add(T val, barriers::erelease)
    {
        return __atomic_fetch_add(&m_val, val, __ATOMIC_RELEASE);
    }
    T fetch_add(T val, barriers::erelease) volatile
    {
        return __atomic_fetch_add(&m_val, val, __ATOMIC_RELEASE);
    }

    T fetch_sub(T val, barriers::erelaxed)
    {
        return __atomic_fetch_sub(&m_val, val, __ATOMIC_RELAXED);
    }
    T fetch_sub(T val, barriers::erelaxed) volatile
    {
        return __atomic_fetch_sub(&m_val, val, __ATOMIC_RELAXED);
    }
    T fetch_sub(T val, barriers::erelease)
    {
        return __atomic_fetch_sub(&m_val, val, __ATOMIC_RELEASE);
    }
    T fetch_sub(T val, barriers::erelease) volatile
    {
        return __atomic_fetch_sub(&m_val, val, __ATOMIC_RELEASE);
    }

    bool atomic_cas(const T e, const T n)
    {
        return __sync_bool_compare_and_swap(&m_val, e, n);
    }
    bool atomic_cas(const this_type & e, const this_type & n)
    {
        return cas(e.m_val, n.m_val);
    }

    // pre
    T operator++()
    {
        return __atomic_add_fetch(&m_val, static_cast<T>(1), __ATOMIC_SEQ_CST);
    }
    T operator++() volatile
    {
        return __atomic_add_fetch(&m_val, static_cast<T>(1), __ATOMIC_SEQ_CST);
    }

    T operator--()
    {
        return __atomic_sub_fetch(&m_val, static_cast<T>(1), __ATOMIC_SEQ_CST);
    }
    T operator--() volatile
    {
        return __atomic_sub_fetch(&m_val, static_cast<T>(1), __ATOMIC_SEQ_CST);
    }
    // post
    T operator++(int)
    {
        return __atomic_fetch_add(&m_val, static_cast<T>(1), __ATOMIC_SEQ_CST);
    }
    T operator++(int) volatile
    {
        return __atomic_fetch_add(&m_val, static_cast<T>(1), __ATOMIC_SEQ_CST);
    }

    T operator--(int)
    {
        return __atomic_fetch_sub(&m_val, static_cast<T>(1), __ATOMIC_SEQ_CST);
    }
    T operator--(int) volatile
    {
        return __atomic_fetch_sub(&m_val, static_cast<T>(1), __ATOMIC_SEQ_CST);
    }
private:
    xtomic(const this_type&) = delete;
    this_type& operator=(const this_type&) = delete;
private:
    volatile T m_val;
};

}

#endif /* INCLUDE_XTOMIC_MODERN_HPP_ */
