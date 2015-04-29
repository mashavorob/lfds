/*
 * queue.hpp
 *
 *  Created on: Dec 15, 2014
 *      Author: masha
 */

#ifndef INCLUDE_QUEUE_HPP_
#define INCLUDE_QUEUE_HPP_

#include "queue_base.hpp"
#include "queue_spscfs.hpp"
#include "buffer_traits.hpp"
#include "cppbasics.hpp"
#include "xtraits.hpp"

#include <utility>

namespace lfds
{

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

template<typename T, Queue::ESize SizeType = Queue::FixedSize,
        Queue::EMultiplicity NumProducers = Queue::ManyProducers,
        Queue::EMultiplicity NumConsumers = Queue::ManyConsumers,
        typename Allocator = std::allocator<T> >
class queue
{
public:
    typedef T value_type;
    typedef buffer_traits<T, Allocator, is_queue_fixed_size<SizeType>::value> traits_type;
    typedef typename traits_type::type buffer_type;
    typedef queue_base<T, are_many_producers<NumProducers>::value,
            are_many_consumers<NumConsumers>::value> queue_type;
    typedef typename buffer_type::size_type size_type;

public:
    static const bool fixed_size = buffer_type::fixed_size;
    static const bool many_producers = queue_type::many_producers;
    static const bool many_consumers = queue_type::many_consumers;
    static const bool wait_free = false;
private:
    typedef queue<T, SizeType, NumProducers, NumConsumers, Allocator> this_class;
    typedef typename buffer_type::node_type node_type;

private:
    queue(const this_class&);
    this_class& operator=(const this_class&);

public:

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
    queue_type m_queue;
    xtomic<size_type> m_size;
};

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

//
// Some sugar
//
template<typename T, typename Allocator = std::allocator<T> >
struct make_wait_free_queue
{
    typedef queue<T, Queue::FixedSize, Queue::OneProducer, Queue::OneConsumer,
            Allocator> type;
};

template<typename T, typename Allocator = std::allocator<T> >
struct make_fixed_size_queue
{
    typedef queue<T, Queue::FixedSize, Queue::ManyProducers,
            Queue::ManyConsumers, Allocator> type;
};

template<typename T, typename Allocator = std::allocator<T> >
struct make_dynamic_size_queue
{
    typedef queue<T, Queue::DynamicSize, Queue::ManyProducers,
            Queue::ManyConsumers, Allocator> type;
};

}

#endif /* INCLUDE_QUEUE_HPP_ */
