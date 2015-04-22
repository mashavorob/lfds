/*
 * hash_node.hpp
 *
 *  Created on: Jan 23, 2015
 *      Author: masha
 */

#ifndef INCLUDE_HASH_MAP_NODE_HPP_
#define INCLUDE_HASH_MAP_NODE_HPP_

#include "cas.hpp"
#include "xtomic.hpp"
#include "inttypes.hpp"

#include <cstddef>

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
    hash_item(const std::size_t code, const std::size_t flags) :
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
    hash_item & operator=(const hash_item & other)
    {
        m_hash = other.m_hash;
        m_state = other.m_state;
        return *this;
    }
    hash_item & operator=(const volatile hash_item & other)
    {
        m_hash = other.m_hash;
        m_state = other.m_state;
        return *this;
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
    typedef Value mapped_type;
    typedef hash_node<key_type, mapped_type> this_class;
    typedef hash_item hash_item_type;

private:
    hash_node(const this_class&); // = delete;
    this_class& operator=(const this_class&); // = delete;
public:
    hash_node() : m_hash(), m_refCount(0)
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
    mapped_type* getValue()
    {
        return reinterpret_cast<mapped_type*>(m_value);
    }
    const mapped_type* getValue() const
    {
        return reinterpret_cast<const mapped_type*>(m_value);
    }
    hash_item_type getHash() const
    {
        return m_hash;
    }
    std::size_t getState() const
    {
        return m_hash.m_state;
    }
    void setState(std::size_t state)
    {
        m_hash.m_state = state;
    }
    void setItem(std::size_t hash, std::size_t state)
    {
        m_hash.m_hash = hash;
        m_hash.m_state = state;
    }
    bool atomic_cas(const hash_item & expected, const hash_item & hash)
    {
        return lfds::atomic_cas(m_hash, expected, hash);
    }
    void addRef() const
    {
        m_refCount.fetch_add(1, barriers::release);
    }
    void release() const
    {
        m_refCount.fetch_sub(1, barriers::release);
    }
    void waitForRelease() const
    {
        while ( m_refCount.load(barriers::relaxed) ) ;
    }

private:
    char m_key[sizeof(key_type)] __attribute__((aligned(__alignof(key_type))));
    char m_value[sizeof(mapped_type)] __attribute__((aligned(__alignof(mapped_type))));
    volatile hash_item_type m_hash;
    mutable xtomic<int> m_refCount;
};

}

#endif /* INCLUDE_HASH_MAP_NODE_HPP_ */
