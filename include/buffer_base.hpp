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

template<class Buffer>
class buffer_base
{
public:
    typedef buffer_base<Buffer> this_class;
    typedef Buffer buffer_type;
    typedef typename buffer_type::value_type value_type;
    typedef stack_base_aba<value_type> collection_type;
    typedef typename collection_type::node_type node_type;
    typedef typename buffer_type::allocator_type allocator_type;
    typedef typename allocator_type::template rebind<value_type>::other data_allocator_type;
    typedef typename data_allocator_type::size_type size_type;
    typedef typename allocator_type::template rebind<node_type>::other node_allocator_type;

    // non copyable
private:
    buffer_base(const this_class &);
    this_class& operator=(const this_class&);

public:
    buffer_base()
    {
    }

public:
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
    void pushFreeNode(node_type* p)
    {
        m_freeNodes.atomic_push(p);
    }
    node_type* popFreeNode()
    {
        return m_freeNodes.atomic_pop();
    }
    template<class ... Args>
    node_type* new_node(Args&&... data)
    {
        node_type* p = popFreeNode();
        if (p)
        {
            construct_data(p, std::forward<Args>(data)...);
        }
        return p;
    }
    void free_node(node_type* p)
    {
        destroy_data(p);
        pushFreeNode(p);
    }


private:
    data_allocator_type m_dataAllocator;
    node_allocator_type m_nodeAllocator;
    collection_type m_freeNodes;
};

}

#endif /* INCLUDE_BUFFER_BASE_HPP_ */
