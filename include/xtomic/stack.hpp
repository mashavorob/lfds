/*
 * stack.hpp
 *
 *  Created on: Dec 15, 2014
 *      Author: masha
 */

/// \file stack.hpp

#ifndef INCLUDE_STACK_HPP_
#define INCLUDE_STACK_HPP_

/// \cond HIDDEN_SYMBOLS
#include "impl/stack_base_aba.hpp"
#include "impl/buffer_traits.hpp"
#include "aux/cppbasics.hpp"
#include "xtomic.hpp"

#include <utility>
/// \endcond

namespace xtomic
{

///
/// \brief The class represents container type of stack (LIFO - last-in-first-out). The container is thread-safe
/// and does not require additional synchronization when used in multy-thread environment.
///
/// The class implements optimization for stacks of fixed size.
///
/// @param T specifies items' type.
/// @param FixedSize specifies whether or not stack of fixed size is needed. Default value is `true`.
/// @param Allocator specifies allocator type. Default value is std::allocator<T>.
///
template<typename T, bool FixedSize = true, typename Allocator = std::allocator<
        T> >
class stack
{
public:
    /// \cond HIDDEN_SYMBOLS
    typedef buffer_traits<T, Allocator, FixedSize> traits_type;
    typedef typename traits_type::type buffer_type;
    typedef stack_base_aba<T> stack_type;
    /// \endcond

    typedef T value_type;                               ///< type of stack's items.
    typedef typename buffer_type::size_type size_type;  ///< size type.
private:
    typedef stack<T, FixedSize, Allocator> this_class;
    typedef typename buffer_type::node_type node_type;

public:
    static const bool fixed_size = buffer_type::fixed_size;        ///< true if fixed sized stack is used.
    static const bool many_producers = stack_type::many_producers; ///< true if many producers may push to the same stack simultaneously. The value is always true and supported for future use.
    static const bool many_consumers = stack_type::many_consumers; ///< true if many consumers may consume data from the same stack simultaneously. The value is always true and supported for future use.
private:
    stack(const this_class&);
    this_class& operator=(const this_class&);

public:

    ///
    ///\brief Constructor.
    ///
    /// @param capacity specifies initial capacity of the stack.
    ///
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

    ///
    /// \brief The method inserts new item into the stack.
    ///
    /// @param val
    ///    - *C++11*: initializer list to construct a new item to insert.
    ///    - *Dinosourus C*++: specifies a value of a new item.
    /// @return true if new item was inserted successfully.
    ///
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

    ///
    /// \brief The method extracts an item from the stack.
    ///
    /// @param val specifies reference to a variable to hold extracted value.
    /// @return
    /// * `false` if the queue was empty and nothing was extracted.
    /// * `true` if value was successfully extracted.
    ///
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

    ///
    /// \brief The method returns capacity of the stack. For fixed sized stacks the value limits maximum
    /// number of elements that can be inserted.
    ///
    /// @return current capacity of the container.
    ///
    size_type getCapacity() const
    {
        return m_buff.getCapacity();
    }

    ///
    /// \brief The method returns number of elements in the stack.
    ///
    /// @return number of elements in the queue.
    ///
    size_type size() const
    {
        return m_size.load(barriers::relaxed);
    }

private:
    buffer_type m_buff;
    stack_type m_stack;
    xtomic::quantum<size_type> m_size;
};

}

#endif /* INCLUDE_STACK_HPP_ */
