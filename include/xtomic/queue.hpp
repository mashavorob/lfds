/*
 * queue.hpp
 *
 *  Created on: Dec 15, 2014
 *      Author: masha
 */

/// \file queue.hpp

#ifndef INCLUDE_QUEUE_HPP_
#define INCLUDE_QUEUE_HPP_

/// \cond HIDDEN_SYMBOLS

#include "impl/queue_base.hpp"
#include "impl/queue_spscfs.hpp"
#include "impl/buffer_traits.hpp"
#include "impl/xtraits.hpp"
#include "aux/cppbasics.hpp"

#include <utility>

/// \endcond

namespace xtomic
{

/// \cond HIDDEN_SYMBOLS
struct Queue
{
    enum ESize
    {
        FixedSize, DynamicSize,
    };

    enum EMultiplicity
    {
        ManyProducers, OneProducer, ManyConsumers, OneConsumer,
    };
};

namespace
{

template<Queue::ESize>
struct is_queue_fixed_size;

template<Queue::EMultiplicity>
struct are_many_producers;

template<Queue::EMultiplicity>
struct are_many_consumers;

template<>
struct is_queue_fixed_size<Queue::FixedSize> : public integral_const<bool, true>
{

};

template<>
struct is_queue_fixed_size<Queue::DynamicSize> : public integral_const<bool,
        false>
{

};

template<>
struct are_many_producers<Queue::ManyProducers> : public integral_const<bool,
        true>
{

};

template<>
struct are_many_producers<Queue::OneProducer> :
                                                public integral_const<bool, true>
{

};

template<>
struct are_many_consumers<Queue::ManyConsumers> : public integral_const<bool,
        true>
{

};

template<>
struct are_many_consumers<Queue::OneConsumer> : public integral_const<bool,
        false>
{

};

}
/// \endcond

///
/// \class queue
///
/// \brief The thread-safe implementation of queue. The class does not require additional
/// synchronization when used in multi-thread environment.
///
/// The class implements different optimizations depending on:
/// - if fixed size is used.
/// - number of producers/consumers.
///
/// The best case is when three following restrictions are satisfied:
/// - single producer.
/// - single consumer.
/// - fixed size.
/// In this case wait-free implementation is used.
///
/// @param T type of element.
/// @param SizeType specifies if fixed size queue is used or not. Default is FixedSize.
/// @param NumProducers specifies how many producers may shirt data into the same queue
///        concurrently. Default is ManyProducers.
/// @param NumConsumers specifies how many consumers may consume data from the same queue
///        concurrently. Default is ManyConsumers.
/// @param Allocator specifies type of allocator. Default is std::allocator<T>.
///
/// There are helper templates that allows to instantiate special cases of the queues:
/// - make_fixed_size_queue
/// - make_dynamic_size_queue
///
template<typename T, Queue::ESize SizeType = Queue::FixedSize,
        Queue::EMultiplicity NumProducers = Queue::ManyProducers,
        Queue::EMultiplicity NumConsumers = Queue::ManyConsumers,
        typename Allocator = std::allocator<T> >
class queue
{
public:
    /// \cond HIDDEN_SYMBOLS
    typedef buffer_traits<T, Allocator, is_queue_fixed_size<SizeType>::value> traits_type;
    typedef typename traits_type::type buffer_type;
    typedef queue_base<T, are_many_producers<NumProducers>::value,
            are_many_consumers<NumConsumers>::value> queue_type;
    /// \endcond

    typedef T value_type;                                           ///< value type.
    typedef typename buffer_type::size_type size_type;              ///< size type.

public:
    static const bool fixed_size = buffer_type::fixed_size;         ///< true if the queue has fixed size.
    static const bool many_producers = queue_type::many_producers;  ///< true if many producers may shirt data simultaneously.
    static const bool many_consumers = queue_type::many_consumers;  ///< true if many consumers may eat data simultaneously.
    static const bool wait_free = false;                            ///< true if wait-free implementation is used.
private:
    typedef queue<T, SizeType, NumProducers, NumConsumers, Allocator> this_class;
    typedef typename buffer_type::node_type node_type;

private:
    queue(const this_class&);
    this_class& operator=(const this_class&);

public:

    ///
    /// \brief Constructor.
    ///
    /// @param capacity specifies initial capacity of the queue.
    ///
    queue(size_type capacity) :
            m_buff(capacity),
            m_size(0)
    {
    }

    ~queue()
    {
        node_type* p = m_queue.pop();
        while (p)
        {
            m_buff.freeNode(p);
            p = m_queue.pop();
        }
    }

    ///
    /// \brief The method inserts new item into the queue.
    ///
    /// @param data
    /// - *C++11*: initializer list to construct a new item to insert.
    /// - *Dinosourus C*++: specifies a value of a new item.
    /// @return
    /// - `true` if a new item was inserted successfully.
    /// - `false` if the container has no capacity to finish the operation.
    ///
    /// *Note:* in case of dynamic sized queue the operation always returns `true`.
    ///
#if LFDS_USE_CPP11
    template<typename ... Args>
    bool push(Args&&... data)
#else
    bool push(const value_type& data)
#endif
    {
        node_type* p = m_buff.newNode(std_forward(Args, data));
        if (!p)
        {
            return false;
        }
        m_queue.atomic_push(p);
        ++m_size;
        return true;
    }

    ///
    /// \brief The method extracts an item from the queue.
    ///
    /// @param val specifies reference to a variable to hold extracted value.
    /// @return
    /// - `false` if the queue was empty and nothing was extracted.
    /// - `true` if value was successfully extracted.
    ///
    bool pop(T & val)
    {
        node_type* p = m_queue.atomic_pop();
        if (!p)
        {
            return false;
        }
        val = std_move(*p->getData());
        m_buff.freeNode(p);
        --m_size;
        return true;
    }

    ///
    /// \brief The method returns capacity of the queue. For fixed sized queues the value limits maximum
    /// number of elements that can be inserted.
    ///
    /// @return current capacity of the container.
    ///
    size_type getCapacity() const
    {
        return m_buff.getCapacity();
    }

    ///
    /// \brief The method returns number of elements in the queue.
    ///
    /// @return number of elements in the queue.
    ///
    size_type size() const
    {
        return m_size.load(barriers::relaxed);
    }
private:
    buffer_type m_buff;
    queue_type m_queue;
    xtomic::quantum<size_type> m_size;
};

/// \cond HIDDEN_SYMBOLS
template<typename T, typename Allocator>
class queue<T, Queue::FixedSize, Queue::OneProducer, Queue::OneConsumer,
        Allocator>
{
public:
    typedef queue<T, Queue::FixedSize, Queue::OneProducer, Queue::OneConsumer,
            Allocator> this_class;
    typedef queue_spscfs<T, Allocator> queue_type;
    typedef typename queue_type::value_type value_type;
    typedef typename queue_type::size_type size_type;
private:
    queue(const this_class&);
    this_class& operator=(const this_class&);
public:
    static const bool fixed_size = true;
    static const bool many_producers = false;
    static const bool many_consumers = false;
    static const bool wait_free = true;
public:
    queue(size_type sz) :
            m_queue(sz)
    {
    }

#if LFDS_USE_CPP11
    template<typename ... Args>
    bool push(Args&&... data)
#else
    bool push(const value_type& data)
#endif
    {
        return m_queue.push(std_forward(Args, data));
    }
    bool pop(T & val)
    {
        return m_queue.pop(val);
    }
    size_type size() const
    {
        return m_queue.size();
    }
private:
    queue_type m_queue;
};
/// \endcond

///
/// \class make_wait_free_queue
///
/// \brief The template implements a shortcut to instantiate a wait-free queue.
///
/// See details [queue](@ref queue)
///
template<typename T, typename Allocator = std::allocator<T> >
struct make_wait_free_queue
{
    /// wait-free queue type.
    typedef queue<T, Queue::FixedSize, Queue::OneProducer, Queue::OneConsumer,
            Allocator> type;
};

///
/// \class make_fixed_size_queue
///
/// \brief The template implements a shortcut to instantiate a fixed sized queue for an
/// arbitrary number of producers and consumers.  See details [queue](@ref queue)
///
template<typename T, typename Allocator = std::allocator<T> >
struct make_fixed_size_queue
{
    /// fixed sized queue type.
    typedef queue<T, Queue::FixedSize, Queue::ManyProducers,
            Queue::ManyConsumers, Allocator> type;
};

///
/// \class make_dynamic_size_queue
///
/// \brief The template implements a shortcut to instantiate a dynamic sized queue for an
/// arbitrary number of producers and consumers.  See details [queue](@ref queue)
///
template<typename T, typename Allocator = std::allocator<T> >
struct make_dynamic_size_queue
{
    /// dynamic sized queue type.
    typedef queue<T, Queue::DynamicSize, Queue::ManyProducers,
            Queue::ManyConsumers, Allocator> type;
};

}

#endif /* INCLUDE_QUEUE_HPP_ */
