/*
 * hash_map_node_simplified.hpp
 *
 *  Created on: May 7, 2015
 *      Author: masha
 */

#ifndef INCLUDE_HASH_MAP_NODE_SIMPLIFIED_HPP_
#define INCLUDE_HASH_MAP_NODE_SIMPLIFIED_HPP_

#include "meta_utils.hpp"
#include "paddings.hpp"
#include "hash_map_node_integral_pair.hpp"

#pragma pack(push)
#pragma pack(1)

namespace lfds
{

namespace
{

template<typename Key, typename Value>
struct get_simple_padding_size
{
    const static int value = get_padding_size_by_size<sizeof(Key),
            sizeof(Value), 0>::value;
};

template<typename Key, typename Value, bool PaddingAtTheEnd = is_greater_size<
        Key, Value>::value, int PaddingSize =
        get_simple_padding_size<Key, Value>::value>
struct simple_item_data_fields;

template<typename Key, typename Value>
struct simple_item_data_fields<Key, Value, false, 0>
{
    Key m_key;
    Value m_value;

#if _DETAILED_CONSTRUCTOR_
    simple_item_data_fields() :
            m_key(),
            m_value()
    {

    }
    simple_item_data_fields(const Key key) :
            m_key(key),
            m_value()
    {

    }
#endif
    simple_item_data_fields(const Key key, const Value value) :
            m_key(key),
            m_value(value)
    {

    }
};

template<typename Key, typename Value, int PaddingSize>
struct simple_item_data_fields<Key, Value, true, PaddingSize>
{
    Key m_key;
    Value m_value;
    padding_type<PaddingSize> m_padding;

#if _DETAILED_CONSTRUCTOR_
    simple_item_data_fields() :
            m_key(),
            m_value(),
            m_padding()
    {

    }
    simple_item_data_fields(const Key key) :
            m_key(key),
            m_value(),
            m_padding()
    {

    }
#endif
    simple_item_data_fields(const Key key, const Value value) :
            m_key(key),
            m_value(value),
            m_padding()
    {

    }
};

template<typename Key, typename Value, int PaddingSize>
struct simple_item_data_fields<Key, Value, false, PaddingSize>
{
    Key m_key;
    padding_type<PaddingSize> m_padding;
    Value m_value;

#if _DETAILED_CONSTRUCTOR_
    simple_item_data_fields() :
            m_key(),
            m_padding(),
            m_value()
    {

    }
    simple_item_data_fields(const Key key) :
            m_key(key),
            m_padding(),
            m_value()
    {

    }
#endif
    simple_item_data_fields(const Key key, const Value value) :
            m_key(key),
            m_padding(),
            m_value(value)
    {

    }
};

template<typename Key, typename Value>
struct hash_node_simplified
{
public:
    typedef hash_node_simplified<Key, Value> this_type;
    typedef Key key_type;
    typedef Value mapped_type;
    typedef simple_item_data_fields<key_type, mapped_type> data_type;

public:

    // normal life cycle of an item in the hash table is:
    //
    //  <after initialization>
    // 1. key == key_type(), value == value_type()
    //
    //  <first insert finished, insertion is atomic operation>
    // 2. key != key_type(), value != value_type() - ready to use, all fields are valid
    //
    //  <first erase finished, erasing is atomic operation>
    // 3. key != key_type(), value == value_type() - erase operation finished.
    //              key is valid
    //
    //  <insert finished>
    // 7. exactly same as 2.

public:
#if _DETAILED_CONSTRUCTOR_
    hash_node_simplified() :
            m_data()
    {

    }
    hash_node_simplified(const key_type key) :
            m_data(key)
    {

    }
    hash_node_simplified(const key_type key, const mapped_type value) :
            m_data(key, value)
    {

    }

#else
    hash_node_simplified() :
    m_data(key_type(), mapped_type())
    {

    }
    hash_node_simplified(const key_type key) :
    m_data(state, key, mapped_type())
    {

    }
    hash_node_simplified(const key_type key,
            const mapped_type value) :
    m_data(state, key, value)
    {

    }
#endif
    hash_node_simplified(const this_type & other) :
    m_data(other.m_data.m_key,
            other.m_data.m_value)
    {

    }
    hash_node_simplified(const volatile this_type & other)
    {
        *this = other;
    }
    this_type & operator=(const this_type & other)
    {
        m_data.m_key = other.m_data.m_key;
        m_data.m_value = other.m_data.m_value;
        return *this;
    }
    this_type & operator=(const volatile this_type & other)
    {
        thread_fence(barriers::acquire);
        m_data.m_key = other.m_data.m_key;
        m_data.m_value = other.m_data.m_value;
        return *this;
    }
    bool atomic_cas(const this_type & expected, const this_type & val) volatile
    {
        return lfds::atomic_cas(m_data, expected.m_data, val.m_data);
    }
public:
    data_type m_data;
};

}
}

#pragma pack(pop)
#endif /* INCLUDE_HASH_MAP_NODE_SIMPLIFIED_HPP_ */
