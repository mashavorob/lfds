/*
 * stack.hpp
 *
 *  Created on: Dec 15, 2014
 *      Author: masha
 */

#ifndef INCLUDE_STACK_HPP_
#define INCLUDE_STACK_HPP_

#include "stack_base_aba.hpp"
#include "xtomic.hpp"
#include "cppbasics.hpp"

#include <utility>

#include "buffer_traits.hpp"

namespace lfds
{

template<typename T, bool FixedSize = true, typename Allocator = std::allocator<
        T> >
class stack
{
public:
    typedef T value_type;
    typedef buffer_traits<T, Allocator, FixedSize> traits_type;
    typedef typename traits_type::type buffer_type;
    typedef stack_base_aba<T> stack_type;
    typedef typename buffer_type::size_type size_type;
private:
    typedef stack<T, FixedSize, Allocator> this_class;
    typedef typename buffer_type::node_type node_type;

public:
    static const bool fixed_size = buffer_type::fixed_size;
    static const bool many_producers = stack_type::many_producers;
    static const bool many_consumers = stack_type::many_consumers;

private:
    stack(const this_class&);
    this_class& operator=(const this_class&);

public:

    stack(size_type capacity) :
            m_buff(capacity),
            m_size(0)
    {
    }

    ~stack()
    {
        node_type* p = m_stack.pop();
        while (p)
        {
            m_buff.freeNode(p);
            p = m_stack.pop();
        }
    }

#if LFDS_USE_CPP11
    template<typename ... Args>
    bool push(Args&&... val)
#else
    bool push(const value_type& val)
#endif
    {
        node_type* p = m_buff.newNode(std_forward(Args, val));
        if (!p)
        {
            return false;
        }
        m_stack.atomic_push(p);
        ++m_size;
        return true;
    }

    bool pop(T & val)
    {
        node_type* p = m_stack.atomic_pop();
        if (!p)
        {
            return false;
        }
        --m_size;
        val = std_move(*p->getData());
        m_buff.freeNode(p);
        return true;
    }

    size_type getCapacity() const
    {
        return m_buff.getCapacity();
    }
    size_type size() const
    {
        return m_size.load(barriers::relaxed);
    }

private:
    buffer_type m_buff;
    stack_type m_stack;
    xtomic<size_type> m_size;
};

}

#endif /* INCLUDE_STACK_HPP_ */
