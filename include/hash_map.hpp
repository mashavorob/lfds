/*
 * hash_map
 *
 *  Created on: Jan 28, 2015
 *      Author: masha
 */

#ifndef INCLUDE_HASH_MAP_HPP_
#define INCLUDE_HASH_MAP_HPP_

#include "hash_table.hpp"

#include <functional>

namespace lfds
{

template<class Key, class Value, class Hash = std::hash<Key>,
        class Pred = std::equal_to<Key>, class Allocator = std::allocator<Value> >
class hash_map
{
public:

    typedef hash_map<Key, Value, Hash, Pred, Allocator> this_type;
    typedef lfds::hash_table<Key, Value, Hash, Pred, Allocator> hash_table_type;
    typedef typename hash_table_type::key_type key_type;
    typedef typename hash_table_type::value_type value_type;
    typedef typename hash_table_type::size_type size_type;
    typedef typename hash_table_type::value_allocator_type value_allocator_type;

    static const bool LOCK_FREE_FIND = hash_table_type::LOCK_FREE_FIND;
    static const bool LOCK_FREE_INSERT = hash_table_type::LOCK_FREE_INSERT;
    static const bool LOCK_FREE_DELETE = hash_table_type::LOCK_FREE_DELETE;

    static const unsigned int MIN_CAPACITY = hash_table_type::MIN_CAPACITY;

private:
    hash_map(const this_type&);
    this_type& operator=(const this_type&);

public:
    hash_map(const size_type initialCapacity =
            static_cast<size_type>(MIN_CAPACITY)) :
            m_key_allocator(), m_value_allocator(), m_node_allocator(), m_hash_table(
                    initialCapacity, m_key_allocator, m_value_allocator,
                    m_node_allocator)
    {

    }

    bool find(const key_type & key, value_type & value) const
    {
        return m_hash_table.find(key, value);
    }
    template<class ... Args>
    bool insert(const key_type & key, Args&&... val)
    {
        return m_hash_table.insert(key, std::forward<Args>(val)...);
    }
    bool erase(const key_type & key)
    {
        return m_hash_table.erase(key);
    }
    size_type size() const
    {
        return m_hash_table.size();
    }
    size_type capacity() const
    {
        return m_hash_table.capacity();
    }

private:
    typedef typename hash_table_type::key_allocator_type key_allocator_type;
    typedef typename hash_table_type::node_allocator_type node_allocator_type;

private:
    key_allocator_type m_key_allocator;
    value_allocator_type m_value_allocator;
    node_allocator_type m_node_allocator;
    hash_table_type m_hash_table;
};

}

#endif /* INCLUDE_HASH_MAP_HPP_ */
