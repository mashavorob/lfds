/*
 * ref_ptr.hpp
 *
 *  Created on: Feb 24, 2015
 *      Author: masha
 */

#ifndef INCLUDE_REF_PTR_HPP_
#define INCLUDE_REF_PTR_HPP_

#include "cas.hpp"
#include <xtomic/xtomic.hpp>

#include <xtomic/aux/cppbasics.hpp>

#include <cstddef>

namespace lfds
{
template<typename T>
struct align_4_cas16 ref_ptr
{
    typedef ref_ptr<T> this_type;
    typedef std::size_t size_type;
    typedef T value_type;
    typedef xtomic<value_type*> atomic_ptr_type;
    typedef xtomic<size_type> atomic_counter;

    atomic_ptr_type m_ptr;
    mutable atomic_counter m_refCount;

    ref_ptr() :
            m_ptr(nullptr),
            m_refCount(0)
    {

    }
    ref_ptr(value_type * const node, const size_type refCount) :
            m_ptr(node),
            m_refCount(refCount)
    {

    }
    ref_ptr(const volatile ref_ptr & other) :
            m_ptr(other.m_ptr),
            m_refCount(other.m_refCount)
    {

    }
    ref_ptr & operator=(const volatile ref_ptr & other)
    {
        m_ptr.store(other.m_ptr.load(barriers::relaxed), barriers::relaxed);
        m_refCount.store(other.m_refCount.load(barriers::relaxed),
                barriers::relaxed);
        return *this;
    }

    bool atomic_cas(const this_type & expected, const this_type & val)
    {
        return lfds::atomic_cas(*this, expected, val);
    }
    size_type addRef() const
    {
        return ++m_refCount;
    }
    size_type release() const
    {
        return --m_refCount;
    }
};

}

#endif /* INCLUDE_REF_PTR_HPP_ */
