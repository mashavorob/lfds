/*
 * hash_map
 *
 *  Created on: Jan 28, 2015
 *      Author: masha
 */

#ifndef INCLUDE_HASH_MAP_HPP_
#define INCLUDE_HASH_MAP_HPP_

#include "hash_map_table.hpp"
#include "hash_map_table_integral_pair.hpp"
#include "hash_map_table_integral_value.hpp"
#include "hash_map_table_integral_key.hpp"
#include "hash_map_table_base.hpp"
#include "xtraits.hpp"
#include "xfunctional.hpp"
#include "cppbasics.hpp"

#include <functional>

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
    enum
    {
        value = is_integral<Key>::value && is_integral<Value>::value
                && (sizeof(dummy_hash_tuple<Key, Value> ) <= 16)
    };
};

template<class Key
        , class Value
        , class Hash = typename get_hash<Key>::type
        , class Pred = std::equal_to<Key>
        , class Allocator = std::allocator<Value>
        , bool IntegralKey = is_integral<Key>::value
        , bool IntegralValue = is_integral<Value>::value
        , bool IntegralKeyValue = is_interal_pair<Key, Value>::value>
struct hash_table_traits
{
    typedef lfds::hash_map_table<Key, Value, Hash, Pred, Allocator> type;
};

template<class Key, class Value, class Hash, class Pred, class Allocator, bool IntegralKey, bool IntegralValue>
struct hash_table_traits<Key, Value, Hash, Pred, Allocator, IntegralKey, IntegralValue, true>
{
    typedef lfds::hash_map_table_integral_pair<Key, Value, Hash, Pred, Allocator> type;
};

template<class Key, class Value, class Hash, class Pred, class Allocator, bool IntegralKey>
struct hash_table_traits<Key, Value, Hash, Pred, Allocator, IntegralKey, true, false>
{
    typedef lfds::hash_map_table_integral_value<Key, Value, Hash, Pred, Allocator> type;
};

template<class Key, class Value, class Hash, class Pred, class Allocator>
struct hash_table_traits<Key, Value, Hash, Pred, Allocator, true, false, false>
{
    typedef lfds::hash_map_table_integral_key<Key, Value, Hash, Pred, Allocator> type;
};

}

//
// Hash map
//
template<class Key, class Value, class Hash = typename get_hash<Key>::type,
        class Pred = std::equal_to<Key>, class Allocator = std::allocator<Value> >
class hash_map
{
public:

    typedef hash_map<Key, Value, Hash, Pred, Allocator> this_type;
    typedef hash_table_traits<Key, Value, Hash, Pred, Allocator> hash_table_traits_type;

    typedef typename hash_table_traits_type::type hash_table_type;
    typedef typename hash_table_type::key_type key_type;
    typedef typename hash_table_type::mapped_type mapped_type;
    typedef typename hash_table_type::size_type size_type;
    typedef typename hash_table_type::value_type value_type;
    typedef typename hash_table_type::snapshot_type snapshot_type;

    static constexpr bool INTEGRAL_KEY = hash_table_type::INTEGRAL_KEY;
    static constexpr bool INTEGRAL_VALUE = hash_table_type::INTEGRAL_VALUE;
    static constexpr bool INTEGRAL_KEYVALUE = hash_table_type::INTEGRAL_KEYVALUE;
private:
    hash_map(const this_type&); // = delete;
    this_type& operator=(const this_type&); // = delete;

public:
    hash_map(const size_type initialCapacity = 0) :
            m_hash_table(),
            m_hash_table_base(m_hash_table, initialCapacity)
    {

    }
    void getSnapshot(snapshot_type & snapshot) const
    {
        m_hash_table_base.getSnapshot(snapshot);
    }
    bool find(const key_type & key, mapped_type & value) const
    {
        return m_hash_table_base.find(key, value);
    }
#if LFDS_USE_CPP11
    template<class ... Args>
    bool insert(const key_type & key, Args&&... val)
#else
    bool insert(const key_type & key, const mapped_type& val)
#endif
    {
        return m_hash_table_base.insert(key, std_forward(Args, val));
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
