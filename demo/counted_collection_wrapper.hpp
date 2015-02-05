/*
 * counted_wrapper.hpp
 *
 *  Created on: Dec 16, 2014
 *      Author: masha
 */

#ifndef DEMO_COUNTED_COLLECTION_WRAPPER_HPP_
#define DEMO_COUNTED_COLLECTION_WRAPPER_HPP_

#include <atomic>
#include <utility>

template<class Collection>
class counted_collection_wrapper
{
public:
    typedef Collection collection_type;
    typedef counted_collection_wrapper<Collection> this_class;
    typedef typename collection_type::value_type value_type;
    typedef typename collection_type::size_type size_type;

public:
    static const bool fixed_size = collection_type::fixed_size;
    static const bool many_producers = collection_type::many_producers;
    static const bool many_consumers = collection_type::many_producers;

private:
    counted_collection_wrapper(const this_class&);
    this_class& operator=(const this_class&);

public:
    counted_collection_wrapper(size_type capacity) :
            m_size(0), m_coll(capacity)
    {

    }

    template<class ... Args>
    bool push(Args&&... args)
    {
        bool success = m_coll.push(std::forward<Args>(args)...);
        if (success)
        {
            ++m_size;
        }
        return success;
    }

    bool pop(value_type & val)
    {
        bool success = m_coll.pop(val);
        if (success)
        {
            --m_size;
        }
        return success;
    }

    size_type size() const
    {
        return m_size.load(std::memory_order_relaxed);
    }

private:
    std::atomic<size_type> m_size;
    collection_type m_coll;
};

#endif /* DEMO_COUNTED_COLLECTION_WRAPPER_HPP_ */
