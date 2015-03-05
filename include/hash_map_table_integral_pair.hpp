/*
 * hash_table_integral_pair.hpp
 *
 *  Created on: Feb 11, 2015
 *      Author: masha
 */

#ifndef INCLUDE_HASH_MAP_TABLE_INTEGRAL_PAIR_HPP_
#define INCLUDE_HASH_MAP_TABLE_INTEGRAL_PAIR_HPP_

#include "hash_map_node_integral_pair.hpp"
#include "cas.hpp"

#include <cassert>
#include <atomic>
#include <functional>

namespace lfds
{
template<class Key, class Value, class Hash, class Pred, class Allocator>
class hash_map_table_integral_pair
{
public:
    typedef hash_map_table_integral_pair<Key, Value, Hash, Pred, Allocator> this_type;

    typedef Key key_type;
    typedef Value mapped_type;
    typedef Hash hash_func_type;
    typedef Pred equal_predicate_type;
    typedef Allocator allocator_type;

    typedef hash_node_integral_pair<key_type, mapped_type> node_type;
    typedef hash_data_table<node_type> table_type;
    typedef typename table_type::size_type size_type;

    static constexpr bool INTEGRAL_KEY = true;
    static constexpr bool INTEGRAL_KEYVALUE = true;

private:
    typedef typename node_type::state_type state_type;

public:
    hash_map_table_integral_pair()
    {
    }
    bool find_impl(const table_type& raw_table, const key_type key, mapped_type & value) const
    {
        hash_func_type hash_func;
        equal_predicate_type eq_func;

        const std::size_t hash = hash_func(key);

        const node_type* table = raw_table.m_table;
        const size_type capacity = raw_table.m_capacity;

        for (size_type i = hash % capacity;; ++i)
        {
            if ( i == capacity )
            {
                i = 0;
            }
            const node_type node = table[i];

            switch (node.m_data.m_state)
            {
            case node_type::unused:
                // the search finished
                return false;
            case node_type::touched:
                if (eq_func(key, node.m_data.m_key))
                {
                    // the item was erased recently
                    return false;
                }
                break;
            case node_type::allocated:
                if (eq_func(key, node.m_data.m_key))
                {
                    value = node.m_data.m_value;
                    return true;
                }
                break;
            default:
                assert(false);
            }
        }

        return false;
    }
    bool insert_impl(table_type& raw_table, const key_type key, const mapped_type val)
    {
        hash_func_type hash_func;
        equal_predicate_type eq_func;

        const std::size_t hash = hash_func(key);

        node_type* table = raw_table.m_table;
        const size_type capacity = raw_table.m_capacity;

        for (size_type i = hash % capacity;; ++i)
        {
            if ( i == capacity )
            {
                i = 0;
            }
            const node_type node = table[i];

            switch (node.m_data.m_state)
            {
            case node_type::unused:
            {
                // the slot is empty so try to use it
                const node_type new_node =
                { node_type::allocated, key, val };

                if (table[i].atomic_cas(node, new_node))
                {
                    ++raw_table.m_size;
                    ++raw_table.m_used;
                    return true;
                }
                // the slot has been updated by other thread so we have to start all over again
                continue;
            }
            case node_type::allocated:
                if (eq_func(key, node.m_data.m_key))
                {
                    // the item is allocated or concurrent insert/delete operation is in progress
                    return false;
                }
                break;
            case node_type::touched:
                if (eq_func(key, node.m_data.m_key))
                {
                    const node_type new_node =
                    { node_type::allocated, key, val };

                    if (table[i].atomic_cas(node, new_node))
                    {
                        ++raw_table.m_size;
                        return true;
                    }
                    // the slot has been updated by other thread
                    // so at least one concurrent insert operation with the same key took place
                    return false;
                }
                break;
            default:
                assert(false);
            }
        }
        throw std::bad_alloc();
        return false;
    }
    bool erase_impl(table_type& raw_table, const key_type key)
    {
        hash_func_type hash_func;
        equal_predicate_type eq_func;

        const std::size_t hash = hash_func(key);

        node_type* table = raw_table.m_table;
        const size_type capacity = raw_table.m_capacity;

        for (size_type i = hash % capacity;; ++i)
        {
            if ( i == capacity )
            {
                i = 0;
            }
            const node_type node = table[i];

            switch (node.m_data.m_state)
            {
            case node_type::unused:
                // the search finished
                return false;
            case node_type::touched:
                if (eq_func(key, node.m_data.m_key))
                {
                    // the item was erased recently
                    return false;
                }
                break;
            case node_type::allocated:
                if (eq_func(key, node.m_data.m_key))
                {

                    volatile state_type & state = table[i].m_data.m_state;
                    static const state_type allocated = node_type::allocated;
                    static const state_type touched = node_type::touched;

                    if (atomic_cas(state, allocated, touched))
                    {
                        --raw_table.m_size;
                        return true;
                    }
                    // the item found but it is being erased in another thread;
                    return false;
                }
                break;
            default:
                assert(false);
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

            if (node_type::allocated == node.m_data.m_state)
            {
                insert_unique_key(dst, node);
            }
        }
    }

    // simplified form of insert()
    // the function assumes:
    //    * exclusive access to the container
    //    * new key is unique
    //    * table has enough capacity to insert specified element
    void insert_unique_key(table_type& dst, const node_type& new_node)
    {
        hash_func_type hash_func;
        const size_type capacity = dst.m_capacity;
        const size_type hash = hash_func(new_node.m_data.m_key);

        for (size_type i = hash % capacity;; ++i)
        {
            if ( i == capacity )
            {
                i = 0;
            }
            node_type& node = dst.m_table[i];
            if (node_type::unused == node.m_data.m_state)
            {
                node = new_node;

                ++dst.m_size;
                ++dst.m_used;
                break;
            }
        }
    }
};

}

#endif /* INCLUDE_HASH_MAP_TABLE_INTEGRAL_PAIR_HPP_ */
