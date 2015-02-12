/*
 * hash_node.hpp
 *
 *  Created on: Jan 23, 2015
 *      Author: masha
 */

#ifndef INCLUDE_HASH_NODE_HPP_
#define INCLUDE_HASH_NODE_HPP_

#include "cas.hpp"
#include <atomic>
#include <cstdint>

namespace lfds
{

// Eclipse Luna does not recognize alignas keyword regardless of any settings
// It seems Eclipse's developers do not admit the bug so the only way
// to avoid annoying error marks these macro are used

struct __attribute__((aligned(sizeof(void*)*2))) hash_item
{
    // states of the item (m_state field)
    enum {
        unused,     // initial state
        pending,    // hash is valid, key & value are being constructed
        touched,    // hash & key are valid
        pending2,   // hash & key are valid, value is being constructed/deleted
        allocated,  // hash & key & value are valid
    };


    std::size_t m_hash;

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
    //  <first erase is in progress>
    // 4. pending2 - erase operation is in progress.
    //              code and key are valid
    //              value is being destroyed
    //
    //  <first erase finished>
    // 5. touched - erase operation finished.
    //              code and key are valid
    //              value is destroyed
    //
    //  <subsequent insert is in progress>
    // 6. pending2 - insert operation is in progress.
    //              code and key are valid
    //              value is being constructed
    //
    //  <insert finished>
    // 7. allocated - exactly same as 3.

    std::size_t m_state;

    hash_item() :
            m_hash(0), m_state(unused)
    {
    }
    hash_item(std::size_t code, std::size_t flags) :
            m_hash(code), m_state(flags)
    {
    }
    hash_item(const hash_item & other) :
            m_hash(other.m_hash), m_state(other.m_state)
    {
    }
    hash_item(const volatile hash_item & other) :
            m_hash(other.m_hash), m_state(other.m_state)
    {
    }

    bool operator==(const hash_item& other) const
    {
        return m_hash == other.m_hash && m_state == other.m_state;
    }
};


template<class Key, class Value>
class hash_node
{
public:
    typedef Key key_type;
    typedef Value value_type;
    typedef hash_node<key_type, value_type> this_class;
    typedef hash_item hash_item_type;

private:
    hash_node(const this_class&) = delete;
    this_class& operator=(const this_class&) = delete;
public:
    hash_node() : m_hash(), m_refCount(0)
    {

    }
    key_type* key()
    {
        return reinterpret_cast<key_type*>(m_key);
    }
    const key_type* key() const
    {
        return reinterpret_cast<const key_type*>(m_key);
    }
    value_type* value()
    {
        return reinterpret_cast<value_type*>(m_value);
    }
    const value_type* value() const
    {
        return reinterpret_cast<const value_type*>(m_value);
    }
    hash_item_type get_hash() const
    {
        return m_hash;
    }
    std::size_t get_state() const
    {
        return m_hash.m_state;
    }
    void set_state(std::size_t state)
    {
        m_hash.m_state = state;
    }
    void set_item(std::size_t hash, std::size_t state)
    {
        m_hash.m_hash = hash;
        m_hash.m_state = state;
    }
    bool atomic_cas_hash(const hash_item & expected, const hash_item & hash)
    {
        return lfds::atomic_cas(m_hash, expected, hash);
    }
    void add_ref() const
    {
        m_refCount.fetch_add(1, std::memory_order_acquire);
    }
    void release() const
    {
        m_refCount.fetch_sub(1, std::memory_order_release);
    }
    void wait_for_release() const
    {
        while ( m_refCount.load(std::memory_order_relaxed) ) ;
    }

private:
    char m_key[sizeof(key_type)];
    char m_value[sizeof(value_type)];
    volatile hash_item_type m_hash;
    mutable std::atomic<int> m_refCount;
};

}

#endif /* INCLUDE_HASH_NODE_HPP_ */
