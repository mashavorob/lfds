/*
 * std_stack_wrapper.hpp
 *
 *  Created on: Dec 16, 2014
 *      Author: masha
 */

#ifndef DEMO_STD_STACK_WRAPPER_HPP_
#define DEMO_STD_STACK_WRAPPER_HPP_

#include <mutex>
#include <utility>

template<class Collection>
class std_stack_wrapper
{
public:
    typedef std_stack_wrapper<Collection> this_class;
    typedef Collection collection_type;
    typedef typename collection_type::value_type value_type;
    typedef typename collection_type::size_type size_type;

public:
    static const bool fixed_size = false;
    static const bool many_producers = true;
    static const bool many_consumers = true;

private:
    typedef std::lock_guard<std::mutex> lock_type;

private:
    std_stack_wrapper(const this_class&);
    this_class& operator=(const this_class&);

public:

    std_stack_wrapper(size_type)
    {
    }

    template<class ... Args>
    bool push(Args&&... args)
    {
        lock_type lock(m_mutex);
        m_coll.push(std::forward<Args>(args)...);
        return true;
    }

    bool pop(value_type& val)
    {
        lock_type lock(m_mutex);
        if (m_coll.empty())
        {
            return false;
        }
        val = m_coll.top();
        m_coll.pop();
        return true;
    }
    std::size_t size() const
    {
        lock_type lock(m_mutex);
        return m_coll.size();
    }

private:
    collection_type m_coll;
    mutable std::mutex m_mutex;
};

#endif /* DEMO_STD_STACK_WRAPPER_HPP_ */
