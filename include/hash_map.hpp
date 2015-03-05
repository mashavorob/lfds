/*
 * hash_map
 *
 *  Created on: Jan 28, 2015
 *      Author: masha
 */

#ifndef INCLUDE_HASH_MAP_HPP_
#define INCLUDE_HASH_MAP_HPP_

#include "hash_table.hpp"
#include "hash_map_table_base.hpp"
#include "hash_table_integral_key.hpp"
#include "hash_table_integral_pair.hpp"

#include <functional>
#include <type_traits>

namespace lfds
{

//
// some meta classes to specify hash table implementation from Key and Value types
//
namespace
{

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
    typedef lfds::hash_table<Key, Value, Hash, Pred, Allocator> type;
};

template<class Key, class Value, class Hash, class Pred, class Allocator>
struct hash_table_traits<Key, Value, Hash, Pred, Allocator, true, false>
{
    typedef lfds::hash_table_integral_key<Key, Value, Hash, Pred, Allocator> type;
};

template<class Key, class Value, class Hash, class Pred, class Allocator>
struct hash_table_traits<Key, Value, Hash, Pred, Allocator, true, true>
{
    typedef lfds::hash_table_integral_pair<Key, Value, Hash, Pred, Allocator> type;
};

}

//
// Hash map
//
template<class Key, class Value, class Hash = std::hash<Key>,
        class Pred = std::equal_to<Key>, class Allocator = std::allocator<Value> >
class hash_map
{
public:

    typedef hash_map<Key, Value, Hash, Pred, Allocator> this_type;
    typedef hash_table_traits<Key, Value, Hash, Pred, Allocator> hash_table_traits_type;

    typedef typename hash_table_traits_type::type hash_table_type;
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
            m_hash_table(), m_hash_table_base(m_hash_table, initialCapacity)
    {

    }
    bool find(const key_type & key, value_type & value) const
    {
        return m_hash_table_base.find(key, value);
    }
    template<class ... Args>
    bool insert(const key_type & key, Args&&... val)
    {
        return m_hash_table_base.insert(key, std::forward<Args>(val)...);
    }
    bool erase(const key_type & key)
    {
        return m_hash_table_base.erase(key);
    }
    size_type size() const
    {
        return m_hash_table_base.size();
    }
    size_type capacity() const
    {
        return m_hash_table_base.capacity();
    }

private:
    typedef hash_map_table_base<hash_table_type> hash_table_base_type;

    hash_table_type m_hash_table;
    hash_table_base_type m_hash_table_base;
};

}

#endif /* INCLUDE_HASH_MAP_HPP_ */
