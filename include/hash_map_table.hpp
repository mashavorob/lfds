/*
 * hash_table.hpp
 *
 *  Created on: Jan 23, 2015
 *      Author: masha
 */

#ifndef INCLUDE_HASH_MAP_TABLE_HPP_
#define INCLUDE_HASH_MAP_TABLE_HPP_

#include "hash_map_node.hpp"
#include "raw_hash_table.hpp"
#include "ref_lock.hpp"
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
class hash_map_table
{
public:
    typedef hash_map_table<Key, Value, Hash, Pred, Allocator> this_type;
    typedef Key key_type;
    typedef Value mapped_type;
    typedef Hash hash_func_type;
    typedef Pred equal_predicate_type;
    typedef Allocator allocator_type;

    typedef hash_node<key_type, mapped_type> node_type;
    typedef hash_data_table<node_type> table_type;
    typedef typename table_type::size_type size_type;
    typedef typename allocator_type::template rebind<mapped_type>::other value_allocator_type;
    typedef typename allocator_type::template rebind<key_type>::other key_allocator_type;

    typedef std::pair<key_type, mapped_type> value_type;
    typedef std::vector<value_type> snapshot_type;

    static constexpr bool INTEGRAL_KEY = false;
    static constexpr bool INTEGRAL_VALUE = false;
    static constexpr bool INTEGRAL_KEYVALUE = false;

//
// template<typename CompatibleKey, Key>
// struct CompatiblePredicate : public binary_function<CompatibleKey, Key, bool>
// {
//      bool operator()(const CompatibleKey & key, const Key & value) const;
//      ...
// };
private:
    typedef typename node_type::hash_item_type hash_item_type;
    typedef ref_lock<const node_type> scoped_ref_lock;

public:
    hash_map_table()
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
            const hash_item_type item = node.getHash();

            if (item.m_state == hash_item_type::allocated)
            {
                scoped_ref_lock guard(node);
                std::size_t state = node.getState();
                if (state == hash_item_type::allocated)
                {
                    snapshot.push_back(
                            value_type(*node.getKey(), *node.getValue()));
                }
            }
        }
    }
    bool find_impl(const table_type& raw_table,
                   const key_type & key,
                   mapped_type & value) const
    {
        const size_type hash = m_hash_func(key);

        const node_type* table = raw_table.m_table;
        const size_type capacity = raw_table.m_capacity;

        for (size_type i = hash % capacity;; ++i)
        {
            if (i == capacity)
            {
                i = 0;
            }

            const node_type& node = table[i];
            const hash_item_type item = node.getHash();

            switch (item.m_state)
            {
            case hash_item_type::unused:
                // the search finished
                return false;
            case hash_item_type::pending:
                if (item.m_hash == hash)
                {
                    // the first insert is in progress
                    // cannot continue until it is finished
                    // so start all over again
                    continue;
                }
                break;
            case hash_item_type::pending2:
                if (m_eq_func(key, *node.getKey()))
                {
                    // the item is being processed now, there is no reason to wait
                    return false;
                }
                break;
            case hash_item_type::touched:
                if (m_eq_func(key, *node.getKey()))
                {
                    // the item was erased recently
                    return false;
                }
                break;
            case hash_item_type::allocated:
                if (m_eq_func(key, *node.getKey()))
                {
                    scoped_ref_lock guard(node);
                    std::size_t state = node.getState();
                    if (state == hash_item_type::allocated)
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
    bool insert_impl(table_type& raw_table, const key_type & key, Args&&... val)
#else
    bool insert_impl(table_type& raw_table,
                     const key_type & key,
                     const mapped_type &val)
#endif
    {
        const size_type hash = m_hash_func(key);

        node_type* table = raw_table.m_table;
        const size_type capacity = raw_table.m_capacity;

        for (size_type i = hash % capacity;; ++i)
        {
            if (i == capacity)
            {
                i = 0;
            }

            node_type& node = table[i];
            const hash_item_type item = node.getHash();

            switch (item.m_state)
            {
            case hash_item_type::unused:
            {
                // the slot is empty so try to use it
                hash_item_type new_item(hash, hash_item_type::pending);

                if (node.atomic_cas(item, new_item))
                {
                    m_value_allocator.construct(node.getValue(),
                            std_forward(Args, val));
                    m_key_allocator.construct(node.getKey(), key);
                    node.setState(hash_item_type::allocated);

                    ++raw_table.m_used;
                    ++raw_table.m_size;
                    return true;
                }
                // the slot has been updated by other thread so we have to start all over again
                continue;
            }
            case hash_item_type::pending:
                if (item.m_hash == hash)
                {
                    // the first insert is in progress
                    // cannot continue until it is finished
                    // so start all over again
                    continue;
                }
                break;
            case hash_item_type::pending2:
            case hash_item_type::allocated:
                if (m_eq_func(key, *node.getKey()))
                {
                    // the item is allocated or concurrent insert/delete operation is in progress
                    return false;
                }
                break;
            case hash_item_type::touched:
                if (m_eq_func(key, *node.getKey()))
                {
                    hash_item_type new_item(hash, hash_item_type::pending2);

                    if (node.atomic_cas(item, new_item))
                    {
                        m_value_allocator.construct(node.getValue(),
                                std_forward(Args, val));
                        node.setState(hash_item_type::allocated);
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
        return false;
    }

    bool erase_impl(table_type& raw_table, const key_type & key)
    {
        const size_type hash = m_hash_func(key);

        node_type* table = raw_table.m_table;
        const size_type capacity = raw_table.m_capacity;

        for (size_type i = hash % capacity;; ++i)
        {
            if (i == capacity)
            {
                i = 0;
            }

            node_type& node = table[i];
            const hash_item_type item = node.getHash();

            switch (item.m_state)
            {
            case hash_item_type::unused:
                // the search finished
                return false;
            case hash_item_type::pending:
                if (item.m_hash == hash)
                {
                    // the first insert is in progress
                    // cannot continue until it is finished
                    // so start all over again
                    continue;
                }
                break;
            case hash_item_type::pending2:
                if (m_eq_func(key, *node.getKey()))
                {
                    // the item is being processed now, there is no reason to wait
                    return false;
                }
                break;
            case hash_item_type::touched:
                if (m_eq_func(key, *node.getKey()))
                {
                    // the item was erased recently
                    return false;
                }
                break;
            case hash_item_type::allocated:
                if (m_eq_func(key, *node.getKey()))
                {
                    // reset readiness
                    hash_item_type new_item(hash, hash_item_type::pending2);

                    if (node.atomic_cas(item, new_item))
                    {
                        // wait for pending finds
                        node.waitForRelease();
                        // destroy the node
                        m_value_allocator.destroy(node.getValue());
                        node.setState(hash_item_type::touched);
                        --raw_table.m_size;
                        return true;
                    }
                    // the item found but it is being erased in an other thread;
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
        hash_item_type item = node.getHash();
        // pending and pending2 states are not allowed here
        assert(
                item.m_state == hash_item_type::allocated
                        || item.m_state == hash_item_type::touched
                        || item.m_state == hash_item_type::unused);
        if (item.m_state == hash_item_type::allocated)
        {
            m_value_allocator.destroy(node.getValue());
            item.m_state = hash_item_type::touched;
        }
        if (item.m_state == hash_item_type::touched)
        {
            m_key_allocator.destroy(node.getKey());
        }
    }
    void rehash_impl(const table_type& src, table_type& dst)
    {
        for (size_type i = 0; i < src.m_capacity; ++i)
        {
            node_type& node = src.m_table[i];
            const hash_item_type item = node.getHash();

            if (item.m_state == hash_item_type::allocated)
            {
                key_type & key = *node.getKey();
                mapped_type & val = *node.getValue();
                insertUniqueKey(dst, item.m_hash, key, val);
            }
        }
    }
    // simplified form of insert()
    // the function assumes:
    //    * exclusive access to the container
    //    * new key is unique
    //    * table has enough capacity to insert specified element
    void insertUniqueKey(table_type& dst,
                         const size_type hash,
                         const key_type & key,
                         const mapped_type & val)
    {
        const size_type capacity = dst.m_capacity;

        for (size_type i = hash % capacity;; ++i)
        {
            if (i == capacity)
            {
                i = 0;
            }
            node_type& node = dst.m_table[i];
            hash_item_type item = node.getHash();
            if (item.m_state == hash_item_type::unused)
            {
                node.setItem(hash, hash_item_type::allocated);

                m_key_allocator.construct(node.getKey(), key);
                m_value_allocator.construct(node.getValue(), val);

                ++dst.m_size;
                ++dst.m_used;
                break;
            }
        }
    }
private:
    key_allocator_type m_key_allocator;
    value_allocator_type m_value_allocator;
    hash_func_type m_hash_func;
    equal_predicate_type m_eq_func;
};

}

#endif /* INCLUDE_HASH_MAP_TABLE_HPP_ */
