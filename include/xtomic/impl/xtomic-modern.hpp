/*
 * modern_async.hpp
 *
 *  Created on: Mar 31, 2015
 *      Author: masha
 */

#ifndef INCLUDE_XTOMIC_MODERN_HPP_
#define INCLUDE_XTOMIC_MODERN_HPP_

namespace xtomic
{

inline void thread_fence(const barriers::erelease)
{
    __atomic_thread_fence(__ATOMIC_RELEASE);
}

inline void thread_fence(const barriers::eacquire)
{
    __atomic_thread_fence(__ATOMIC_ACQUIRE);
}

template<typename T>
inline void quantum<T>::store(const T val, barriers::erelaxed)
{
    __atomic_store_n(&m_val, val, __ATOMIC_RELAXED);
}

template<typename T>
inline void quantum<T>::store(const T val, barriers::erelaxed) volatile
{
    __atomic_store_n(&m_val, val, __ATOMIC_RELAXED);
}

template<typename T>
inline void quantum<T>::store(const T val, barriers::erelease)
{
    __atomic_store_n(&m_val, val, __ATOMIC_RELEASE);
}

template<typename T>
inline void quantum<T>::store(const T val, barriers::erelease) volatile
{
    __atomic_store_n(&m_val, val, __ATOMIC_RELEASE);
}

template<typename T>
inline T quantum<T>::load(const barriers::erelaxed) const
{
    return __atomic_load_n(&m_val, __ATOMIC_RELAXED);
}

template<typename T>
inline T quantum<T>::load(const barriers::erelaxed) const volatile
{
    return __atomic_load_n(&m_val, __ATOMIC_RELAXED);
}

template<typename T>
inline T quantum<T>::load(const barriers::eacquire) const
{
    return __atomic_load_n(&m_val, __ATOMIC_ACQUIRE);
}

template<typename T>
inline T quantum<T>::load(const barriers::eacquire) const volatile
{
    return __atomic_load_n(&m_val, __ATOMIC_ACQUIRE);
}

template<typename T>
inline T quantum<T>::fetch_add(const T val, barriers::erelaxed)
{
    return __atomic_fetch_add(&m_val, val, __ATOMIC_RELAXED);
}

template<typename T>
inline T quantum<T>::fetch_add(const T val, barriers::erelaxed) volatile
{
    return __atomic_fetch_add(&m_val, val, __ATOMIC_RELAXED);
}

template<typename T>
inline T quantum<T>::fetch_add(const T val, barriers::erelease)
{
    return __atomic_fetch_add(&m_val, val, __ATOMIC_RELEASE);
}

template<typename T>
inline T quantum<T>::fetch_add(const T val, barriers::erelease) volatile
{
    return __atomic_fetch_add(&m_val, val, __ATOMIC_RELEASE);
}

template<typename T>
inline T quantum<T>::fetch_sub(const T val, barriers::erelaxed)
{
    return __atomic_fetch_sub(&m_val, val, __ATOMIC_RELAXED);
}

template<typename T>
inline T quantum<T>::fetch_sub(const T val, barriers::erelaxed) volatile
{
    return __atomic_fetch_sub(&m_val, val, __ATOMIC_RELAXED);
}

template<typename T>
inline T quantum<T>::fetch_sub(const T val, barriers::erelease)
{
    return __atomic_fetch_sub(&m_val, val, __ATOMIC_RELEASE);
}

template<typename T>
inline T quantum<T>::fetch_sub(const T val, barriers::erelease) volatile
{
    return __atomic_fetch_sub(&m_val, val, __ATOMIC_RELEASE);
}

template<typename T>
inline bool quantum<T>::atomic_cas(const T e, const T n)
{
    return __sync_bool_compare_and_swap(&m_val, e, n);
}

template<typename T>
inline bool quantum<T>::atomic_cas(const this_type & e, const this_type & n)
{
    return cas(e.m_val, n.m_val);
}

template<typename T>
inline T quantum<T>::operator++()
{
    return __atomic_add_fetch(&m_val, static_cast<T>(1), __ATOMIC_SEQ_CST);
}

template<typename T>
inline T quantum<T>::operator++() volatile
{
    return __atomic_add_fetch(&m_val, static_cast<T>(1), __ATOMIC_SEQ_CST);
}

template<typename T>
inline T quantum<T>::operator--()
{
    return __atomic_sub_fetch(&m_val, static_cast<T>(1), __ATOMIC_SEQ_CST);
}

template<typename T>
inline T quantum<T>::operator--() volatile
{
    return __atomic_sub_fetch(&m_val, static_cast<T>(1), __ATOMIC_SEQ_CST);
}

template<typename T>
inline T quantum<T>::operator++(int)
{
    return __atomic_fetch_add(&m_val, static_cast<T>(1), __ATOMIC_SEQ_CST);
}

template<typename T>
inline T quantum<T>::operator++(int) volatile
{
    return __atomic_fetch_add(&m_val, static_cast<T>(1), __ATOMIC_SEQ_CST);
}

template<typename T>
inline T quantum<T>::operator--(int)
{
    return __atomic_fetch_sub(&m_val, static_cast<T>(1), __ATOMIC_SEQ_CST);
}

template<typename T>
inline T quantum<T>::operator--(int) volatile
{
    return __atomic_fetch_sub(&m_val, static_cast<T>(1), __ATOMIC_SEQ_CST);
}
};

}

#endif /* INCLUDE_XTOMIC_MODERN_HPP_ */
