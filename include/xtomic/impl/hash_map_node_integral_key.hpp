/*
 * hash_node_integral_key.hpp
 *
 *  Created on: Feb 7, 2015
 *      Author: masha
 */

#ifndef INCLUDE_HASH_MAP_NODE_INTEGRAL_KEY_HPP_
#define INCLUDE_HASH_MAP_NODE_INTEGRAL_KEY_HPP_

#include <xtomic/quantum.hpp>
#include "meta_utils.hpp"

namespace xtomic
{

// Eclipse Luna does not recognize alignas keyword regardless of any settings
// It seems Eclipse's developers do not admit the bug so the only way
// to avoid annoying error marks these macro are used
template<typename Key>
struct align_by(2*sizeof(Key)) key_item
{
    typedef key_item<Key> this_type;
    typedef typename get_int_by_size<sizeof(Key)>::type state_type;
    // states of the item (m_state field)
    enum
    {
        unused,     // initial state
        pending,// key is valid, value is being constructed/deleted
        touched,// key is valid
        allocated,// key & value are valid
    };

    // normal life cycle of an item in the hash table is:
    //
    //  <after initialization>
    // 1. unused  - initial state. Fields key and value have not been initialized
    //
    //  <insert is in progress>
    // 2. pending - insert operation is in progress.
    //              key is initialized
    //              value is being constructed
    //
    //  <first insert finished>
    // 3. allocated - ready to use, all fields are valid
    //
    //  <first erase is in progress>
    // 4. pending - erase operation is in progress.
    //              key is valid
    //              value is being destroyed
    //
    //  <first erase finished>
    // 5. touched - erase operation finished.
    //              key is valid
    //              value is destroyed
    //
    //  <subsequent insert is in progress>
    // 6. pending - insert operation is in progress.
    //              key is valid
    //              value is being constructed
    //
    //  <insert finished>
    // 7. allocated - exactly same as 3.

    key_item() :
    m_key(), m_state()
    {

    }
    key_item(const Key key, const state_type state) :
    m_key(key), m_state(state)
    {

    }
    key_item(const this_type & other) :
    m_key(other.m_key), m_state(other.m_state)
    {

    }
    key_item(const volatile this_type & other) :
    m_key(other.m_key), m_state(other.m_state)
    {

    }

    Key m_key;
    state_type m_state;
};

template<typename Key, typename Value>
class hash_node_integral_key
{
public:
    typedef Key key_type;
    typedef Value mapped_type;
    typedef hash_node_integral_key<key_type, mapped_type> this_class;
    typedef key_item<Key> key_item_type;
    typedef typename key_item_type::state_type state_type;

private:
    hash_node_integral_key(const this_class&); // = delete;
    this_class& operator=(const this_class&); // = delete;
public:
    hash_node_integral_key() :
            m_key(),
            m_refCount(0)
    {

    }
    const key_item_type getKey() const
    {
        return m_key;
    }
    void setKey(const key_type keyValue, const state_type stateValue)
    {
        m_key.m_key = keyValue;
        m_key.m_state = stateValue;
    }
    state_type getState() const
    {
        return m_key.m_state;
    }
    void setState(const state_type val)
    {
        thread_fence(barriers::release);
        m_key.m_state = val;
    }
    mapped_type* getValue()
    {
        return reinterpret_cast<mapped_type*>(m_value);
    }
    const mapped_type* getValue() const
    {
        return reinterpret_cast<const mapped_type*>(m_value);
    }
    bool atomic_cas(const key_item_type & expected,
                    const key_item_type & newkey)
    {
        return xtomic::atomic_cas(m_key, expected, newkey);
    }
    void addRef() const
    {
        ++m_refCount;
    }
    void release() const
    {
        --m_refCount;
    }
    void waitForRelease() const
    {
        while (m_refCount.load(barriers::relaxed))
            ;
    }

private:
    volatile key_item_type m_key;char m_value[sizeof(mapped_type)] align_as(mapped_type);
    mutable xtomic::quantum<int> m_refCount;
};

}

#endif /* INCLUDE_HASH_MAP_NODE_INTEGRAL_KEY_HPP_ */
