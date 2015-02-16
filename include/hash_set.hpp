/*
 * hash_set.hpp
 *
 *  Created on: Feb 12, 2015
 *      Author: masha
 */

#ifndef INCLUDE_HASH_SET_HPP_
#define INCLUDE_HASH_SET_HPP_

#include "hash_set_table.hpp"
#include "hash_set_table_integral_key.hpp"
#include "hash_map.hpp"

#include <type_traits>

namespace lfds
{

namespace
{

// hash_set is an adapter for hash_map
template<class T, class Hash, class Pred, class Allocator>
class hash_set_table_bridge
{
public:
    typedef hash_set_table<T, Hash, Pred, Allocator> hash_table_type;
    typedef typename hash_table_type::size_type size_type;
    typedef typename hash_table_type::key_type key_type;
    typedef typename hash_table_type::hash_type hash_type;
    typedef typename hash_table_type::equal_predicate_type equal_predicate_type;
    typedef typename hash_table_type::node_allocator_type allocator_type;

    static constexpr bool INTEGRAL = hash_table_type::INTEGRAL;
public:
    hash_set_table_bridge(size_type initialCapacity) :
            m_key_allocator(), m_node_allocator(), m_hash_table(initialCapacity,
                    m_key_allocator, m_node_allocator)
    {

    }

    hash_table_type & getHashTable()
    {
        return m_hash_table;
    }
    const hash_table_type & getHashTable() const
    {
        return m_hash_table;
    }

private:
    typedef typename hash_table_type::key_allocator_type key_allocator_type;

private:
    key_allocator_type m_key_allocator;
    allocator_type m_node_allocator;
    hash_table_type m_hash_table;
};

// hash_set is an adapter for hash_map
template<class T, class Hash, class Pred, class Allocator>
class hash_set_table_bridge_integral_key
{
public:
    typedef hash_set_table_integral_key<T, Hash, Pred, Allocator> hash_table_type;
    typedef typename hash_table_type::size_type size_type;
    typedef typename hash_table_type::key_type key_type;
    typedef typename hash_table_type::hash_type hash_type;
    typedef typename hash_table_type::equal_predicate_type equal_predicate_type;
    typedef typename hash_table_type::node_allocator_type allocator_type;

    static constexpr bool INTEGRAL = hash_table_type::INTEGRAL;
public:
    hash_set_table_bridge_integral_key(size_type initialCapacity) :
            m_node_allocator(), m_hash_table(initialCapacity, m_node_allocator)
    {

    }

    hash_table_type & getHashTable()
    {
        return m_hash_table;
    }
    const hash_table_type & getHashTable() const
    {
        return m_hash_table;
    }

private:
    allocator_type m_node_allocator;
    hash_table_type m_hash_table;
};

// hash_set is an adapter for hash_map
template<class T, class Hash, class Pred, class Allocator, bool = std::is_integral<T>::value>
struct hash_set_table_selector;

template<class T, class Hash, class Pred, class Allocator>
struct hash_set_table_selector<T, Hash, Pred, Allocator, false>
{
    typedef hash_set_table_bridge<T, Hash, Pred, Allocator> type;
};

template<class T, class Hash, class Pred, class Allocator>
struct hash_set_table_selector<T, Hash, Pred, Allocator, true>
{
    typedef hash_set_table_bridge_integral_key<T, Hash, Pred, Allocator> type;
};
}

// hash_set is an adapter for hash_map
template<class T, class Hash = std::hash<T>, class Pred = std::equal_to<T>,
        class Allocator = std::allocator<T> >
class hash_set
{
public:
    typedef hash_set<T, Hash, Pred, Allocator> this_type;
    typedef typename hash_set_table_selector<T, Hash, Pred, Allocator>::type bridge_type;
    typedef typename bridge_type::hash_table_type hash_table_type;
    typedef typename bridge_type::size_type size_type;
    typedef typename bridge_type::key_type key_type;
    typedef typename bridge_type::hash_type hash_type;
    typedef typename bridge_type::equal_predicate_type equal_predicate_type;
    typedef typename bridge_type::allocator_type allocator_type;

    static constexpr int INTEGRAL = bridge_type::INTEGRAL;

    hash_set(size_type initialCapacity = 0) :
        m_bridge(initialCapacity)
    {
    }
    bool find(const key_type & key) const
    {
        return m_bridge.getHashTable().find(key);
    }
    bool insert(const key_type & key)
    {
        return m_bridge.getHashTable().insert(key);
    }
    bool erase(const key_type & key)
    {
        return m_bridge.getHashTable().erase(key);
    }
    size_type size() const
    {
        return m_bridge.getHashTable().size();
    }
    size_type capacity() const
    {
        return m_bridge.getHashTable().capacity();
    }

private:
    bridge_type m_bridge;
};

}

#endif /* INCLUDE_HASH_SET_HPP_ */
