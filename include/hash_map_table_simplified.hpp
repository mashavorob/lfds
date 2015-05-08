/*
 * hash_map_table_simplified.hpp
 *
 *  Created on: May 7, 2015
 *      Author: masha
 */

#ifndef INCLUDE_HASH_MAP_TABLE_SIMPLIFIED_HPP_
#define INCLUDE_HASH_MAP_TABLE_SIMPLIFIED_HPP_

#include "hash_map_node_simplified.hpp"
#include "cas.hpp"

#include <cassert>
#include <functional>
#include <vector>

namespace lfds
{
template<typename Key, typename Value, typename Hash, typename Pred,
        typename Allocator>
class hash_map_table_simplified
{
public:
    typedef hash_map_table_simplified<Key, Value, Hash, Pred, Allocator> this_type;

    typedef Key key_type;
    typedef Value mapped_type;
    typedef Hash hash_func_type;
    typedef Pred equal_predicate_type;
    typedef Allocator allocator_type;

    typedef hash_node_simplified<key_type, mapped_type> node_type;
    typedef hash_data_table<node_type> table_type;
    typedef typename table_type::size_type size_type;

    typedef std::pair<key_type, mapped_type> value_type;
    typedef std::vector<value_type> snapshot_type;

    static constexpr bool INTEGRAL_KEY = true;
    static constexpr bool INTEGRAL_VALUE = true;
    static constexpr bool INTEGRAL_KEYVALUE = true;

public:
    hash_map_table_simplified()
    {
    }
    void getSnapshot_imp(const table_type& raw_table,
                         snapshot_type & snapshot) const
    {
        const node_type* table = raw_table.m_table;
        const size_type capacity = raw_table.m_capacity;

        for (size_type i = 0; i < capacity; ++i)
        {
            const node_type& node = table[i];

            if (node.m_data.m_value != s_defaultValue)
            {
                snapshot.push_back(
                        value_type(node.m_data.m_key, node.m_data.m_value));
            }
        }
    }
    bool find_impl(const table_type& raw_table,
                   const key_type key,
                   mapped_type & value) const
    {
        const std::size_t hash = m_hash_func(key);

        const node_type* table = raw_table.m_table;
        const size_type capacity = raw_table.m_capacity;

        for (size_type i = hash % capacity;; ++i)
        {
            if (i == capacity)
            {
                i = 0;
            }
            const node_type node = table[i];
            if (m_eq_func(node.m_data.m_key, s_defaultKey))
            {
                return false;
            }
            if (m_eq_func(key, node.m_data.m_key))
            {
                value = node.m_data.m_value;
                return value != s_defaultValue;
            }
        }

        return false;
    }
    bool insert_impl(table_type& raw_table,
                     const key_type key,
                     const bool updateIfExists,
                     const mapped_type val)
    {
        if (m_eq_func(key, s_defaultKey))
        {
            return false;
        }
        if (val == s_defaultValue)
        {
            return false;
        }

        const std::size_t hash = m_hash_func(key);

        node_type* table = raw_table.m_table;
        const size_type capacity = raw_table.m_capacity;

        for (size_type i = hash % capacity;;)
        {
            if (i == capacity)
            {
                i = 0;
            }
            const node_type node = table[i];

            if (m_eq_func(node.m_data.m_key, s_defaultKey))
            {
                // the slot is empty so try to use it
                const node_type new_node(key, val);

                if (table[i].atomic_cas(node, new_node))
                {
                    ++raw_table.m_size;
                    ++raw_table.m_used;
                    return true;
                }
                continue;
            }
            else if (m_eq_func(key, node.m_data.m_key))
            {
                bool itemIsEmpty = (s_defaultValue == node.m_data.m_value);
                if (!updateIfExists && !itemIsEmpty)
                {
                    return false;
                }
                const node_type new_node(key, val);
                if (table[i].atomic_cas(node, new_node))
                {
                    if (itemIsEmpty)
                    {
                        ++raw_table.m_size;
                    }
                    return true;
                }
                continue;
            }
            ++i;
        }
        throw std::bad_alloc();
        return false;
    }
    bool erase_impl(table_type& raw_table, const key_type key)
    {
        const std::size_t hash = m_hash_func(key);

        node_type* table = raw_table.m_table;
        const size_type capacity = raw_table.m_capacity;

        for (size_type i = hash % capacity;; ++i)
        {
            if (i == capacity)
            {
                i = 0;
            }
            const node_type node = table[i];
            if (m_eq_func(node.m_data.m_key, s_defaultKey))
            {
                return false;
            }
            else if (m_eq_func(key, node.m_data.m_key))
            {
                bool emptyItem = s_defaultValue == node.m_data.m_value;
                if (emptyItem)
                {
                    return false;
                }
                const node_type new_node(key, s_defaultValue);
                if (table[i].atomic_cas(node, new_node))
                {
                    --raw_table.m_size;
                    return true;
                }
                return false;
            }
        }
        return false;
    }
    void destroyNode_impl(node_type & node)
    {
    }
    void rehash_impl(const table_type& src, table_type& dst)
    {
        for (size_type i = 0; i < src.m_capacity; ++i)
        {
            // remove volatile because of e
            const node_type& node = src.m_table[i];

            if (s_defaultValue != node.m_data.m_value)
            {
                insertUniqueKey(dst, node);
            }
        }
    }

    // simplified form of insert()
    // the function assumes:
    //    * exclusive access to the container
    //    * new key is unique
    //    * table has enough capacity to insert the specified element
    void insertUniqueKey(table_type& dst, const node_type& new_node)
    {
        const size_type capacity = dst.m_capacity;
        const size_type hash = m_hash_func(new_node.m_data.m_key);

        for (size_type i = hash % capacity;; ++i)
        {
            if (i == capacity)
            {
                i = 0;
            }
            node_type& node = dst.m_table[i];
            if (m_eq_func(s_defaultKey, node.m_data.m_key))
            {
                node = new_node;

                ++dst.m_size;
                ++dst.m_used;
                break;
            }
        }
    }
private:
    static const key_type s_defaultKey;
    static const mapped_type s_defaultValue;
    hash_func_type m_hash_func;
    equal_predicate_type m_eq_func;
};

template<typename Key, typename Value, typename Hash, typename Pred,
        typename Allocator>
const Key hash_map_table_simplified<Key, Value, Hash, Pred, Allocator>::s_defaultKey =
        Key();

template<typename Key, typename Value, typename Hash, typename Pred,
        typename Allocator>
const Value hash_map_table_simplified<Key, Value, Hash, Pred, Allocator>::s_defaultValue =
        Value();

}
#endif /* INCLUDE_HASH_MAP_TABLE_SIMPLIFIED_HPP_ */
