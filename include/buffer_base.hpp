/*
 * buffer_base.hpp
 *
 *  Created on: Dec 15, 2014
 *      Author: masha
 */

#ifndef INCLUDE_BUFFER_BASE_HPP_
#define INCLUDE_BUFFER_BASE_HPP_

#include "stack_base_aba.hpp"
#include "cppbasics.hpp"

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

#if LFDS_USE_CPP11
    template<class ... Args>
    void construct_data(node_type* p, Args&&... data)
#else // LFDS_USE_CPP11
    void construct_data(node_type* p, const value_type &data)
#endif // LFDS_USE_CPP11
    {
        m_dataAllocator.construct(p->data(), std_forward(Args, data));
    }
#if !LFDS_USE_CPP11
    void construct_data(node_type* p)
    {
        ::new (static_cast<void*>(p->data())) value_type();
    }
#endif // LFDS_USE_CPP11
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
#if LFDS_USE_CPP11
    template<class ... Args>
    node_type* new_node(Args&&... data)
#else
    node_type* new_node(const value_type &data)
#endif
    {
        node_type* p = popFreeNode();
        if (p)
        {
            construct_data(p, std_forward(Args, data));
        }
        return p;
    }
#if !LFDS_USE_CPP11
    node_type* new_node()
    {
        node_type* p = popFreeNode();
        if (p)
        {
            construct_data(p);
        }
        return p;
    }
#endif
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
