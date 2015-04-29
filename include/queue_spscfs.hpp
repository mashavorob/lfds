/*
 * queue_scsp.hpp
 *
 *  Created on: Dec 25, 2014
 *      Author: masha
 */

#ifndef INCLUDE_QUEUE_SPSCFS_HPP_
#define INCLUDE_QUEUE_SPSCFS_HPP_

#include "xtomic.hpp"
#include "cppbasics.hpp"

#include <memory>
#include <utility>

namespace lfds
{

namespace
{
template<typename T>
class BufferNode
{
public:
    typedef BufferNode<T> this_class;
private:
    BufferNode(const this_class&);
    this_class& operator=(const this_class&);
public:
    BufferNode()
    {
    }
    T* getData()
    {
        return reinterpret_cast<T*>(m_data);
    }
private:
    char m_data[sizeof(T)];
};
}

template<typename T, typename Allocator = std::allocator<T> >
class queue_spscfs
{
public:
    static const bool many_producers = false;
    static const bool many_consumers = false;
    static const bool fixed_size = true;
    static const bool wait_free = true;

public:
    typedef T value_type;
    typedef std::size_t size_type;
private:
    typedef queue_spscfs<T, Allocator> this_class;
    typedef BufferNode<T> node_type;

    typedef Allocator data_allocator_type;
    typedef typename Allocator::template rebind<node_type>::other node_allocator_type;

private:
    queue_spscfs(const this_class&);
    this_class& operator=(const this_class&);

public:
    queue_spscfs(std::size_t capacity) :
            m_capacity(capacity),
            m_size(0),
            m_head(0),
            m_tail(0)
    {
        m_buffer = m_nodeAllocator.allocate(capacity);
    }

    ~queue_spscfs()
    {
        node_type* p = raw_pop();
        while (p)
        {
            m_dataAllocator.destroy(p->getData());
            --m_size;
            p = raw_pop();
        }
        m_nodeAllocator.deallocate(m_buffer, m_capacity);
    }

#if LFDS_USE_CPP11
    template<typename ... Args>
    bool push(Args&&... data)
#else
    bool push(const value_type& data)
#endif
    {
        node_type* p = raw_push();
        if (p)
        {
            m_dataAllocator.construct(p->getData(), std_forward(Args, data));
            ++m_size;
            return true;
        }
        return false;
    }
    bool pop(T & val)
    {
        node_type* p = raw_pop();
        if (p)
        {
            val = std_move(*p->getData());
            m_dataAllocator.destroy(p->getData());
            --m_size;
            return true;
        }
        return false;
    }
    size_type size() const
    {
        return m_size.load(barriers::relaxed);
    }
private:
    node_type* raw_pop()
    {
        size_type sz = m_size.load(barriers::relaxed);
        if (!sz)
        {
            return nullptr;
        }
        node_type* p = m_buffer + m_tail;
        if (++m_tail == m_capacity)
        {
            m_tail = 0;
        }
        return p;
    }
    node_type* raw_push()
    {
        size_type sz = m_size.load(barriers::relaxed);
        if (sz == m_capacity)
        {
            return nullptr;
        }
        node_type* p = m_buffer + m_head;
        if (++m_head == m_capacity)
        {
            m_head = 0;
        }
        return p;
    }
private:
    const size_type m_capacity;
    xtomic<size_type> m_size;
    size_type m_head;
    size_type m_tail;
    node_type* m_buffer;
    node_allocator_type m_nodeAllocator;
    data_allocator_type m_dataAllocator;
};
}

#endif /* INCLUDE_QUEUE_SPSCFS_HPP_ */
