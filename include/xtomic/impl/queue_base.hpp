/*
 * queue_base.hpp
 *
 *  Created on: Dec 12, 2014
 *      Author: masha
 */

#ifndef INCLUDE_QUEUE_BASE_HPP_
#define INCLUDE_QUEUE_BASE_HPP_

#include "stack_base_weak.hpp"
#include "stack_base_aba.hpp"
#include <xtomic/xtomic.hpp>

namespace xtomic
{

namespace
{
template<typename Node, bool DoNotInvert>
struct node_inverter
{
    static Node* invert(Node* p);
};

template<typename Node>
struct node_inverter<Node, false>
{
    static Node* invert(Node* p)
    {
        Node* p1 = nullptr;
        while (p)
        {
            Node* next = p->m_next;
            p->m_next = p1;
            p1 = p;
            p = next;
        }
        return p1;
    }
};
template<typename Node>
struct node_inverter<Node, true>
{
    static Node* invert(Node* p)
    {
        return p;
    }
};

template<typename T>
class basic_queue_mc
{
public:
    typedef stack_base_weak<T> producer_stack_type;
    typedef stack_base_aba<T> consumer_stack_type;
    typedef typename producer_stack_type::node_type node_type;
    typedef typename std::size_t counter_type;

public:
    basic_queue_mc() :
            m_lock(0)
    {
    }
    node_type* atomic_pop()
    {
        // acquire consumer lock
        const counter_type count = ++m_lock;

        // attempt to pop an item
        node_type* p = m_consumerEnd.atomic_pop();
        if (!p && count == 1)
        {
            p = m_producerEnd.atomic_removeHead();
            m_consumerEnd.atomic_setHead(p);
            p = m_consumerEnd.atomic_pop();
        }
        --m_lock;
        return p;
    }
private:
    xtomic::quantum<counter_type> m_lock;
public:
    producer_stack_type m_producerEnd;
    consumer_stack_type m_consumerEnd;
};

template<typename T, bool ManyProducers>
class basic_queue_sc
{
public:
    typedef stack_base_weak<T> stack_type;
    typedef typename stack_type::node_type node_type;
    typedef node_inverter<node_type, ManyProducers> inverter_type;

public:
    node_type* atomic_pop()
    {
        // attempt to pop an item
        node_type* p = m_consumerEnd.pop();
        if (!p)
        {
            p = inverter_type::invert(m_producerEnd.atomic_removeHead());
            m_consumerEnd.setHead(p);
            p = m_consumerEnd.pop();
        }
        return p;
    }
public:
    stack_type m_producerEnd;
    stack_type m_consumerEnd;
};

template<typename T, bool ManyProducer, bool ManyConsumer>
struct basic_queue_traits;

template<typename T, bool ManyProducer>
struct basic_queue_traits<T, ManyProducer, true>
{
    typedef basic_queue_mc<T> basic_queue;
};

template<typename T, bool ManyProducer>
struct basic_queue_traits<T, ManyProducer, false>
{
    typedef basic_queue_sc<T, ManyProducer> basic_queue;
};

}

// many consumer version
template<typename T, bool ManyProducers, bool ManyConsumers>
class queue_base:
                  private basic_queue_traits<T, ManyProducers, ManyConsumers>::basic_queue
{
public:
    typedef queue_base<T, ManyProducers, ManyConsumers> this_class;
    typedef basic_queue_traits<T, ManyProducers, ManyConsumers> traits_type;
    typedef typename traits_type::basic_queue base_class;
    typedef typename base_class::node_type node_type;

public:
    static const bool many_producers = ManyProducers;
    static const bool many_consumers = ManyConsumers;
    static const bool wait_free = false;

private:
    queue_base(const this_class &);
    this_class & operator=(const this_class &);
public:
    queue_base()
    {
    }
    void atomic_push(node_type* p)
    {
        base_class::m_producerEnd.atomic_push(p);
    }
    node_type* atomic_pop()
    {
        return base_class::atomic_pop();
    }
    void push(node_type* p)
    {
        base_class::m_producerEnd.push(p);
    }
    node_type* pop()
    {
        node_type* p = base_class::m_consumerEnd.pop();
        if (!p)
        {
            p = base_class::m_producerEnd.pop();
            while (p)
            {
                base_class::m_consumerEnd.push(p);
                p = base_class::m_producerEnd.pop();
            }
            p = base_class::m_consumerEnd.pop();
        }
        return p;
    }
};

}
#endif /* INCLUDE_QUEUE_BASE_HPP_ */
