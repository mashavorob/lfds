/*
 * fixed_buffer.hpp
 *
 *  Created on: Dec 12, 2014
 *      Author: masha
 */

#ifndef INCLUDE_FIXED_BUFFER_HPP_
#define INCLUDE_FIXED_BUFFER_HPP_

#include "buffer_base.hpp"
#include <xtomic/aux/cppbasics.hpp>

namespace xtomic
{

template<typename T, typename Allocator>
class fixed_buffer
{
public:
    typedef T value_type;
    typedef Allocator allocator_type;
    typedef fixed_buffer<value_type, allocator_type> this_class;
    typedef buffer_base<this_class> base_class;
    typedef typename base_class::node_type node_type;
    typedef typename base_class::size_type size_type;
public:
    static const bool fixed_size = true;

public:
    fixed_buffer(size_type capacity) :
            m_capacity(capacity),
            m_reserved(nullptr)
    {
        m_reserved = m_base.allocate_nodes(m_capacity);
        for (size_type i = 0; i < m_capacity; ++i)
        {
            m_base.pushFreeNode(m_reserved + i);
        }
    }

    ~fixed_buffer()
    {
        m_base.deallocate_nodes(m_reserved, m_capacity);
    }
#if XTOMIC_USE_CPP11
    template<typename ... Args>
    node_type* newNode(Args&&... data)
#else
    node_type* newNode(const value_type& data)
#endif
    {
        return m_base.newNode(std_forward(Args, data));
    }

    void freeNode(node_type* p)
    {
        m_base.freeNode(p);
    }

    size_type getCapacity() const
    {
        return m_capacity;
    }

private:

    base_class m_base;
    size_type m_capacity;
    node_type* m_reserved;
};

}

#endif /* INCLUDE_FIXED_BUFFER_HPP_ */
