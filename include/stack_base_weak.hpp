/*
 * stack_base_weak.hpp
 *
 *  Created on: Dec 23, 2014
 *      Author: masha
 */

#ifndef INCLUDE_STACK_BASE_WEAK_HPP_
#define INCLUDE_STACK_BASE_WEAK_HPP_

#include "xtomic.hpp"
#include "cas.hpp"
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
            node_type* expected = m_head.load(barriers::relaxed);
            p->m_next = expected;
            success = m_head.cas(expected, p);
        } while (!success);
    }
    node_type* atomic_pop()
    {
        bool success;
        node_type* p;
        do
        {
            p = m_head.load(barriers::relaxed);
            if (!p)
            {
                return nullptr;
            }
            node_type* n = p->m_next;
            success = m_head.cas(p, n);
        } while (!success);
        return p;
    }
    node_type* atomic_removeHead()
    {
        bool success;
        node_type* p;
        node_type* head = nullptr;
        do
        {
            p = m_head.load(barriers::relaxed);
            if (!p)
            {
                return nullptr;
            }
            success = m_head.cas(p, head);
        } while (!success);
        return p;
    }

    // Synchronous operations
    void push(node_type* p)
    {
        p->m_next = m_head.load(barriers::relaxed);
        m_head.store(p, barriers::relaxed);
    }
    node_type* pop()
    {
        node_type* p = m_head.load(barriers::relaxed);
        if (p)
        {
            m_head.store(p->m_next, barriers::relaxed);
            p->m_next = nullptr;
        }
        return p;
    }
    void swap(this_class & other)
    {
        node_type* head1 = m_head.load(barriers::relaxed);
        node_type* head2 = other.m_head.load(barriers::relaxed);
        m_head.store(head2, barriers::relaxed);
        other.m_head.store(head1, barriers::relaxed);
    }
    void setHead(node_type* p)
    {
        m_head.store(p, barriers::relaxed);
    }
private:
    xtomic<node_type*> m_head;
};

}

#endif /* INCLUDE_STACK_BASE_WEAK_HPP_ */
