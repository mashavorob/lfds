/*
 * hash_node_integral_pair.hpp
 *
 *  Created on: Feb 10, 2015
 *      Author: masha
 */

#ifndef INCLUDE_HASH_MAP_NODE_INTEGRAL_PAIR_HPP_
#define INCLUDE_HASH_MAP_NODE_INTEGRAL_PAIR_HPP_

#include "meta_utils.hpp"
#include "paddings.hpp"
#include "cas.hpp"

#pragma pack(push)
#pragma pack(1)

namespace lfds
{

namespace
{

template<int KeySize, int ValueSize, int StateSize, bool = is_greater<KeySize,
        ValueSize>::value, bool = is_greater<ValueSize, KeySize>::value>
struct get_padding_size_by_size;

// KeySize == ValueSize
template<int KeySize, int ValueSize, int StateSize>
struct get_padding_size_by_size<KeySize, ValueSize, StateSize, false, false>
{
    const static int value = 0;
};

template<int KeySize, int ValueSize, int StateSize>
struct get_padding_size_by_size<KeySize, ValueSize, StateSize, true, false>
{
    const static int value = KeySize - ValueSize - StateSize;
};

template<int KeySize, int ValueSize, int StateSize>
struct get_padding_size_by_size<KeySize, ValueSize, StateSize, false, true>
{
    const static int value = ValueSize - KeySize - StateSize;
};

template<class Key, class Value, class State>
struct get_padding_size
{
    const static int value = get_padding_size_by_size<sizeof(Key),
            sizeof(Value), sizeof(State)>::value;
};

template<int KeySize, int ValueSize, bool =
        is_greater<KeySize, ValueSize>::value, bool = is_greater<ValueSize,
        KeySize>::value>
struct get_state_size;

template<int KeySize>
struct get_state_size<KeySize, 0, true, false>
{
    static const int value = KeySize;
};

template<int KeySize, int ValueSize>
struct get_state_size<KeySize, ValueSize, false, false>
{
    static const int value = KeySize + ValueSize;
};

template<int KeySize, int ValueSize>
struct get_state_size<KeySize, ValueSize, true, false>
{
    static const int value = ValueSize;
};

template<int KeySize, int ValueSize>
struct get_state_size<KeySize, ValueSize, false, true>
{
    static const int value = KeySize;
};

template<class Key, class Value>
struct get_state_type
{
    static const int size = get_state_size<sizeof(Key), sizeof(Value)>::value;
    typedef typename get_int_by_size<size>::type type;
};

template<class Key, class Value, class State, bool PaddingAtTheEnd =
        !is_greater_size<Value, Key>::value, int PaddingSize = get_padding_size<
        Key, Value, State>::value>
struct integral_item_data_fields;

#define _DETAILED_CONSTRUCTOR_ 1

template<class Key, class Value, class State>
struct integral_item_data_fields<Key, Value, State, true, 0>
{
    typedef State state_type;

    Key m_key;
    Value m_value;
    State m_state;

#if _DETAILED_CONSTRUCTOR_
    integral_item_data_fields(const State state) :
            m_key(), m_value(state), m_state()
    {

    }
    integral_item_data_fields(const State state, const Key key) :
            m_key(key), m_value(), m_state(state)
    {

    }
#endif
    integral_item_data_fields(const State state, const Key key,
            const Value value) :
            m_key(key), m_value(value), m_state(state)
    {

    }
};

template<class Key, class Value, class State>
struct integral_item_data_fields<Key, Value, State, false, 0>
{
    typedef State state_type;

    Key m_key;
    State m_state;
    Value m_value;

#if _DETAILED_CONSTRUCTOR_
    integral_item_data_fields(const State state) :
            m_key(), m_state(state), m_value()
    {

    }
    integral_item_data_fields(const State state, const Key key) :
            m_key(key), m_state(state), m_value()
    {

    }
#endif
    integral_item_data_fields(const State state, const Key key,
            const Value value) :
            m_key(key), m_state(state), m_value(value)
    {

    }
};

template<class Key, class Value, class State, int PaddingSize>
struct integral_item_data_fields<Key, Value, State, true, PaddingSize>
{
    Key m_key;
    Value m_value;
    State m_state;
    padding_type<PaddingSize> m_padding;

#if _DETAILED_CONSTRUCTOR_
    integral_item_data_fields(const State state) :
            m_key(), m_value(), m_state(state), m_padding()
    {

    }
    integral_item_data_fields(const State state, const Key key) :
            m_key(key), m_value(), m_state(state), m_padding()
    {

    }
#endif
    integral_item_data_fields(const State state, const Key key,
            const Value value) :
            m_key(key), m_value(value), m_state(state), m_padding()
    {

    }
};

template<class Key, class Value, class State, int PaddingSize>
struct integral_item_data_fields<Key, Value, State, false, PaddingSize>
{
    Key m_key;
    State m_state;
    padding_type<PaddingSize> m_padding;
    Value m_value;

#if _DETAILED_CONSTRUCTOR_
    integral_item_data_fields(const State state) :
            m_key(), m_state(state), m_padding(), m_value()
    {

    }
    integral_item_data_fields(const State state, const Key key) :
            m_key(key), m_state(state), m_padding(), m_value()
    {

    }
#endif
    integral_item_data_fields(const State state, const Key key,
            const Value value) :
            m_key(key), m_state(state), m_padding(), m_value(value)
    {

    }
};

template<class Key, class Value>
struct hash_node_integral_pair
{
public:
    typedef hash_node_integral_pair<Key, Value> this_type;
    typedef Key key_type;
    typedef Value mapped_type;
    typedef typename get_state_type<key_type, mapped_type>::type state_type;
    typedef integral_item_data_fields<key_type, mapped_type, state_type> data_type;

public:
    enum
    {
        unused,     // initial state
        allocated,  // key & value are valid
        touched,    // key is valid
    };

    // normal life cycle of an item in the hash table is:
    //
    //  <after initialization>
    // 1. unused  - initial state. Fields key and value have not been initialized
    //
    //  <first insert finished, insertion is atomic operation>
    // 2. allocated - ready to use, all fields are valid
    //
    //  <first erase finished, erasing is atomic operation>
    // 3. touched - erase operation finished.
    //              key is valid
    //              value is destroyed
    //
    //  <insert finished>
    // 7. allocated - exactly same as 2.

public:
#if _DETAILED_CONSTRUCTOR_
    hash_node_integral_pair() :
            m_data(unused)
    {

    }
    hash_node_integral_pair(const state_type state, const key_type key) :
            m_data(state, key)
    {

    }
    hash_node_integral_pair(const state_type state, const key_type key,
            const mapped_type value) :
            m_data(state, key, value)
    {

    }

#else
    hash_node_integral_pair() :
    m_data(unused, key_type(), mapped_type())
    {   l

    }
    hash_node_integral_pair(const state_type state, const key_type key) :
    m_data(state, key, mapped_type())
    {

    }
    hash_node_integral_pair(const state_type state, const key_type key,
            const mapped_type value) :
    m_data(state, key, value)
    {

    }
#endif
    hash_node_integral_pair(const this_type & other) :
            m_data(other.m_data.m_state, other.m_data.m_key,
                    other.m_data.m_value)
    {

    }
    hash_node_integral_pair(const volatile this_type & other) :
            m_data(unused)
    {
        *this = other;
    }
    this_type & operator=(const this_type & other)
    {
        m_data.m_key = other.m_data.m_key;
        m_data.m_state = other.m_data.m_state;
        m_data.m_value = other.m_data.m_value;
        return *this;
    }
    this_type & operator=(const volatile this_type & other)
    {
        m_data.m_state = other.m_data.m_state;
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
#endif /* INCLUDE_HASH_MAP_NODE_INTEGRAL_PAIR_HPP_ */
