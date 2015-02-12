/*
 * hash_table_integral_pair.hpp
 *
 *  Created on: Feb 11, 2015
 *      Author: masha
 */

#ifndef INCLUDE_HASH_TABLE_INTEGRAL_PAIR_HPP_
#define INCLUDE_HASH_TABLE_INTEGRAL_PAIR_HPP_

#include "hash_node_integral_pair.hpp"
#include "cas.hpp"

#include <cassert>
#include <atomic>
#include <functional>

namespace lfds
{
template<class Key, class Value, class Hash, class Pred, class Allocator>
class hash_table_integral_pair: private hash_table_base
{
public:
    typedef hash_table_base base_type;
    typedef hash_table_integral_pair<Key, Value, Hash, Pred, Allocator> this_type;
    typedef Key key_type;
    typedef Value value_type;
    typedef Hash hash_type;
    typedef Pred equal_predicate_type;
    typedef hash_node_integral_pair<key_type, value_type> node_type;
    typedef typename base_type::size_type size_type;
    typedef typename Allocator::template rebind<node_type>::other node_allocator_type;

    static const bool INTEGRAL_KEY = true;
    static const bool INTEGRAL_KEYVALUE = true;

private:
    typedef typename node_type::state_type state_type;

public:
    hash_table_integral_pair(const size_type reserve,
            node_allocator_type& node_allocator) :
            base_type(reserve), m_node_allocator(node_allocator)
    {
        m_table = m_node_allocator.allocate(m_capacity);

        for (size_type i = 0; i < m_capacity; ++i)
        {
            m_node_allocator.construct(&m_table[i]);
        }
    }
    ~hash_table_integral_pair()
    {
        // remove volatile
        node_type* table = const_cast<node_type*>(m_table);
        m_node_allocator.deallocate(table, m_capacity);
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
            const node_type node = m_table[i];

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
            advance_index(i);
        } while (i != start);

        return false;
    }
    bool insert(const key_type key, const value_type value)
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
            const node_type node = m_table[i];

            switch (node.m_data.m_state)
            {
            case node_type::unused:
            {
                // the slot is empty so try to use it
                const node_type new_node =
                { node_type::allocated, key, value };

                if (m_table[i].atomic_cas(node, new_node))
                {
                    m_size.fetch_add(1, std::memory_order_relaxed);
                    m_used.fetch_add(1, std::memory_order_relaxed);
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
                    { node_type::allocated, key, value };

                    if (m_table[i].atomic_cas(node, new_node))
                    {
                        m_size.fetch_add(1, std::memory_order_relaxed);
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
            const node_type node = m_table[i];

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

                    volatile state_type & state = m_table[i].m_data.m_state;
                    static const state_type allocated = node_type::allocated;
                    static const state_type touched = node_type::touched;

                    if (atomic_cas(state, allocated, touched))
                    {
                        m_size.fetch_sub(1, std::memory_order_relaxed);
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
        this_type buffer(new_size, m_node_allocator);

        // assume exclusive access so remove volatile
        const node_type* table = const_cast<node_type*>(m_table);
        for (size_type i = 0; i < m_capacity; ++i)
        {
            // remove volatile because of e
            const node_type& node = table[i];

            if (node_type::allocated == node.m_data.m_state)
            {
                buffer.insert_unique_key(node);
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
    void insert_unique_key(const node_type& new_node)
    {
        hash_type hash_func;
        size_type hash = hash_func(new_node.m_data.m_key);
        size_type start = hash % m_capacity;
        size_type i = start;

        // assume exclusive access so remove volatile
        node_type* table = const_cast<node_type*>(m_table);

        do
        {
            node_type& node = table[i];
            if (node_type::unused == node.m_data.m_state)
            {
                node = new_node;

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
    volatile node_type* m_table;
    // fields below do not participate in swap() operation
    node_allocator_type& m_node_allocator;
};

}

#endif /* INCLUDE_HASH_TABLE_INTEGRAL_PAIR_HPP_ */
