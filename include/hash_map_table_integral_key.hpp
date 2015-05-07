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
#include "cppbasics.hpp"

#include <utility>
#include <algorithm>
#include <cassert>
#include <functional>
#include <vector>

namespace lfds
{

template<typename Key, typename Value, typename Hash, typename Pred,
        typename Allocator>
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
    static constexpr bool INTEGRAL_VALUE = false;
    static constexpr bool INTEGRAL_KEYVALUE = false;

private:

    typedef typename node_type::key_item_type key_item_type;
    typedef typename node_type::state_type state_type;
    typedef ref_lock<const node_type> scoped_node_ref_lock;

public:
    hash_map_table_integral_key()
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
            const key_item_type item = node.getKey();

            if (item.m_state == key_item_type::allocated)
            {
                scoped_node_ref_lock guard(node);
                state_type state = node.getState();
                if (state == key_item_type::allocated)
                {
                    snapshot.push_back(
                            value_type(item.m_key, *node.getValue()));
                }
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
            const node_type& node = table[i];
            const key_item_type item = node.getKey();

            switch (item.m_state)
            {
            case key_item_type::unused:
                // the search finished
                return false;
            case key_item_type::pending:
                if (m_eq_func(key, item.m_key))
                {
                    // the item is being processed now, there is no reason to wait
                    return false;
                }
                break;
            case key_item_type::touched:
                if (m_eq_func(key, item.m_key))
                {
                    // the item was erased recently
                    return false;
                }
                break;
            case key_item_type::allocated:
                if (m_eq_func(key, item.m_key))
                {
                    scoped_node_ref_lock guard(node);
                    state_type state = node.getState();
                    if (state == key_item_type::allocated)
                    {
                        value = *node.getValue();
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
#if LFDS_USE_CPP11
    template<typename ... Args>
    bool insert_impl(table_type& raw_table,
                     const key_type key,
                     const bool updateIfExists,
                     Args&&... val)
#else // LFDS_USE_CPP11
    bool insert_impl(table_type& raw_table,
                     const key_type key,
                     const bool updateIfExists,
                     const mapped_type &val)
#endif // LFDS_USE_CPP11
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

            node_type& node = table[i];
            const key_item_type item = node.getKey();

            switch (item.m_state)
            {
            case key_item_type::unused:
            {
                // the slot is empty so try to use it
                key_item_type new_item(key, key_item_type::pending);

                if (node.atomic_cas(item, new_item))
                {
                    m_value_allocator.construct(node.getValue(),
                            std_forward(Args, val));
                    node.setState(key_item_type::allocated);
                    ++raw_table.m_used;
                    ++raw_table.m_size;
                    return true;
                }
                // the slot has been updated by other thread so we have to start all over again
                continue;
            }
            case key_item_type::pending:
                if (m_eq_func(key, item.m_key))
                {
                    // insert operation is in progress
                    // cannot continue until it is finished
                    // so start all over again
                    continue;
                }
                break;
            case key_item_type::allocated:
                if (m_eq_func(key, item.m_key))
                {
                    if (!updateIfExists)
                    {
                        // the item is allocated or concurrent insert/delete operation is in progress
                        return false;
                    }
                    key_item_type new_item(key, key_item_type::pending);

                    if (node.atomic_cas(item, new_item))
                    {
                        m_value_allocator.destroy(node.getValue());
                        m_value_allocator.construct(node.getValue(),
                                std_forward(Args, val));
                        node.setState(key_item_type::allocated);
                        return true;
                    }
                    continue;
                }
                break;
            case key_item_type::touched:
                if (m_eq_func(key, item.m_key))
                {
                    key_item_type new_item(key, key_item_type::pending);

                    if (node.atomic_cas(item, new_item))
                    {
                        m_value_allocator.construct(node.getValue(),
                                std_forward(Args, val));
                        node.setState(key_item_type::allocated);
                        ++raw_table.m_size;
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
        const std::size_t hash = m_hash_func(key);

        node_type* table = raw_table.m_table;
        const size_type capacity = raw_table.m_capacity;

        for (size_type i = hash % capacity;; ++i)
        {
            if (i == capacity)
            {
                i = 0;
            }
            node_type& node = table[i];
            const key_item_type item = node.getKey();

            switch (item.m_state)
            {
            case key_item_type::unused:
                // the search finished
                return false;
            case key_item_type::pending:
                if (m_eq_func(key, item.m_key))
                {
                    // the item is being processed now, there is no reason to wait
                    return false;
                }
                break;
            case key_item_type::touched:
                if (m_eq_func(key, item.m_key))
                {
                    // the item was erased recently
                    return false;
                }
                break;
            case key_item_type::allocated:
                if (m_eq_func(key, item.m_key))
                {
                    // reset readiness
                    key_item_type new_item(key, key_item_type::pending);

                    if (node.atomic_cas(item, new_item))
                    {
                        // wait for pending finds
                        node.waitForRelease();
                        // destroy the node
                        m_value_allocator.destroy(node.getValue());
                        node.setState(key_item_type::touched);
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
        key_item_type item = node.getKey();
        // pending and pending2 states are not allowed here
        assert(
                item.m_state == key_item_type::allocated
                        || item.m_state == key_item_type::touched
                        || item.m_state == key_item_type::unused);
        if (item.m_state == key_item_type::allocated)
        {
            m_value_allocator.destroy(node.getValue());
            item.m_state = key_item_type::touched;
        }
    }
    // the function assumes exclusive access
    void rehash_impl(const table_type& src, table_type& dst)
    {
        for (size_type i = 0; i < src.m_capacity; ++i)
        {
            const node_type& node = src.m_table[i];
            const key_item_type item = node.getKey();

            if (item.m_state == key_item_type::allocated)
            {
                const mapped_type & val = *node.getValue();
                insertUniqueKey(dst, item.m_key, val);
            }
        }
    }
private:

    // simplified form of insert()
    // the function assumes:
    //    * exclusive access to the container
    //    * new key is unique
    //    * table has enough capacity to insert specified element
    void insertUniqueKey(table_type& dst,
                         const key_type key,
                         const mapped_type & val)
    {
        const size_type capacity = dst.m_capacity;
        const size_type hash = m_hash_func(key);
        for (size_type i = hash % capacity;; ++i)
        {
            if (i == capacity)
            {
                i = 0;
            }
            node_type& node = dst.m_table[i];
            key_item_type item = node.getKey();
            if (item.m_state == key_item_type::unused)
            {
                node.setKey(key, key_item_type::allocated);
                m_value_allocator.construct(node.getValue(), val);
                ++dst.m_size;
                ++dst.m_used;
                break;
            }
        }
    }
private:
    // fields below do not participate in swap() operation
    value_allocator_type m_value_allocator;
    hash_func_type m_hash_func;
    equal_predicate_type m_eq_func;
};

}

#endif /* INCLUDE_HASH_TABLE_INTEGRAL_KEY_HPP_ */
