/*
 * queue_many2many.hpp
 *
 *  Created on: Apr 15, 2015
 *      Author: masha
 */

#ifndef TESTS_SYNC_QUEUE_MANY2MANY_HPP_
#define TESTS_SYNC_QUEUE_MANY2MANY_HPP_

#include <xtomic/xtomic.hpp>

#include <vector>
#include <pthread.h>

#include <iostream>

namespace lfds
{
namespace testing
{

template<typename Queue, int Size, int Producers, int Consumers>
class queue_many2many
{
public:
    typedef queue_many2many<Queue, Size, Producers, Consumers> this_type;
    typedef Queue queue_type;
    typedef typename queue_type::value_type value_type;
    typedef typename queue_type::size_type size_type;

private:
    struct node_type
    {
        lfds::xtomic<size_type> m_count;
        lfds::xtomic<size_type> m_index;

        node_type() :
                m_count(0),
                m_index(0)
        {

        }
        node_type(const node_type& other) :
                m_count(other.m_count.load(lfds::barriers::relaxed)),
                m_index(other.m_index.load(lfds::barriers::relaxed))
        {

        }
        node_type operator=(const node_type& other)
        {
            m_count.store(other.m_count.load(lfds::barriers::relaxed),
                    lfds::barriers::relaxed);
            m_index.store(other.m_index.load(lfds::barriers::relaxed),
                    lfds::barriers::relaxed);
            return *this;
        }
    };

    typedef std::vector<node_type> table_type;
    typedef typename table_type::const_iterator const_iterator;
public:
    queue_many2many(queue_type& q) :
            m_q(q),
            m_table()
    {

    }

    void run()
    {
        pthread_t consumers[Consumers] =
        { 0 };
        pthread_t producers[Producers] =
        { 0 };

        table_type table(Size);

        m_table.swap(table);

        m_finish = false;
        m_index.store(-1, lfds::barriers::relaxed);
        m_failedPushes.store(0, lfds::barriers::relaxed);
        m_numOfIncorrectValues.store(0, lfds::barriers::relaxed);
        void* parg = reinterpret_cast<void*>(this);

        // ensure that all data is written at this point
        lfds::thread_fence(lfds::barriers::release);

        // start threads
        for (int i = 0; i < Consumers; ++i)
        {
            pthread_create(&consumers[i], 0, &consumer_end, parg);
        }
        for (int i = 0; i < Producers; ++i)
        {
            pthread_create(&producers[i], 0, &producer_end, parg);
        }

        // wait for producers
        for (int i = 0; i < Producers; ++i)
        {
            pthread_join(producers[i], 0);
        }
        m_finish = true;
        // wait for consumers
        for (int i = 0; i < Consumers; ++i)
        {
            pthread_join(consumers[i], 0);
        }
    }

    size_type getNumOfFailedPushes() const
    {
        return m_failedPushes.load(lfds::barriers::relaxed);
    }
    size_type getNumOfIncorrectValues() const
    {
        return m_numOfIncorrectValues.load(lfds::barriers::relaxed);
    }
    bool isDataComplete() const
    {
        bool res = true;
        const_iterator end = m_table.end();
        int index = 0;
        for (const_iterator i = m_table.begin(); i != end; ++i, ++index)
        {
            const node_type & node = *i;
            if (node.m_count.load(lfds::barriers::relaxed) == 0)
            {
                res = false;
                std::cerr << "***item " << index << " was not received" << std::endl;
            }
        }
        return true;
    }
    int getMaxSequenceDiff() const
    {
        const_iterator beg = m_table.begin();
        const_iterator end = m_table.end();

        int maxDiff = 0;
        size_type index = 0;
        for (const_iterator i = beg; i != end; ++i, ++index)
        {
            const node_type & node = *i;
            if (node.m_count.load(lfds::barriers::relaxed) != 1)
            {
                continue;
            }
            int diff = static_cast<int>(index
                    - node.m_index.load(lfds::barriers::relaxed));
            if (diff < 0)
            {
                diff = -diff;
            }
            if (diff > maxDiff)
            {
                maxDiff = diff;
            }
        }
        return maxDiff;
    }
private:
    static void* consumer_end(void* parg)
    {
        this_type& This = *reinterpret_cast<this_type*>(parg);
        queue_type & q = This.m_q;
        table_type & table = This.m_table;

        for (size_type order = 0;; ++order)
        {
            value_type val = static_cast<value_type>(-1);
            bool res;
            do
            {
                res = q.pop(val);
            }
            while (!res && !This.m_finish);

            if (!res)
            {
                break;
            }

            int index = static_cast<int>(val);
            if (index < 0 || index >= table.size())
            {
                ++This.m_numOfIncorrectValues;
            }
            else
            {
                node_type& node = table[index];

                int count = ++node.m_count;
                if ( count == 1)
                {
                    node.m_index.store(order, lfds::barriers::release);
                }

                if (count > 1)
                {
                    ++This.m_numOfIncorrectValues;
                }
            }
        }
        pthread_exit(0);
    }
    static void* producer_end(void* parg)
    {
        this_type& This = *reinterpret_cast<this_type*>(parg);

        for (;;)
        {
            int i = ++This.m_index;
            if (i >= Size)
            {
                break;
            }
            const value_type val = static_cast<value_type>(i);
            bool res;
            do
            {
                res = This.m_q.push(val);
                if (!res)
                {
                    ++This.m_failedPushes;
                }
            }
            while (!res);
        }
        pthread_exit(0);
    }
private:
    queue_type& m_q;
    lfds::xtomic<int> m_index;
    table_type m_table;
    lfds::xtomic<size_type> m_failedPushes;
    lfds::xtomic<size_type> m_numOfIncorrectValues;
    volatile bool m_finish;
};

}
}

#endif /* TESTS_SYNC_QUEUE_MANY2MANY_HPP_ */
