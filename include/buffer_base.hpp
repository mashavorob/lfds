/*
 * buffer_base.hpp
 *
 *  Created on: Dec 15, 2014
 *      Author: masha
 */

#ifndef INCLUDE_BUFFER_BASE_HPP_
#define INCLUDE_BUFFER_BASE_HPP_

#include "stack_base_aba.hpp"

namespace lfds
{

template<class T, class Allocator>
class buffer_base
{
public:
    typedef buffer_base<T, Allocator> this_class;
    typedef stack_base_aba<T> collection_type;
    typedef typename collection_type::node_type node_type;
    typedef Allocator data_allocator_type;
    typedef typename Allocator::template rebind<node_type>::other node_allocator_type;
    typedef typename node_allocator_type::size_type size_type;

    // non copyable
private:
    buffer_base(const this_class &);
    this_class& operator=(const this_class&);

public:
    buffer_base()
    {
    }

protected:
    node_type* allocate_nodes(size_type count)
    {
        return m_nodeAllocator.allocate(count);
    }
    void deallocate_nodes(node_type* p, size_type count)
    {
        m_nodeAllocator.deallocate(p, count);
    }

    template<class ... Args>
    void construct_data(node_type* p, Args&&... data)
    {
        m_dataAllocator.construct(p->data(), std::forward<Args>(data)...);
    }
    void destroy_data(node_type* p)
    {
        m_dataAllocator.destroy(p->data());
    }

public:
    void free_node(node_type* p)
    {
        destroy_data(p);
        m_freeNodes.atomic_push(p);
    }

private:
    data_allocator_type m_dataAllocator;
    node_allocator_type m_nodeAllocator;
protected:
    collection_type m_freeNodes;
};

}

#endif /* INCLUDE_BUFFER_BASE_HPP_ */
