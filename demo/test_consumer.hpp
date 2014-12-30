/*
 * test_consumer.hpp
 *
 *  Created on: Dec 16, 2014
 *      Author: masha
 */

#ifndef DEMO_TEST_CONSUMER_HPP_
#define DEMO_TEST_CONSUMER_HPP_


#include <atomic>

template<class Collection>
class test_consumer
{
public:
	typedef Collection								collection_type;
	typedef typename collection_type::value_type 	value_type;
	typedef typename collection_type::size_type		size_type;
	typedef test_consumer<Collection>				this_type;

public:
	test_consumer(collection_type& coll, const std::atomic<bool> & stop)
		: m_coll(coll), m_counter(0), m_stop(stop)
	{ }
	~test_consumer()
	{
		flush_counter();
	}

	void operator()()
	{
		value_type val;
		while ( !m_stop.load(std::memory_order_relaxed) )
		{
			if ( m_coll.pop(val) )
			{
				++m_counter;
			}
		}
	}

	void flush_counter()
	{
		get_counter() += m_counter;
		m_counter = 0;
	}

	static void reset_perf_counter()
	{
		get_counter() = 0;
	}

	static size_type get_perf_counter()
	{
		return get_counter();
	}
private:
	static size_type & get_counter()
	{
		static size_type counter;
		return counter;
	}
private:
	collection_type&			m_coll;
	size_type					m_counter;
	const std::atomic<bool>&	m_stop;
};


#endif /* DEMO_TEST_CONSUMER_HPP_ */
