/*
 * buffer_base.hpp
 *
 *  Created on: Dec 15, 2014
 *      Author: masha
 */

#ifndef INCLUDE_BUFFER_BASE_HPP_
#define INCLUDE_BUFFER_BASE_HPP_

#include "stack_base_aba.hpp"
#include <xtomic/aux/cppbasics.hpp>

namespace xtomic
{

template<typename Buffer>
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
    template<typename ... Args>
    void constructData(node_type* p, Args&&... data)
#else // LFDS_USE_CPP11
    void constructData(node_type* p, const value_type &data)
#endif // LFDS_USE_CPP11
    {
        m_dataAllocator.construct(p->getData(), std_forward(Args, data));
    }
#if !LFDS_USE_CPP11
    void constructData(node_type* p)
    {
        ::new (static_cast<void*>(p->getData())) value_type();
    }
#endif // LFDS_USE_CPP11
    void destroyData(node_type* p)
    {
        m_dataAllocator.destroy(p->getData());
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
    template<typename ... Args>
    node_type* newNode(Args&&... data)
#else
    node_type* newNode(const value_type &data)
#endif
    {
        node_type* p = popFreeNode();
        if (p)
        {
            constructData(p, std_forward(Args, data));
        }
        return p;
    }
#if !LFDS_USE_CPP11
    node_type* newNode()
    {
        node_type* p = popFreeNode();
        if (p)
        {
            constructData(p);
        }
        return p;
    }
#endif
    void freeNode(node_type* p)
    {
        destroyData(p);
        pushFreeNode(p);
    }

private:
    data_allocator_type m_dataAllocator;
    node_allocator_type m_nodeAllocator;
    collection_type m_freeNodes;
};

}

#endif /* INCLUDE_BUFFER_BASE_HPP_ */
