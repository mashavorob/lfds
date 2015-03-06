/*
 * fixed_buffer.hpp
 *
 *  Created on: Dec 12, 2014
 *      Author: masha
 */

#ifndef INCLUDE_FIXED_BUFFER_HPP_
#define INCLUDE_FIXED_BUFFER_HPP_

#include "buffer_base.hpp"
#include "stack_base_aba.hpp"

namespace lfds
{

template<class T, class Allocator>
class fixed_buffer
{
public:
    typedef fixed_buffer<T, Allocator> this_class;
    typedef stack_base_aba<T> collection_type;
    typedef typename collection_type::node_type node_type;
    typedef Allocator allocator_type;
    typedef typename allocator_type::size_type size_type;
public:
    static const bool fixed_size = true;

public:
    fixed_buffer(size_type capacity) :
            m_capacity(capacity), m_reserved(nullptr)
    {
        m_reserved = m_base.allocate_nodes(m_capacity);
        for (auto i = 0; i < m_capacity; ++i)
        {
            m_freeNodes.push(m_reserved + i);
        }
    }

    ~fixed_buffer()
    {
        m_base.deallocate_nodes(m_reserved, m_capacity);
    }

    template<class ... Args>
    node_type* new_node(Args&&... data)
    {
        node_type* p = m_freeNodes.atomic_pop();
        if (p)
        {
            m_base.construct_data(p, std::forward<Args>(data)...);
        }
        return p;
    }
    void free_node(node_type* p)
    {
        m_base.destroy_data(p);
        m_freeNodes.atomic_push(p);
    }

    size_type capacity() const
    {
        return m_capacity;
    }

private:
    typedef buffer_base<this_class> base_class;

    base_class m_base;
    size_type m_capacity;
    node_type* m_reserved;
    collection_type m_freeNodes;
};

}

#endif /* INCLUDE_FIXED_BUFFER_HPP_ */
