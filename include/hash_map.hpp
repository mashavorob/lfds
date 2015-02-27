/*
 * hash_map
 *
 *  Created on: Jan 28, 2015
 *      Author: masha
 */

#ifndef INCLUDE_HASH_MAP_HPP_
#define INCLUDE_HASH_MAP_HPP_

#include "hash_table.hpp"
#include "hash_table_integral_key.hpp"
#include "hash_table_integral_pair.hpp"

#include <functional>
#include <type_traits>

namespace lfds
{

namespace
{

template<class HashTable>
class hash_map_bridge
{
public:
    typedef hash_map_bridge<HashTable> this_type;

    typedef HashTable hash_table_type;

    typedef typename hash_table_type::key_type key_type;
    typedef typename hash_table_type::value_type value_type;
    typedef typename hash_table_type::size_type size_type;
    typedef typename hash_table_type::key_allocator_type key_allocator_type;
    typedef typename hash_table_type::value_allocator_type value_allocator_type;
    typedef typename hash_table_type::node_allocator_type node_allocator_type;

public:
    hash_map_bridge(size_type initialCapacity) :
            m_key_allocator(), m_value_allocator(), m_node_allocator(), m_hash_table(
                    initialCapacity, m_key_allocator, m_value_allocator,
                    m_node_allocator)
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
    hash_map_bridge(const this_type&) = delete;
    this_type& operator=(const this_type&) = delete;

private:
    key_allocator_type m_key_allocator;
    value_allocator_type m_value_allocator;
    node_allocator_type m_node_allocator;
    hash_table_type m_hash_table;
};

template<class HashTable>
class hash_map_bridge_integral_key
{
public:
    typedef hash_map_bridge_integral_key<HashTable> this_type;

    typedef HashTable hash_table_type;

    typedef typename hash_table_type::key_type key_type;
    typedef typename hash_table_type::value_type value_type;
    typedef typename hash_table_type::size_type size_type;
    typedef typename hash_table_type::value_allocator_type value_allocator_type;
    typedef typename hash_table_type::node_allocator_type node_allocator_type;

public:
    hash_map_bridge_integral_key(size_type initialCapacity) :
            m_value_allocator(), m_node_allocator(), m_hash_table(
                    initialCapacity, m_value_allocator, m_node_allocator)
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
    hash_map_bridge_integral_key(const this_type&) = delete;
    this_type& operator=(const this_type&) = delete;

private:
    value_allocator_type m_value_allocator;
    node_allocator_type m_node_allocator;
    hash_table_type m_hash_table;
};

template<class HashTable>
class hash_map_bridge_integral_pair
{
public:
    typedef hash_map_bridge_integral_pair<HashTable> this_type;

    typedef HashTable hash_table_type;

    typedef typename hash_table_type::key_type key_type;
    typedef typename hash_table_type::value_type value_type;
    typedef typename hash_table_type::size_type size_type;
    typedef typename hash_table_type::node_allocator_type node_allocator_type;

public:
    hash_map_bridge_integral_pair(size_type initialCapacity) :
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
    hash_map_bridge_integral_pair(const this_type&) = delete;
    this_type& operator=(const this_type&) = delete;

private:
    node_allocator_type m_node_allocator;
    hash_table_type m_hash_table;
};

template<class Key, class Value>
struct dummy_hash_tuple
{
    Key m_key;
    Value m_value;
    int8_t m_dummy;
};

template<class Key, class Value>
struct is_interal_pair
{
    static const bool value = std::is_integral<Key>::value
            && std::is_integral<Value>::value
            && sizeof(dummy_hash_tuple<Key, Value> ) <= 16;
};

template<class Key, class Value, class Hash = std::hash<Key>,
        class Pred = std::equal_to<Key>,
        class Allocator = std::allocator<Value>, bool =
                std::is_integral<Key>::value, bool =
                is_interal_pair<Key, Value>::value>
struct hash_table_traits;

template<class Key, class Value, class Hash, class Pred, class Allocator>
struct hash_table_traits<Key, Value, Hash, Pred, Allocator, false, false>
{
    typedef lfds::hash_table<Key, Value, Hash, Pred, Allocator> hash_table_type;
    typedef hash_map_bridge<hash_table_type> hash_table_wrapper_type;
};

template<class Key, class Value, class Hash, class Pred, class Allocator>
struct hash_table_traits<Key, Value, Hash, Pred, Allocator, true, false>
{
    typedef lfds::hash_table_integral_key<Key, Value, Hash, Pred, Allocator> hash_table_type;
    typedef hash_map_bridge_integral_key<hash_table_type> hash_table_wrapper_type;
};

template<class Key, class Value, class Hash, class Pred, class Allocator>
struct hash_table_traits<Key, Value, Hash, Pred, Allocator, true, true>
{
    // TODO: implement
    typedef lfds::hash_table_integral_pair<Key, Value, Hash, Pred, Allocator> hash_table_type;
    typedef hash_map_bridge_integral_pair<hash_table_type> hash_table_wrapper_type;
};

}

template<class Key, class Value, class Hash = std::hash<Key>,
        class Pred = std::equal_to<Key>, class Allocator = std::allocator<Value> >
class hash_map
{
public:

    typedef hash_map<Key, Value, Hash, Pred, Allocator> this_type;
    typedef hash_table_traits<Key, Value, Hash, Pred, Allocator> hash_table_traits_type;

    typedef typename hash_table_traits_type::hash_table_wrapper_type hash_table_wrapper_type;
    typedef typename hash_table_traits_type::hash_table_type hash_table_type;
    typedef typename hash_table_type::key_type key_type;
    typedef typename hash_table_type::value_type value_type;
    typedef typename hash_table_type::size_type size_type;

    static constexpr bool INTEGRAL_KEY = hash_table_type::INTEGRAL_KEY;
    static constexpr bool INTEGRAL_KEYVALUE = hash_table_type::INTEGRAL_KEYVALUE;
private:
    hash_map(const this_type&) = delete;
    this_type& operator=(const this_type&) = delete;

public:
    hash_map(const size_type initialCapacity = 0) :
            m_hash_table_wrapper(initialCapacity)
    {

    }

    bool find(const key_type & key, value_type & value) const
    {
        return m_hash_table_wrapper.getHashTable().find(key, value);
    }
    template<class ... Args>
    bool insert(const key_type & key, Args&&... val)
    {
        return m_hash_table_wrapper.getHashTable().insert(key,
                std::forward<Args>(val)...);
    }
    bool erase(const key_type & key)
    {
        return m_hash_table_wrapper.getHashTable().erase(key);
    }
    size_type size() const
    {
        return m_hash_table_wrapper.getHashTable().size();
    }
    size_type capacity() const
    {
        return m_hash_table_wrapper.getHashTable().capacity();
    }

private:
    hash_table_wrapper_type m_hash_table_wrapper;
};

}

#endif /* INCLUDE_HASH_MAP_HPP_ */
