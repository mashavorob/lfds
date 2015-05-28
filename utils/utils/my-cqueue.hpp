/*
 * my-cqueue.hpp
 *
 *  Created on: Apr 14, 2015
 *      Author: masha
 */

#ifndef UTILS_UTILS_MY_CQUEUE_HPP_
#define UTILS_UTILS_MY_CQUEUE_HPP_

#include "my-sync.hpp"
#include "my-noncopyable.hpp"

#include <queue>

namespace xtomic
{
namespace my
{

template<typename T>
class cqueue : private noncopyable
{
public:
    typedef T value_type;
    typedef std::queue<T> queue_type;
    typedef std::size_t size_type;

public:
    cqueue()
    {

    }
    cqueue(size_type &)
    {

    }
    bool push(const value_type & val)
    {
        guard lock(m_mutex);
        m_q.push(val);
        return true;
    }
    bool pop(value_type & val)
    {
        guard lock(m_mutex);
        if ( m_q.empty() )
        {
            return false;
        }
        val = m_q.front();
        m_q.pop();
        return true;
    }
    size_type size()
    {
        guard lock(m_mutex);
        return m_q.size();
    }

private:
    mutex m_mutex;
    queue_type m_q;
};

}
}
#endif /* UTILS_UTILS_MY_CQUEUE_HPP_ */
