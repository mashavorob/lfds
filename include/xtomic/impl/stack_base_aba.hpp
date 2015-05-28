/*
 * stack_base.hpp
 *
 *  Created on: Dec 12, 2014
 *      Author: masha
 */

#ifndef INCLUDE_STACK_BASE_ABA_HPP_
#define INCLUDE_STACK_BASE_ABA_HPP_

#include "aba_ptr.hpp"
#include "stack_node.hpp"

namespace xtomic
{

// base implementation for stack
template<typename T>
class stack_base_aba
{
public:
    typedef stack_node<T> node_type;
    typedef aba_ptr<node_type> node_ptr;

public:
    static const bool many_producers = true;
    static const bool many_consumers = true;
    static const bool aba_proof = true;

private:
    // non copyable
    stack_base_aba(const stack_base_aba<T>&);
    stack_base_aba<T>& operator=(const stack_base_aba<T>&);

public:
    stack_base_aba() :
            m_head(nullptr)
    {
    }

    void atomic_push(node_type* p)
    {
        bool success;
        node_ptr expected;
        node_ptr newhead(p);
        do
        {
            expected = m_head;
            newhead.m_counter = expected.m_counter + 1;
            p->m_next = expected.m_ptr;
            success = m_head.atomic_cas(expected, newhead);
        }
        while (!success);
    }
    void atomic_setHead(node_type* p)
    {
        bool success;
        node_ptr expected;
        node_ptr newhead(p);
        do
        {
            expected.m_counter = m_head.m_counter;
            newhead.m_counter = expected.m_counter + 1;
            success = m_head.atomic_cas(expected, newhead);
        }
        while (!success);
    }
    node_type* atomic_pop()
    {
        bool success;
        node_ptr expected;
        node_ptr newhead;
        do
        {
            expected = m_head;
            if (!expected.m_ptr)
            {
                return nullptr;
            }
            newhead.m_ptr = expected.m_ptr->m_next;
            newhead.m_counter = expected.m_counter + 1;
            success = m_head.atomic_cas(expected, newhead);
        }
        while (!success);
        return expected.m_ptr;
    }
    void push(node_type* p)
    {
        p->m_next = m_head.m_ptr;
        m_head.m_ptr = p;
    }
    node_type* pop()
    {
        node_type* p = m_head.m_ptr;
        if (p)
        {
            m_head.m_ptr = p->m_next;
            p->m_next = nullptr;
            ++m_head.m_counter;
        }
        return p;
    }

    static void copy_upsidedown(stack_base_aba<T> & from,
                                stack_base_aba<T> & to)
    {
        node_type* node = from.m_head.m_ptr;
        node_type* newhead = to.m_head.m_ptr;
        while (node)
        {
            node_type* next = node->m_next;
            node->m_next = newhead;
            newhead = node;
            node = next;
        }
        to.m_head.m_ptr = newhead;
        from.m_head.m_ptr = nullptr;
        ++from.m_head.m_counter;
        ++to.m_head.m_counter;
    }
private:
    volatile node_ptr m_head;
};

}

#endif /* INCLUDE_STACK_BASE_ABA_HPP_ */
