/*
 * hash_table_integral_key.hpp
 *
 *  Created on: Feb 6, 2015
 *      Author: masha
 */

#ifndef INCLUDE_HASH_TABLE_INTEGRAL_KEY_HPP_
#define INCLUDE_HASH_TABLE_INTEGRAL_KEY_HPP_

#include "hash_table_base.hpp"
#include "two_level_lock.hpp"
#include "hash_node_integral_key.hpp"
#include "hash_table_base.hpp"
#include "ref_lock.hpp"

#include <utility>
#include <algorithm>
#include <atomic>
#include <cassert>
#include <iostream>
#include <functional>

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
class hash_table_integral_key: private hash_table_base
{
public:
    typedef hash_table_base base_type;
    typedef hash_table_integral_key<Key, Value, Hash, Pred, Allocator> this_type;
    typedef Key key_type;
    typedef Value value_type;
    typedef Hash hash_type;
    typedef Pred equal_predicate_type;
    typedef hash_node_integral_key<key_type, value_type> node_type;
    typedef typename base_type::size_type size_type;
    typedef Allocator value_allocator_type;
    typedef typename Allocator::template rebind<node_type>::other node_allocator_type;

    static const bool INTEGRAL_KEY = true;
    static const bool INTEGRAL_KEYVALUE = false;

private:

    typedef typename node_type::key_item_type key_item_type;
    typedef typename node_type::state_type state_type;
    typedef ref_lock<node_type> scoped_ref_lock;

public:
    hash_table_integral_key(const size_type reserve,
            value_allocator_type& value_allocator,
            node_allocator_type& node_allocator) :
            base_type(reserve), m_value_allocator(value_allocator), m_node_allocator(
                    node_allocator)
    {
        m_table = m_node_allocator.allocate(m_capacity);

        for (size_type i = 0; i < m_capacity; ++i)
        {
            m_node_allocator.construct(&m_table[i]);
        }
    }
    ~hash_table_integral_key()
    {
        for (size_type i = 0; i < m_capacity; ++i)
        {
            node_type& node = m_table[i];
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
        m_node_allocator.deallocate(m_table, m_capacity);
    }

    bool find(const key_type key, value_type & value) const
    {
        hash_type hash_func;
        equal_predicate_type eq_func;

        std::size_t hash = hash_func(key);

        size_type start = hash % m_capacity;
        size_type i = start;

        // prevent concurrent deleting
        typedef weak_lock lock_type;
        typedef lock_type::guard_type guard_type;

        guard_type guard = lock_type::create(m_lock);
        guard.lock();

        do
        {
            const node_type& node = m_table[i];
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
                    scoped_ref_lock guard(node);
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
            advance_index(i);
        } while (i != start);

        return false;
    }

    template<class ... Args>
    bool insert(const key_type key, Args&&... val)
    {
        check_watermark(std::bind(&this_type::rehash, this, std::placeholders::_1));

        hash_type hash_func;
        equal_predicate_type eq_func;

        std::size_t hash = hash_func(key);

        size_type start = hash % m_capacity;
        size_type i = start;

        // prevent concurrent deleting
        typedef weak_lock lock_type;
        typedef lock_type::guard_type guard_type;

        guard_type guard = lock_type::create(m_lock);
        guard.lock();

        do
        {
            node_type& node = m_table[i];
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
                    m_node_allocator.construct(node.value(),
                            std::forward<Args>(val)...);

                    m_size.fetch_add(1, std::memory_order_relaxed);
                    m_used.fetch_add(1, std::memory_order_relaxed);

                    node.state(key_item_type::allocated);
                    std::atomic_thread_fence(std::memory_order_release);
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
                        m_node_allocator.construct(node.value(),
                                std::forward<Args>(val)...);
                        m_size.fetch_add(1, std::memory_order_relaxed);
                        node.state(key_item_type::allocated);
                        std::atomic_thread_fence(std::memory_order_release);
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
            advance_index(i);
        } while (i != start);
        throw std::bad_alloc();
        return false;
    }

    bool erase(const key_type key)
    {
        hash_type hash_func;
        equal_predicate_type eq_func;

        std::size_t hash = hash_func(key);

        size_type start = hash % m_capacity;
        size_type i = start;

        // prevent concurrent accessing
        typedef weak_lock lock_type;
        typedef lock_type::guard_type guard_type;

        guard_type guard = lock_type::create(m_lock);
        guard.lock();

        do
        {
            node_type& node = m_table[i];
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
                        m_size.fetch_sub(1, std::memory_order_relaxed);
                        std::atomic_thread_fence(std::memory_order_release);
                        return true;
                    }
                    // the item found but it is being erased in another thread;
                    return false;
                }
                break;
            default:
                assert(false);
            }
            advance_index(i);
        } while (i != start);

        return false;
    }

    size_type size() const
    {
        return m_size;
    }
    size_type capacity() const
    {
        return m_capacity;
    }
private:
    // the function assumes exclusive access
    void rehash(const size_type new_size)
    {
        this_type buffer(new_size, m_value_allocator, m_node_allocator);

        for (size_type i = 0; i < m_capacity; ++i)
        {
            node_type& node = m_table[i];
            const key_item_type item = node.key();

            if (item.m_state == key_item_type::allocated)
            {
                value_type & val = *node.value();
                buffer.insert_unique_key(item.m_key,
                        std::forward<value_type>(val));
            }
        }
        assert(buffer.size() == size());
        assert(buffer.size() < buffer.m_high_watermark);
        swap(buffer);
    }

    // simplified form of insert()
    // the function assumes:
    //    * exclusive access to the container
    //    * new key is unique
    //    * table has enough capacity to insert specified element
    void insert_unique_key(key_type key, value_type && val)
    {
        hash_type hash_func;
        size_type hash = hash_func(key);
        size_type start = hash % m_capacity;
        size_type i = start;

        do
        {
            node_type& node = m_table[i];
            key_item_type item = node.key();
            if (item.m_state == key_item_type::unused)
            {
                node.key(key, key_item_type::allocated);

                m_value_allocator.construct(node.value(),
                        std::forward<value_type>(val));

                m_size.fetch_add(1, std::memory_order_relaxed);
                m_used.fetch_add(1, std::memory_order_relaxed);
                break;
            }
            advance_index(i);
        } while (i != start);
    }
    void swap(this_type& other)
    {
        base_type::swap(other);
        std::swap(m_table, other.m_table);
    }
private:
    node_type* m_table;
    // fields below do not participate in swap() operation
    value_allocator_type& m_value_allocator;
    node_allocator_type& m_node_allocator;
};

}

#endif /* INCLUDE_HASH_TABLE_INTEGRAL_KEY_HPP_ */
