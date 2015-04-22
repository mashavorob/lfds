/*
 * hash_set_node.hpp
 *
 *  Created on: Feb 13, 2015
 *      Author: masha
 */

#ifndef INCLUDE_HASH_SET_NODE_HPP_
#define INCLUDE_HASH_SET_NODE_HPP_

#include "cas.hpp"
#include "inttypes.hpp"

namespace lfds
{

// Eclipse Luna does not recognize alignas keyword regardless of any settings
// It seems Eclipse's developers do not admit the bug so the only way
// to avoid annoying error marks these macro are used

struct __attribute__((aligned(sizeof(void*)*2))) hash_set_item
{
    // states of the item (m_state field)
    enum {
        unused,     // initial state
        pending,    // hash is valid, key is being constructed
        touched,    // hash & key are valid
        allocated,  // hash & key are valid
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
    //              key is being constructed
    //
    //  <first insert finished>
    // 3. allocated - ready to use, all fields are valid
    //
    //  <first erase finished, erase is atomic>
    // 4. touched - erase operation finished.
    //              code and key are valid
    //
    //  <subsequent insert finished, the operation is atomic>
    // 5. allocated - exactly same as 3.

    std::size_t m_state;

    hash_set_item() :
            m_hash(0), m_state(unused)
    {
    }
    hash_set_item(const std::size_t code, const std::size_t flags) :
            m_hash(code), m_state(flags)
    {
    }
    hash_set_item(const hash_set_item & other) :
            m_hash(other.m_hash), m_state(other.m_state)
    {
    }
    hash_set_item(const volatile hash_set_item & other) :
            m_hash(other.m_hash), m_state(other.m_state)
    {
    }
    hash_set_item & operator=(const hash_set_item & other)
    {
        m_hash = other.m_hash;
        m_state = other.m_state;
        return *this;
    }
    hash_set_item & operator=(const volatile hash_set_item & other)
    {
        m_hash = other.m_hash;
        m_state = other.m_state;
        return *this;
    }
    bool operator==(const hash_set_item& other) const
    {
        return m_hash == other.m_hash && m_state == other.m_state;
    }
};

template<class Key>
class hash_set_node
{
public:
    typedef Key key_type;
    typedef hash_set_node<key_type> this_class;
    typedef hash_set_item hash_item_type;

private:
    hash_set_node(const this_class&); // = delete;
    this_class& operator=(const this_class&); // = delete;
public:
    hash_set_node() : m_hash()
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
public:
    volatile hash_item_type m_hash;

private:
    char m_key[sizeof(key_type)];
};

}

#endif /* INCLUDE_HASH_SET_NODE_HPP_ */
