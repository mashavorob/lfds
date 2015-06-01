/*
 * dynamic_buffer.hpp
 *
 *  Created on: Dec 15, 2014
 *      Author: masha
 */

#ifndef INCLUDE_DYNAMIC_BUFFER_HPP_
#define INCLUDE_DYNAMIC_BUFFER_HPP_

#include "buffer_base.hpp"
#include <xtomic/aux/cppbasics.hpp>

namespace xtomic
{

template<typename T, typename Allocator>
class dynamic_buffer
{
public:
    typedef T value_type;
    typedef Allocator allocator_type;
    typedef dynamic_buffer<value_type, allocator_type> this_class;
    typedef buffer_base<this_class> base_class;
    typedef typename base_class::node_type node_type;
    typedef typename base_class::size_type size_type;

private:
    typedef typename Allocator::template rebind<node_type>::other node_allocator_type;

public:
    static const bool fixed_size = false;

public:
    dynamic_buffer(size_type initialCapacity)
    {
        for (size_type i = 0; i < initialCapacity; ++i)
        {
            node_type* node = m_base.allocate_nodes(1);
            m_base.pushFreeNode(node);
        }
    }
    ~dynamic_buffer()
    {
        node_type* node = m_base.popFreeNode();
        while (node)
        {
            m_base.deallocate_nodes(node, 1);
            node = m_base.popFreeNode();
        }
    }

#if XTOMIC_USE_CPP11
    template<typename ... Args>
    node_type* newNode(Args&&... data)
#else
    node_type* newNode(const value_type &data)
#endif
    {
        node_type* node = m_base.popFreeNode();
        if (!node)
        {
            node = m_base.allocate_nodes(1);
        }
        m_base.constructData(node, std_forward(Args, data));
        return node;
    }
#if !XTOMIC_USE_CPP11
    node_type* newNode()
    {
        node_type* node = m_base.popFreeNode();
        if (!node)
        {
            node = m_base.allocate_nodes(1);
        }
        m_base.constructData(node);
        return node;
    }
#endif
    void freeNode(node_type* p)
    {
        m_base.freeNode(p);
    }
    size_type getCapacity() const
    {
        return size_type();
    }

private:
    base_class m_base;
};

}

#endif /* INCLUDE_DYNAMIC_BUFFER_HPP_ */
