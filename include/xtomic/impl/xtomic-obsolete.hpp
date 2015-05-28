/*
 * old_async.hpp
 *
 *  Created on: Mar 31, 2015
 *      Author: masha
 */

#ifndef INCLUDE_XTOMIC_OBSOLETE_HPP_
#define INCLUDE_XTOMIC_OBSOLETE_HPP_

#include "cas.hpp"

namespace xtomic
{

inline void thread_fence(const barriers::erelease)
{
    asm volatile("" : : : "memory");
}

inline void thread_fence(const barriers::eacquire)
{
    asm volatile("" : : : "memory");
}

template<typename T>
inline void quantum<T>::store(const T val, barriers::erelaxed)
{
    m_val = val;
}

template<typename T>
inline void quantum<T>::store(const T val, barriers::erelaxed) volatile
{
    m_val = val;
}

template<typename T>
inline void quantum<T>::store(const T val, barriers::erelease)
{
    m_val = val;
    thread_fence(barriers::release);
}

template<typename T>
inline void quantum<T>::store(const T val, barriers::erelease) volatile
{
    m_val = val;
    thread_fence(barriers::release);
}

template<typename T>
inline T quantum<T>::load(const barriers::erelaxed) const
{
    return m_val;
}

template<typename T>
inline T quantum<T>::load(const barriers::erelaxed) const volatile
{
    return m_val;
}

template<typename T>
inline T quantum<T>::load(const barriers::eacquire) const
{
    thread_fence(barriers::acquire);
    return m_val;
}

template<typename T>
inline T quantum<T>::load(const barriers::eacquire) const volatile
{
    thread_fence(barriers::acquire);
    return m_val;
}

template<typename T>
inline T quantum<T>::fetch_add(const T add, barriers::erelaxed)
{
    T res = m_val;
    m_val += add;
    return res;
}

template<typename T>
inline T quantum<T>::fetch_add(const T add, barriers::erelaxed) volatile
{
    T res = m_val;
    m_val += add;
    return res;
}

template<typename T>
inline T quantum<T>::fetch_add(const T add, barriers::erelease)
{
    return __sync_fetch_and_add(&m_val, add);
}

template<typename T>
inline T quantum<T>::fetch_add(const T add, barriers::erelease) volatile
{
    return __sync_fetch_and_add(&m_val, add);
}

template<typename T>
inline T quantum<T>::fetch_sub(const T add, barriers::erelaxed)
{
    T res = m_val;
    m_val -= add;
    return res;
}

template<typename T>
inline T quantum<T>::fetch_sub(const T add, barriers::erelaxed) volatile
{
    T res = m_val;
    m_val -= add;
    return res;
}

template<typename T>
inline T quantum<T>::fetch_sub(const T add, barriers::erelease)
{
    return __sync_fetch_and_sub(&m_val, add);
}

template<typename T>
inline T quantum<T>::fetch_sub(const T add, barriers::erelease) volatile
{
    return __sync_fetch_and_sub(&m_val, add);
}

template<typename T>
inline bool quantum<T>::atomic_cas(const T e, const T n)
{
    return xtomic::atomic_cas(m_val, e, n);
}

template<typename T>
inline bool quantum<T>::atomic_cas(const this_type & e, const this_type & n)
{
    return atomic_cas(e.m_val, n.m_val);
}

// pre
template<typename T>
inline T quantum<T>::operator++()
{
    return __sync_add_and_fetch(&m_val, static_cast<T>(1));
}

template<typename T>
inline T quantum<T>::operator++() volatile
{
    return __sync_add_and_fetch(&m_val, static_cast<T>(1));
}

template<typename T>
inline T quantum<T>::operator--()
{
    return __sync_sub_and_fetch(&m_val, static_cast<T>(1));
}

template<typename T>
inline T quantum<T>::operator--() volatile
{
    return __sync_sub_and_fetch(&m_val, static_cast<T>(1));
}

// post
template<typename T>
inline T quantum<T>::operator++(int)
{
    return __sync_fetch_and_add(&m_val, static_cast<T>(1));
}

template<typename T>
inline T quantum<T>::operator++(int) volatile
{
    return __sync_fetch_and_add(&m_val, static_cast<T>(1));
}

template<typename T>
inline T quantum<T>::operator--(int)
{
    return __sync_fetch_and_sub(&m_val, static_cast<T>(1));
}

template<typename T>
inline T quantum<T>::operator--(int) volatile
{
    return __sync_fetch_and_sub(&m_val, static_cast<T>(1));
}

}

#endif /* INCLUDE_XTOMIC_OBSOLETE_HPP_ */
