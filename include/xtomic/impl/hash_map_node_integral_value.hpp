/*
 * hash_map_node_integral_pair_16b.hpp
 *
 *  Created on: Apr 20, 2015
 *      Author: masha
 */

#ifndef INCLUDE_HASH_MAP_NODE_INTEGRAL_VALUE_HPP_
#define INCLUDE_HASH_MAP_NODE_INTEGRAL_VALUE_HPP_

#include "cas.hpp"
#include <xtomic/xtomic.hpp>
#include <xtomic/aux/inttypes.hpp>

#include <cstddef>

namespace xtomic
{

template<typename Value>
struct align_4_cas16 mapped_value
{
    // states of the item (m_state field)
    enum
    {
        unused,     // initial state
        pending,// hash is valid, key & value are being constructed
        touched,// hash & key are valid
        allocated,// hash & key & value are valid
    };

    typedef mapped_value<Value> this_type;
    typedef Value value_type;
    typedef typename get_int_by_size<sizeof(value_type)>::type state_type;

    mapped_value() :
    m_state(unused), m_value(0)
    {
    }
    mapped_value(const state_type state, const value_type value) :
    m_state(state), m_value(value)
    {
    }
    mapped_value(const this_type & other) :
    m_state(other.m_state), m_value(other.m_value)
    {
    }
    mapped_value(const volatile this_type & other) :
    m_state(other.m_state), m_value(other.m_value)
    {
    }
    this_type & operator=(const this_type & other)
    {
        m_state = other.m_state;
        m_value = other.m_value;
        return *this;
    }
    this_type & operator=(const volatile this_type & other)
    {
        m_state = other.m_state;
        m_value = other.m_value;
        return *this;
    }
    bool atomic_cas(const this_type & expected, const this_type & val)
    {
        return xtomic::atomic_cas(*this, expected, val);
    }
    bool atomic_cas(const state_type expected, const state_type val)
    {
        return xtomic::atomic_cas(m_state, expected, val);
    }

public:
    // normal life cycle of an item in the hash table is:
    //
    //  <after initialization>
    // 1. unused  - initial state. code, key and value have not been initialized
    //
    //  <first insert is in progress>
    // 2. pending - insert operation is in progress.
    //              hash is initialized with the key's hash
    //              key and value are being constructed
    //
    //  <first insert finished>
    // 3. allocated - ready to use, all fields are valid
    //
    //  <first erase finished>
    // 4. touched - erase operation finished.
    //              code and key are valid
    //              value is destroyed
    //
    //  <insert finished>
    // 5. allocated - exactly same as 3.
    volatile state_type m_state;
    volatile value_type m_value;
};

template<typename Key, typename Value>
class hash_node_integral_value
{
public:
    typedef Key key_type;
    typedef Value mapped_type;
    typedef std::size_t hash_type;
    typedef mapped_value<mapped_type> value_item_type;
    typedef typename value_item_type::state_type state_type;

    typedef hash_node_integral_value<key_type, mapped_type> this_type;

private:
    hash_node_integral_value(const this_type&); // = delete;
    this_type& operator=(const this_type&); // = delete;
public:
    hash_node_integral_value() :
            m_value()
    {

    }
    key_type* getKey()
    {
        return reinterpret_cast<key_type*>(m_key);
    }
    const key_type* getKey() const
    {
        return reinterpret_cast<const key_type*>(m_key);
    }
    value_item_type& getValue()
    {
        return m_value;
    }
    const value_item_type& getValue() const
    {
        return m_value;
    }
private:
    char m_key[sizeof(key_type)] align_as(key_type);
    value_item_type m_value;
};

}

#endif /* INCLUDE_HASH_MAP_NODE_INTEGRAL_VALUE_HPP_ */
