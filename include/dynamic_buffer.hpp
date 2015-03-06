/*
 * dynamic_buffer.hpp
 *
 *  Created on: Dec 15, 2014
 *      Author: masha
 */

#ifndef INCLUDE_DYNAMIC_BUFFER_HPP_
#define INCLUDE_DYNAMIC_BUFFER_HPP_

#include "buffer_base.hpp"
#include "pool_buffer.hpp"
#include "stack_base_aba.hpp"

namespace lfds
{

template<class T, class Allocator>
class dynamic_buffer
{
public:
    typedef fixed_buffer<T, Allocator> this_class;
    typedef stack_base_aba<T> collection_type;
    typedef typename collection_type::node_type node_type;
    typedef Allocator allocator_type;
    typedef typename allocator_type::size_type size_type;
public:
    static const bool fixed_size = false;

public:
    dynamic_buffer(size_type initialCapacity) : m_pool(initialCapacity)
    {
    }
    ~dynamic_buffer()
    {
    }

    template<class ... Args>
    node_type* new_node(Args&&... data)
    {
        node_type* p = m_pool.allocate();
        m_base.construct_data(p, std::forward<Args>(data)...);
        return p;
    }
    void free_node(node_type* p)
    {
        m_base.destroy_data(p);
        m_pool.deallocate(p);
    }
    size_type capacity() const
    {
        return size_type();
    }

private:
    typedef buffer_base<this_class> base_class;
    typedef pool_buffer<node_type, Allocator> pool_buffer_type;

    base_class m_base;
    pool_buffer_type m_pool;
};

}

#endif /* INCLUDE_DYNAMIC_BUFFER_HPP_ */
