/*
 * ref_ptr.hpp
 *
 *  Created on: Feb 24, 2015
 *      Author: masha
 */

#ifndef INCLUDE_REF_PTR_HPP_
#define INCLUDE_REF_PTR_HPP_

#include "cas.hpp"

#include <atomic>

namespace lfds
{
template<class T>
struct __attribute__((aligned(sizeof(void*)*2))) ref_ptr
{
    typedef std::size_t size_type;
    typedef T value_type;
    typedef std::atomic<value_type*> atomic_ptr;
    typedef std::atomic<size_type> atomic_counter;

    atomic_ptr m_ptr;
    mutable atomic_counter m_refCount;

    ref_ptr() :
            m_ptr(nullptr), m_refCount(0)
    {

    }
    explicit ref_ptr(value_type* node, size_type refCount) :
            m_ptr(node), m_refCount(refCount)
    {

    }
    ref_ptr(const volatile ref_ptr & other) :
            m_ptr(other.m_ptr), m_refCount(other.m_refCount)
    {

    }
    ref_ptr & operator=(const volatile ref_ptr & other)
    {
        m_ptr.store(other.m_ptr.load(std::memory_order_relaxed),
                std::memory_order_relaxed);
        m_refCount.store(other.m_refCount.load(std::memory_order_relaxed),
                std::memory_order_relaxed);
        return *this;
    }

    bool atomic_cas(const ref_ptr & expected, const ref_ptr & val)
    {
        return lfds::atomic_cas(*this, expected, val);
    }
};

}

#endif /* INCLUDE_REF_PTR_HPP_ */
