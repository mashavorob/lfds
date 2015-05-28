/*
 * hash_set_table_integral_key.hpp
 *
 *  Created on: Feb 13, 2015
 *      Author: masha
 */

#ifndef INCLUDE_HASH_SET_TABLE_INTEGRAL_KEY_HPP_
#define INCLUDE_HASH_SET_TABLE_INTEGRAL_KEY_HPP_

#include "hash_set_integral_key.hpp"

#include <cassert>
#include <functional>
#include <vector>

namespace xtomic
{
template<typename Key, typename Hash, typename Pred, typename Allocator>
class hash_set_table_integral_key
{
public:
    typedef hash_set_table_integral_key<Key, Hash, Pred, Allocator> this_type;
    typedef Key key_type;
    typedef Hash hash_func_type;
    typedef Pred equal_predicate_type;
    typedef Allocator allocator_type;

    typedef hash_set_integral_key<key_type> node_type;
    typedef hash_data_table<node_type> table_type;

    typedef typename table_type::size_type size_type;
    typedef std::vector<key_type> snapshot_type;

    static constexpr bool INTEGRAL = true;

private:
    typedef typename node_type::state_type state_type;

public:
    hash_set_table_integral_key()
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

            if (node.m_state == node_type::allocated)
            {
                snapshot.push_back(node.m_key);
            }
        }
    }
    bool find_impl(const table_type& raw_table, const key_type key) const
    {
        hash_func_type hash_func;
        equal_predicate_type eq_func;

        const size_type hash = hash_func(key);

        const node_type* table = raw_table.m_table;
        const size_type capacity = raw_table.m_capacity;

        for (size_type i = hash % capacity;; ++i)
        {
            if (i == capacity)
            {
                i = 0;
            }
            const node_type& node = table[i];

            switch (node.m_state)
            {
            case node_type::unused:
                // the search finished
                return false;
            case node_type::touched:
                if (eq_func(key, node.m_key))
                {
                    // the item was erased recently
                    return false;
                }
                break;
            case node_type::allocated:
                if (eq_func(key, node.m_key))
                {
                    return true;
                }
                break;
            default:
                assert(false);
            }
        }
        return false;
    }
    bool insert_impl(table_type& raw_table, const key_type key)
    {
        hash_func_type hash_func;
        equal_predicate_type eq_func;

        const size_type hash = hash_func(key);

        node_type* table = raw_table.m_table;
        const size_type capacity = raw_table.m_capacity;

        for (size_type i = hash % capacity;; ++i)
        {
            if (i == capacity)
            {
                i = 0;
            }
            node_type& node = table[i];

            switch (node.m_state)
            {
            case node_type::unused:
            {
                // the slot is empty so try to use it
                const node_type new_node(key, node_type::allocated);

                if (atomic_cas(table[i], node, new_node))
                {
                    ++raw_table.m_used;
                    ++raw_table.m_size;
                    return true;
                }
                // the slot has been updated by other thread so we have to start all over again
                continue;
            }
            case node_type::allocated:
                if (eq_func(key, node.m_key))
                {
                    // the item is allocated or concurrent insert/delete operation is in progress
                    return false;
                }
                break;
            case node_type::touched:
                if (eq_func(key, node.m_key))
                {
                    static const state_type touched = node_type::touched;
                    static const state_type allocated = node_type::allocated;

                    if (atomic_cas(table[i].m_state, touched, allocated))
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
    bool erase_impl(table_type& raw_table, const key_type & key)
    {
        hash_func_type hash_func;
        equal_predicate_type eq_func;

        const size_type hash = hash_func(key);

        node_type* table = raw_table.m_table;
        const size_type capacity = raw_table.m_capacity;

        for (size_type i = hash % capacity;; ++i)
        {
            if (i == capacity)
            {
                i = 0;
            }
            node_type& node = table[i];

            switch (node.m_state)
            {
            case node_type::unused:
                // the search finished
                return false;
            case node_type::touched:
                if (eq_func(key, node.m_key))
                {
                    // the item was erased recently
                    return false;
                }
                break;
            case node_type::allocated:
                if (eq_func(key, node.m_key))
                {
                    static const state_type touched = node_type::touched;
                    static const state_type allocated = node_type::allocated;

                    if (atomic_cas(table[i].m_state, allocated, touched))
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
        assert(
                node.m_state == node_type::allocated
                        || node.m_state == node_type::touched
                        || node.m_state == node_type::unused);
    }
    void rehash_impl(const table_type& src, table_type& dst)
    {
        for (size_type i = 0; i < src.m_capacity; ++i)
        {
            node_type& node = src.m_table[i];

            if (node.m_state == node_type::allocated)
            {
                key_type & key = node.m_key;

                insertUniqueKey(dst, node);
            }
        }
    }
    // simplified form of insert()
    // the function assumes:
    //    * exclusive access to the container
    //    * new key is unique
    //    * table has enough capacity to insert specified element
    void insertUniqueKey(table_type& dst, const node_type& new_node)
    {
        hash_func_type hash_func;
        const size_type hash = hash_func(new_node.m_key);
        const size_type capacity = dst.m_capacity;
        node_type* table = dst.m_table;

        for (size_type i = hash % capacity;; ++i)
        {
            if (i == capacity)
            {
                i = 0;
            }
            node_type& node = table[i];
            if (node_type::unused == node.m_state)
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

#endif /* INCLUDE_HASH_SET_TABLE_INTEGRAL_KEY_HPP_ */
