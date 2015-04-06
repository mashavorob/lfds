/*
 * hash_set_table.hpp
 *
 *  Created on: Feb 13, 2015
 *      Author: masha
 */

#ifndef INCLUDE_HASH_SET_TABLE_HPP_
#define INCLUDE_HASH_SET_TABLE_HPP_

#include "hash_set_node.hpp"
#include "hash_table_base.hpp"
#include "raw_hash_table.hpp"
#include "ref_lock.hpp"
#include "cas.hpp"

#include <cassert>
#include <vector>

// thread-safe implementation of hash table without value (hash_set) based
// on open addressing algorithm for resolving collisions
//
// generic hash table is not "true lock free" object
// it implements two kinds of operations:
//        * lock free lookup
//        * lock free insert when resizing is not required
//        * resize with exclusive access
//        * lock free delete
namespace lfds
{

template<class Key, class Hash, class Pred, class Allocator>
class hash_set_table
{
public:
    typedef hash_set_table<Key, Hash, Pred, Allocator> this_type;
    typedef Key key_type;
    typedef Hash hash_func_type;
    typedef Pred equal_predicate_type;
    typedef Allocator allocator_type;

    typedef hash_set_node<key_type> node_type;
    typedef hash_data_table<node_type> table_type;

    typedef typename table_type::size_type size_type;
    typedef typename Allocator::template rebind<key_type>::other key_allocator_type;
    typedef std::vector<key_type> snapshot_type;

    static constexpr bool INTEGRAL = false;

//
// template<class CompatibleKey, Key>
// struct CompatiblePredicate : public binary_function<CompatibleKey, Key, bool>
// {
//      bool operator()(const CompatibleKey & key, const Key & value) const;
//      ...
// };
private:
    typedef typename node_type::hash_item_type hash_item_type;
    typedef ref_lock<node_type> scoped_ref_lock;

public:
    hash_set_table()
    {
    }

    void getSnapshot_imp(const table_type& raw_table, snapshot_type & snapshot) const
    {
        const node_type* table = raw_table.m_table;
        const size_type capacity = raw_table.m_capacity;

        for (size_type i = 0; i < capacity; ++i)
        {
            const node_type& node = table[i];
            const hash_item_type item = node.m_hash;

            if (item.m_state == hash_item_type::allocated)
            {
                snapshot.push_back(*node.key());
            }
        }
    }
    bool find_impl(const table_type& raw_table, const key_type & key) const
    {
        hash_func_type hash_func;
        equal_predicate_type eq_func;

        const size_type hash = hash_func(key);
        const node_type* table = raw_table.m_table;
        const size_type capacity = raw_table.m_capacity;

        for (size_type i = hash % capacity;; ++i)
        {
            if ( i == capacity )
            {
                i = 0;
            }
            const node_type& node = table[i];
            const hash_item_type item = node.m_hash;

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
            case hash_item_type::touched:
                if (eq_func(key, *node.key()))
                {
                    // the item was erased recently
                    return false;
                }
                break;
            case hash_item_type::allocated:
                if (eq_func(key, *node.key()))
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

    bool insert_impl(table_type& raw_table, const key_type & key)
    {
        hash_func_type hash_func;
        equal_predicate_type eq_func;

        const size_type hash = hash_func(key);
        node_type* table = raw_table.m_table;
        const size_type capacity = raw_table.m_capacity;

        for (size_type i = hash % capacity;; ++i)
        {
            if ( i == capacity )
            {
                i = 0;
            }
            node_type& node = table[i];
            const hash_item_type item = node.m_hash;

            switch (item.m_state)
            {
            case hash_item_type::unused:
            {
                // the slot is empty so try to use it
                hash_item_type new_item(hash, hash_item_type::pending);

                if (atomic_cas(node.m_hash, item, new_item))
                {
                    m_key_allocator.construct(node.key(), key);
                    node.m_hash.m_state = hash_item_type::allocated;
                    ++raw_table.m_size;
                    ++raw_table.m_used;
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
            case hash_item_type::allocated:
                if (eq_func(key, *node.key()))
                {
                    // the item is allocated or concurrent insert/delete operation is in progress
                    return false;
                }
                break;
            case hash_item_type::touched:
                if (eq_func(key, *node.key()))
                {
                    static constexpr std::size_t touched = hash_item_type::touched;
                    static constexpr std::size_t allocated = hash_item_type::allocated;

                    if (atomic_cas(node.m_hash.m_state, touched, allocated))
                    {
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
    bool erase_impl(table_type& raw_table, const key_type & key)
    {
        hash_func_type hash_func;
        equal_predicate_type eq_func;

        const size_type hash = hash_func(key);
        node_type* table = raw_table.m_table;
        const size_type capacity = raw_table.m_capacity;

        for (size_type i = hash % capacity;; ++i)
        {
            if ( i == capacity )
            {
                i = 0;
            }
            node_type& node = table[i];
            const hash_item_type item = node.m_hash;

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
            case hash_item_type::touched:
                if (eq_func(key, *node.key()))
                {
                    // the item was erased recently
                    return false;
                }
                break;
            case hash_item_type::allocated:
                if (eq_func(key, *node.key()))
                {
                    static constexpr std::size_t touched = hash_item_type::touched;
                    static constexpr std::size_t allocated = hash_item_type::allocated;

                    // reset readiness
                    if (atomic_cas(node.m_hash.m_state, allocated, touched))
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
        hash_item_type item = node.m_hash;
        // pending and pending2 states are not allowed here
        assert(
                item.m_state == hash_item_type::allocated
                        || item.m_state == hash_item_type::touched
                        || item.m_state == hash_item_type::unused);
        if (item.m_state == hash_item_type::allocated || item.m_state == hash_item_type::touched)
        {
            m_key_allocator.destroy(node.key());
        }
    }
    void rehash_impl(const table_type& src, table_type& dst)
    {
        for (size_type i = 0; i < src.m_capacity; ++i)
        {
            node_type& node = src.m_table[i];
            const hash_item_type item = node.m_hash;

            if (item.m_state == hash_item_type::allocated)
            {
                key_type & key = *node.key();

                insert_unique_key(dst, item.m_hash, key);
            }
        }
    }

    // simplified form of insert()
    // the function assumes:
    //    * exclusive access to the container
    //    * new key is unique
    //    * table has enough capacity to insert specified element
    void insert_unique_key(table_type& dst, const size_type hash, const key_type & key)
    {
        const size_type capacity = dst.m_capacity;

        for (size_type i = hash % capacity;; ++i)
        {
            if ( i == capacity )
            {
                i = 0;
            }
            node_type& node = dst.m_table[i];
            const hash_item_type item = node.m_hash;
            if (item.m_state == hash_item_type::unused)
            {
                node.m_hash.m_hash = hash;
                node.m_hash.m_state = hash_item_type::allocated;

                m_key_allocator.construct(node.key(), key);

                ++dst.m_size;
                ++dst.m_used;
                break;
            }
        }
    }
private:
    key_allocator_type m_key_allocator;
};

}

#endif /* INCLUDE_HASH_SET_TABLE_HPP_ */
