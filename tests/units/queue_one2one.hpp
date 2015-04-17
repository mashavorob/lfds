/*
 * queue_one2one.hpp
 *
 *  Created on: Apr 14, 2015
 *      Author: masha
 */

#ifndef TESTS_SYNC_QUEUE_ONE2ONE_HPP_
#define TESTS_SYNC_QUEUE_ONE2ONE_HPP_

#include <vector>
#include <pthread.h>

namespace lfds
{
namespace testing
{

template<typename Queue, int Size>
class queue_one2one
{
public:
    typedef queue_one2one<Queue, Size> this_type;
    typedef Queue queue_type;
    typedef typename queue_type::value_type value_type;
    typedef typename queue_type::size_type size_type;

private:
    struct node_type
    {
        size_type m_count;
        size_type m_index;
    };
    typedef std::vector<node_type> table_type;
    typedef typename table_type::const_iterator const_iterator;
public:
    queue_one2one(queue_type& q) :
            m_q(q),
            m_table(),
            m_failedPushes(0),
            m_numOfIncorrectValues(0)
    {

    }

    void run()
    {
        pthread_t consumer = 0;
        pthread_t producer = 0;

        node_type node = { 0, 0 };
        table_type table(Size, node);

        m_table.swap(table);

        m_failedPushes = 0;
        m_numOfIncorrectValues = 0;
        m_finish = false;

        void* parg = reinterpret_cast<void*>(this);
        pthread_create(&consumer, 0, &consumer_end, parg);
        pthread_create(&producer, 0, &producer_end, parg);

        pthread_join(producer, 0);
        m_finish = true;
        pthread_join(consumer, 0);
    }

    size_type getNumOfFailedPushes() const
    {
        return m_failedPushes;
    }
    size_type getNumOfIncorrectValues() const
    {
        return m_numOfIncorrectValues;
    }
    bool isDataComplete() const
    {
        const_iterator end = m_table.end();
        for ( const_iterator i = m_table.begin(); i != end; ++i )
        {
            const node_type & node = *i;
            if ( node.m_count == 0 )
            {
                return false;
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
        for ( const_iterator i = beg; i != end; ++i, ++index )
        {
            const node_type & node = *i;
            if ( node.m_count == 0 )
            {
                continue;
            }
            int diff = static_cast<int>(index - node.m_index);
            if ( diff < 0 )
            {
                diff = -diff;
            }
            if ( diff > maxDiff )
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

        for ( size_type order = 0; ; ++order )
        {
            value_type val = static_cast<value_type>(-1);
            bool res;
            do
            {
                res = This.m_q.pop(val);
            }
            while (!res && !This.m_finish);

            if ( !res )
            {
                break;
            }

            int index = static_cast<int>(val);
            if ( index < 0 || index >= table.size() )
            {
                ++This.m_numOfIncorrectValues;
            }
            else
            {
                node_type& node = table[index];

                ++node.m_count;
                node.m_index = order;

                if ( node.m_count > 1 )
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

        for (int i = 0; i < Size; ++i)
        {
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
    table_type m_table;
    size_type m_failedPushes;
    size_type m_numOfIncorrectValues;
    volatile bool m_finish;
};

}
}
#endif /* TESTS_SYNC_QUEUE_ONE2ONE_HPP_ */
