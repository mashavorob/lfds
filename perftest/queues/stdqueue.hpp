/*
 * stdqueue.hpp
 *
 *  Created on: Jun 3, 2015
 *      Author: masha
 */

#ifndef PERFTEST_QUEUES_STDQUEUE_HPP_
#define PERFTEST_QUEUES_STDQUEUE_HPP_

#include "testsync.hpp"

#include <queue>

namespace xtomic
{
namespace perftest
{
namespace queues
{

namespace adapter
{

template<typename T, typename Allocator=std::allocator<T> >
class stdqueue
{
private:
    typedef stdqueue<T, Allocator> this_type;
    typedef xtomic::perftest::sync::mutex mutex_type;
    typedef xtomic::perftest::sync::guard guard_type;

public:
    typedef std::deque<T, Allocator> sequence_type;
    typedef std::queue<T, sequence_type> collection_type;

    typedef typename collection_type::value_type value_type;
    typedef typename collection_type::size_type size_type;

public:
    stdqueue(size_type dummy = 0);

    bool push(const value_type& v)
    {
        guard_type guard(m_mutex);
        m_coll.push(v);
        return true;
    }
    bool pop(value_type& v)
    {
        guard_type guard(m_mutex);
        if ( m_coll.empty() )
        {
            return false;
        }
        v = m_coll.front();
        m_coll.pop();
        return true;
    }
    size_type size() const
    {
        guard_type guard(m_mutex);
        return m_coll.size();
    }

private:
    stdqueue(const this_type&);
    this_type& operator=(const this_type&);
private:
    mutable mutex_type m_mutex;
    collection_type m_coll;
};

template<typename T, typename Allocator>
inline stdqueue<T, Allocator>::stdqueue(size_type)
{

}

}
}
}
}
#endif /* PERFTEST_QUEUES_STDQUEUE_HPP_ */
