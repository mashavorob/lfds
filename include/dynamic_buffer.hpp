/*
 * dynamic_buffer.hpp
 *
 *  Created on: Dec 15, 2014
 *      Author: masha
 */

#ifndef INCLUDE_DYNAMIC_BUFFER_HPP_
#define INCLUDE_DYNAMIC_BUFFER_HPP_

#include "buffer_base.hpp"

namespace lfds
{

template<class T, class Allocator>
class dynamic_buffer: public buffer_base<T, Allocator>
{
public:
    typedef fixed_buffer<T, Allocator> this_class;
    typedef buffer_base<T, Allocator> base_class;
    typedef typename base_class::collection_type collection_type;
    typedef typename base_class::node_type node_type;
    typedef typename base_class::size_type size_type;
public:
    static const bool fixed_size = false;

public:
    dynamic_buffer(size_type initialCapacity)
    {
        while (initialCapacity--)
        {
            try
            {
                node_type* p = base_class::allocate_nodes(1);
            } catch (...)
            {
                clear();
                throw;
            }
        }
    }
    ~dynamic_buffer()
    {
        clear();
    }

    template<class ... Args>
    node_type* new_node(Args&&... data)
    {
        node_type* p = base_class::m_freeNodes.atomic_pop();
        if (!p)
        {
            try
            {
                p = base_class::allocate_nodes(1);
            } catch (std::bad_alloc &)
            {
                // just return nullptr
            }
        }
        if (p)
        {
            base_class::construct_data(p, std::forward<Args>(data)...);
        }
        return p;
    }

    size_type capacity() const
    {
        return size_type();
    }

private:
    void clear()
    {
        node_type* p = base_class::m_freeNodes.pop();
        while (p)
        {
            base_class::deallocate_nodes(p, 1);
            p = base_class::m_freeNodes.pop();
        }
    }
};

}

#endif /* INCLUDE_DYNAMIC_BUFFER_HPP_ */
