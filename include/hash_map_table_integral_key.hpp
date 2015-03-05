/*
 * hash_table_integral_key.hpp
 *
 *  Created on: Feb 6, 2015
 *      Author: masha
 */

#ifndef INCLUDE_HASH_TABLE_INTEGRAL_KEY_HPP_
#define INCLUDE_HASH_TABLE_INTEGRAL_KEY_HPP_

#include "hash_map_node_integral_key.hpp"
#include "hash_table_base.hpp"
#include "ref_lock.hpp"
#include "ref_ptr.hpp"

#include <utility>
#include <algorithm>
#include <atomic>
#include <cassert>
#include <iostream>
#include <functional>
#include <vector>

// thread-safe implementation of hash table with open addressing
//
// generic hash table is not "true lock free" object
// it implements two kinds of operations:
//        * lock free lookup
//        * lock free insert when resizing is not required
//        * resize with exclusive access
//        * deleting items with exclusive access
namespace lfds
{

template<class Key, class Value, class Hash, class Pred, class Allocator>
class hash_map_table_integral_key
{
public:
    typedef hash_map_table_integral_key<Key, Value, Hash, Pred, Allocator> this_type;

    typedef Key key_type;
    typedef Value mapped_type;
    typedef Hash hash_func_type;
    typedef Pred equal_predicate_type;
    typedef Allocator allocator_type;

    typedef hash_node_integral_key<key_type, mapped_type> node_type;
    typedef hash_data_table<node_type> table_type;
    typedef typename table_type::size_type size_type;
    typedef typename allocator_type::template rebind<mapped_type>::other value_allocator_type;

    typedef std::pair<key_type, mapped_type> value_type;
    typedef std::vector<value_type> snapshot_type;

    static constexpr bool INTEGRAL_KEY = true;
    static constexpr bool INTEGRAL_KEYVALUE = false;

private:

    typedef typename node_type::key_item_type key_item_type;
    typedef typename node_type::state_type state_type;
    typedef ref_lock<node_type> scoped_node_ref_lock;

public:
    hash_map_table_integral_key()
    {
    }

    void getSnapshot_imp(const table_type& raw_table, snapshot_type & snapshot) const
    {
        const node_type* table = raw_table.m_table;
        const size_type capacity = raw_table.m_capacity;

        for (size_type i = 0; i < capacity; ++i)
        {
            const node_type& node = table[i];
            const key_item_type item = node.key();

            if (item.m_state == key_item_type::allocated)
            {
                scoped_node_ref_lock guard(node);
                state_type state = node.state();
                if (state == key_item_type::allocated)
                {
                    snapshot.push_back(value_type(*node.key(), *node.value()));
                }
            }
        }
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
            const node_type& node = table[i];
            const key_item_type item = node.key();

            switch (item.m_state)
            {
            case key_item_type::unused:
                // the search finished
                return false;
            case key_item_type::pending:
                if (eq_func(key, item.m_key))
                {
                    // the item is being processed now, there is no reason to wait
                    return false;
                }
                break;
            case key_item_type::touched:
                if (eq_func(key, item.m_key))
                {
                    // the item was erased recently
                    return false;
                }
                break;
            case key_item_type::allocated:
                if (eq_func(key, item.m_key))
                {
                    scoped_node_ref_lock guard(node);
                    state_type state = node.state();
                    if (state == key_item_type::allocated)
                    {
                        value = *node.value();
                        return true;
                    }
                    else
                    {
                        // the item is being updated, there is no reason to wait
                        return false;
                    }
                }
                break;
            default:
                assert(false);
            }
        }

        return false;
    }

    template<class ... Args>
    bool insert_impl(table_type& raw_table, const key_type key, Args&&... val)
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

            node_type& node = table[i];
            const key_item_type item = node.key();

            switch (item.m_state)
            {
            case key_item_type::unused:
            {
                // the slot is empty so try to use it
                key_item_type new_item =
                { key, key_item_type::pending };

                if (node.atomic_cas_hash(item, new_item))
                {
                    m_value_allocator.construct(node.value(),
                            std::forward<Args>(val)...);
                    node.state(key_item_type::allocated);
                    raw_table.m_used.fetch_add(1, std::memory_order_relaxed);
                    raw_table.m_size.fetch_add(1, std::memory_order_release);
                    return true;
                }
                // the slot has been updated by other thread so we have to start all over again
                continue;
            }
            case key_item_type::pending:
                if (eq_func(key, item.m_key))
                {
                    // insert operation is in progress
                    // cannot continue until it is finished
                    // so start all over again
                    continue;
                }
                break;
            case key_item_type::allocated:
                if (eq_func(key, item.m_key))
                {
                    // the item is allocated or concurrent insert/delete operation is in progress
                    return false;
                }
                break;
            case key_item_type::touched:
                if (eq_func(key, item.m_key))
                {
                    key_item_type new_item =
                    { key, key_item_type::pending };

                    if (node.atomic_cas_hash(item, new_item))
                    {
                        m_value_allocator.construct(node.value(),
                                std::forward<Args>(val)...);
                        node.state(key_item_type::allocated);
                        raw_table.m_size.fetch_add(1, std::memory_order_release);
                        return true;
                    }
                    // the slot has been updated by other thread
                    // so at least one concurrent insert operation took place
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
            node_type& node = table[i];
            const key_item_type item = node.key();

            switch (item.m_state)
            {
            case key_item_type::unused:
                // the search finished
                return false;
            case key_item_type::pending:
                if (eq_func(key, item.m_key))
                {
                    // the item is being processed now, there is no reason to wait
                    return false;
                }
                break;
            case key_item_type::touched:
                if (eq_func(key, item.m_key))
                {
                    // the item was erased recently
                    return false;
                }
                break;
            case key_item_type::allocated:
                if (eq_func(key, item.m_key))
                {
                    // reset readiness
                    key_item_type new_item =
                    { key, key_item_type::pending };

                    if (node.atomic_cas_hash(item, new_item))
                    {
                        // wait for pending finds
                        node.wait_for_release();
                        // destroy the node
                        m_value_allocator.destroy(node.value());
                        node.state(key_item_type::touched);
                        raw_table.m_size.fetch_sub(1, std::memory_order_release);
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
        key_item_type item = node.key();
        // pending and pending2 states are not allowed here
        assert(
                item.m_state == key_item_type::allocated
                        || item.m_state == key_item_type::touched
                        || item.m_state == key_item_type::unused);
        if (item.m_state == key_item_type::allocated)
        {
            m_value_allocator.destroy(node.value());
            item.m_state = key_item_type::touched;
        }
    }
    // the function assumes exclusive access
    void rehash_impl(const table_type& src, table_type& dst)
    {
        for (size_type i = 0; i < src.m_capacity; ++i)
        {
            const node_type& node = src.m_table[i];
            const key_item_type item = node.key();

            if (item.m_state == key_item_type::allocated)
            {
                const mapped_type & val = *node.value();
                insert_unique_key(dst, item.m_key, val);
            }
        }
    }
private:

    // simplified form of insert()
    // the function assumes:
    //    * exclusive access to the container
    //    * new key is unique
    //    * table has enough capacity to insert specified element
    void insert_unique_key(table_type& dst, const key_type key,
            const mapped_type & val)
    {
        hash_func_type hash_func;
        const size_type capacity = dst.m_capacity;
        const size_type hash = hash_func(key);
        for (size_type i = hash % capacity;; ++i)
        {
            if ( i == capacity )
            {
                i = 0;
            }
            node_type& node = dst.m_table[i];
            key_item_type item = node.key();
            if (item.m_state == key_item_type::unused)
            {
                node.key(key, key_item_type::allocated);
                m_value_allocator.construct(node.value(), val);
                ++dst.m_size;
                ++dst.m_used;
                break;
            }
        }
    }
private:
    // fields below do not participate in swap() operation
    value_allocator_type m_value_allocator;
};

}

#endif /* INCLUDE_HASH_TABLE_INTEGRAL_KEY_HPP_ */
