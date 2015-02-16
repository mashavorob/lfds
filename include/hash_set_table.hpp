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
#include "ref_lock.hpp"
#include "cas.hpp"

#include <cassert>

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
class hash_set_table: private hash_table_base
{
public:
    typedef hash_table_base base_type;
    typedef hash_set_table<Key, Hash, Pred, Allocator> this_type;
    typedef Key key_type;
    typedef Hash hash_type;
    typedef Pred equal_predicate_type;
    typedef std::size_t size_type;
    typedef hash_set_node<key_type> node_type;
    typedef typename Allocator::template rebind<key_type>::other key_allocator_type;
    typedef typename Allocator::template rebind<node_type>::other node_allocator_type;

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
    hash_set_table(const size_type reserve, key_allocator_type& key_allocator,
            node_allocator_type& node_allocator) :
            base_type(reserve), m_key_allocator(key_allocator), m_node_allocator(
                    node_allocator)
    {
        m_table = m_node_allocator.allocate(m_capacity);

        for (size_type i = 0; i < m_capacity; ++i)
        {
            m_node_allocator.construct(&m_table[i]);
        }
    }
    ~hash_set_table()
    {
        for (size_type i = 0; i < m_capacity; ++i)
        {
            node_type& node = m_table[i];
            hash_item_type item = node.m_hash;
            // pending and pending2 states are not allowed here
            assert(
                    item.m_state == hash_item_type::allocated
                            || item.m_state == hash_item_type::touched
                            || item.m_state == hash_item_type::unused);
            if (item.m_state == hash_item_type::touched)
            {
                m_key_allocator.destroy(node.key());
            }
        }
        m_node_allocator.deallocate(m_table, m_capacity);
    }

    bool find(const key_type & key) const
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
            advance_index(i);
        } while (i != start);

        return false;
    }

    bool insert(const key_type & key)
    {
        check_watermark(
                std::bind(&this_type::rehash, this, std::placeholders::_1));

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
            const hash_item_type item = node.m_hash;

            switch (item.m_state)
            {
            case hash_item_type::unused:
            {
                // the slot is empty so try to use it
                hash_item_type new_item =
                { hash, hash_item_type::pending };

                if (atomic_cas(node.m_hash, item, new_item))
                {
                    m_key_allocator.construct(node.key(), key);

                    m_size.fetch_add(1, std::memory_order_relaxed);
                    m_used.fetch_add(1, std::memory_order_relaxed);

                    node.m_hash.m_state = hash_item_type::allocated;
                    std::atomic_thread_fence(std::memory_order_release);
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
                        m_size.fetch_add(1, std::memory_order_relaxed);
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

    bool erase(const key_type & key)
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
                        m_size.fetch_sub(1, std::memory_order_relaxed);
                        return true;
                    }
                    // the item found but it is being erased in an other thread;
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
    void rehash(const size_type new_capacity)
    {
        this_type buffer(new_capacity, m_key_allocator, m_node_allocator);

        for (size_type i = 0; i < m_capacity; ++i)
        {
            node_type& node = m_table[i];
            const hash_item_type item = node.m_hash;

            if (item.m_state == hash_item_type::allocated)
            {
                key_type & key = *node.key();
                buffer.insert_unique_key(item.m_hash,
                        std::forward<key_type>(key));
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
    void insert_unique_key(const size_type hash, key_type && key)
    {
        size_type start = hash % m_capacity;
        size_type i = start;

        do
        {
            node_type& node = m_table[i];
            const hash_item_type item = node.m_hash;
            if (item.m_state == hash_item_type::unused)
            {
                node.m_hash.m_hash = hash;
                node.m_hash.m_state = hash_item_type::allocated;

                m_key_allocator.construct(node.key(),
                        std::forward<key_type>(key));

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
    key_allocator_type& m_key_allocator;
    node_allocator_type& m_node_allocator;
};

}

#endif /* INCLUDE_HASH_SET_TABLE_HPP_ */
