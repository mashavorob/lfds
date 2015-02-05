/*
 * test_producer.hpp
 *
 *  Created on: Dec 16, 2014
 *      Author: masha
 */

#ifndef DEMO_TEST_PRODUCER_HPP_
#define DEMO_TEST_PRODUCER_HPP_

#include "test_events.hpp"

template<class Collection>
class test_producer
{
public:
    typedef Collection collection_type;
    typedef typename collection_type::value_type value_type;
    typedef typename collection_type::size_type size_type;
    typedef test_producer<Collection> this_type;

public:
    test_producer(collection_type & coll, const test_events::event& start,
            const std::atomic<bool> & stop) :
            m_coll(coll), m_start(start), m_stop(stop)
    {
    }

    void operator()()
    {
        value_type val;
        m_start.wait();
        static const size_type max_size = 100;
        while (!m_stop.load(std::memory_order_relaxed))
        {
            if (!collection_type::fixed_size && m_coll.size() > max_size)
            {
                continue;
            }
            m_coll.push(val);
        }
    }
private:
    collection_type& m_coll;
    test_events::event m_start;
    const std::atomic<bool>& m_stop;
};

#endif /* DEMO_TEST_PRODUCER_HPP_ */
