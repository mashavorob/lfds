/*
 * stack_base_weak.hpp
 *
 *  Created on: Dec 23, 2014
 *      Author: masha
 */

#ifndef INCLUDE_STACK_BASE_WEAK_HPP_
#define INCLUDE_STACK_BASE_WEAK_HPP_

#include <atomic>
#include "stack_node.hpp"

namespace lfds
{

// base implementation for stack
template<class T>
class stack_base_weak
{
public:
    typedef stack_base_weak<T> this_class;
    typedef stack_node<T> node_type;

public:
    static const bool many_producers = true;
    static const bool many_consumers = true;
    static const bool aba_proof = false;

private:
    // non copyable
    stack_base_weak(const this_class &);
    this_class & operator=(const this_class &);

public:
    stack_base_weak() :
            m_head(nullptr)
    {
    }

    // Atomic operations
    void atomic_push(node_type* p)
    {
        bool success;
        do
        {
            node_type* expected = m_head.load(std::memory_order_relaxed);
            p->m_next = expected;
            success = std::atomic_compare_exchange_weak(&m_head, &expected, p);
        } while (!success);
    }
    node_type* atomic_pop()
    {
        bool success;
        node_type* p;
        do
        {
            p = m_head.load(std::memory_order_relaxed);
            if (!p)
            {
                return nullptr;
            }
            success = std::atomic_compare_exchange_weak(&m_head, &p, p->m_next);
        } while (!success);
        return p;
    }
    node_type* atomic_remove_head()
    {
        bool success;
        node_type* p;
        node_type* head = nullptr;
        do
        {
            p = m_head.load(std::memory_order_relaxed);
            if (!p)
            {
                return nullptr;
            }
            success = std::atomic_compare_exchange_weak(&m_head, &p, head);
        } while (!success);
        return p;
    }

    // Synchronous operations
    void push(node_type* p)
    {
        p->m_next = m_head.load(std::memory_order_relaxed);
        m_head.store(p, std::memory_order_relaxed);
    }
    node_type* pop()
    {
        node_type* p = m_head.load(std::memory_order_relaxed);
        if (p)
        {
            m_head.store(p->m_next, std::memory_order_relaxed);
            p->m_next = nullptr;
        }
        return p;
    }
    void swap(this_class & other)
    {
        node_type* head1 = m_head.load(std::memory_order_relaxed);
        node_type* head2 = other.m_head.load(std::memory_order_relaxed);
        m_head.store(head2, std::memory_order_relaxed);
        other.m_head.store(head1, std::memory_order_relaxed);
    }
    void set_head(node_type* p)
    {
        m_head.store(p, std::memory_order_relaxed);
    }
private:
    std::atomic<node_type*> m_head;
};

}

#endif /* INCLUDE_STACK_BASE_WEAK_HPP_ */
