/*
 * hash_map_integral_pair_16b.hpp
 *
 *  Created on: Apr 20, 2015
 *      Author: masha
 */

#ifndef INCLUDE_HASH_MAP_TABLE_INTEGRAL_VALUE_HPP_
#define INCLUDE_HASH_MAP_TABLE_INTEGRAL_VALUE_HPP_

#include "hash_map_node_integral_value.hpp"
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

template<class Key, class Value, class Hash, class Pred, class Allocator>
class hash_map_table_integral_value
{
public:
    typedef hash_map_table_integral_value<Key, Value, Hash, Pred, Allocator> this_type;
    typedef Key key_type;
    typedef Value mapped_type;
    typedef Hash hash_func_type;
    typedef Pred equal_predicate_type;
    typedef Allocator allocator_type;

    typedef hash_node_integral_value<key_type, mapped_type> node_type;
    typedef hash_data_table<node_type> table_type;
    typedef typename node_type::value_item_type value_item_type;
    typedef typename table_type::size_type size_type;
    typedef typename allocator_type::template rebind<key_type>::other key_allocator_type;

    typedef std::pair<key_type, mapped_type> value_type;
    typedef std::vector<value_type> snapshot_type;

    static const bool INTEGRAL_KEY = false;
    static const bool INTEGRAL_VALUE = true;
    static const bool INTEGRAL_KEYVALUE = false;

//
// template<class CompatibleKey, Key>
// struct CompatiblePredicate : public binary_function<CompatibleKey, Key, bool>
// {
//      bool operator()(const CompatibleKey & key, const Key & value) const;
//      ...
// };

public:
    hash_map_table_integral_value()
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
            const value_item_type& item = node.getValue();
            if (item.m_state == value_item_type::allocated)
            {
                snapshot.push_back(value_type(*node.getKey(), static_cast<mapped_type>(item.m_value)));
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
            const value_item_type item = node.getValue();

            switch (item.m_state)
            {
            case value_item_type::unused:
                // the search finished
                return false;
            case value_item_type::pending:
                // the first insert is in progress
                // cannot continue until it is finished
                // so start all over again
                //continue;
                break;
            case value_item_type::touched:
                if (m_eq_func(key, *node.getKey()))
                {
                    // the item was erased recently
                    return false;
                }
                break;
            case value_item_type::allocated:
                if (m_eq_func(key, *node.getKey()))
                {
                    value = item.m_value;
                    return true;
                }
                break;
            default:
                assert(false);
            }
        }

        return false;
    }

    bool insert_impl(table_type& raw_table,
                     const key_type & key,
                     const mapped_type &val)
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
            const value_item_type item = node.getValue();

            switch (item.m_state)
            {
            case value_item_type::unused:
            {
                // the slot is empty so try to use it
                if (node.getValue().atomic_cas(value_item_type::unused,
                        value_item_type::pending))
                {
                    value_item_type & ritem = node.getValue();
                    ritem.m_value = val;
                    m_key_allocator.construct(node.getKey(), key);
                    ritem.m_state = value_item_type::allocated;
                    ++raw_table.m_used;
                    ++raw_table.m_size;
                    return true;
                }
                // the slot has been updated by other thread so we have to start all over again
                continue;
            }
            case value_item_type::pending:
                // the first insert is in progress
                // cannot continue until it is finished
                // so start all over again
                continue;
            case value_item_type::allocated:
                if (m_eq_func(key, *node.getKey()))
                {
                    // the item is allocated or concurrent insert/delete operation is in progress
                    return false;
                }
                break;
            case value_item_type::touched:
                if (m_eq_func(key, *node.getKey()))
                {
                    if (node.getValue().atomic_cas(value_item_type::touched,
                            value_item_type::pending))
                    {
                        value_item_type & ritem = node.getValue();
                        ritem.m_value = val;
                        ritem.m_state = value_item_type::allocated;
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
            const value_item_type item = node.getValue();

            switch (item.m_state)
            {
            case value_item_type::unused:
                // the search finished
                return false;
            case value_item_type::pending:
                // the first insert is in progress
                break;
            case value_item_type::touched:
                if (m_eq_func(key, *node.getKey()))
                {
                    // the item was erased recently
                    return false;
                }
                break;
            case value_item_type::allocated:
                if (m_eq_func(key, *node.getKey()))
                {
                    // reset readiness
                    if (node.getValue().atomic_cas(value_item_type::allocated,
                            value_item_type::touched))
                    {
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
        const value_item_type item = node.getValue();
        // pending and pending2 states are not allowed here
        assert(
                item.m_state == value_item_type::allocated
                        || item.m_state == value_item_type::touched
                        || item.m_state == value_item_type::unused);
        if (item.m_state == value_item_type::allocated
                || item.m_state == value_item_type::touched)
        {
            m_key_allocator.destroy(node.getKey());
        }
    }
    void rehash_impl(const table_type& src, table_type& dst)
    {
        for (size_type i = 0; i < src.m_capacity; ++i)
        {
            node_type& node = src.m_table[i];
            const value_item_type& item = node.getValue();

            if (item.m_state == value_item_type::allocated)
            {
                key_type & key = *node.getKey();
                const mapped_type val = node.getValue().m_value;
                insertUniqueKey(dst, key, val);
            }
        }
    }
    // simplified form of insert()
    // the function assumes:
    //    * exclusive access to the container
    //    * new key is unique
    //    * table has enough capacity to insert specified element
    void insertUniqueKey(table_type& dst,
                           const key_type & key,
                           const mapped_type val)
    {
        const size_type hash = m_hash_func(key);
        const size_type capacity = dst.m_capacity;

        for (size_type i = hash % capacity;; ++i)
        {
            if (i == capacity)
            {
                i = 0;
            }
            node_type& node = dst.m_table[i];
            value_item_type& item = node.getValue();
            if (item.m_state == value_item_type::unused)
            {
                m_key_allocator.construct(node.getKey(), key);
                item.m_value = val;
                item.m_state = value_item_type::allocated;
                ++dst.m_size;
                ++dst.m_used;
                break;
            }
        }
    }
private:
    key_allocator_type m_key_allocator;
    hash_func_type m_hash_func;
    equal_predicate_type m_eq_func;
};

}

#endif /* INCLUDE_HASH_MAP_TABLE_INTEGRAL_VALUE_HPP_ */
