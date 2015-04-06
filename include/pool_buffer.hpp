/*
 * trie_buffer.hpp
 *
 *  Created on: Feb 17, 2015
 *      Author: masha
 */

#ifndef INCLUDE_POOL_BUFFER_HPP_
#define INCLUDE_POOL_BUFFER_HPP_

#include "dynamic_buffer.hpp"

namespace lfds
{

template<class T, class Allocator>
class pool_buffer
{
public:
    typedef T value_type;
    typedef Allocator allocator_type;
    typedef pool_buffer<value_type, allocator_type> this_type;

    typedef dynamic_buffer<value_type, allocator_type> buffer_type;
    typedef typename buffer_type::size_type size_type;
    typedef typename buffer_type::node_type node_type;

private:
    pool_buffer(const this_type&); // = delete;
    this_type& operator=(const this_type&); // = delete;

public:
    pool_buffer(size_type initialCapacity) : m_buffer(initialCapacity)
    {
    }
    ~pool_buffer()
    {
    }

    T* allocate()
    {
        node_type* node = m_buffer.new_node();
        value_type* p = node->data();
        return p;
    }
    void deallocate(T* p)
    {
        node_type* node = node_type::recover(p);
        m_buffer.free_node(node);
    }
private:
    buffer_type m_buffer;
};
}

#endif /* INCLUDE_POOL_BUFFER_HPP_ */
