/*
 * hash_table.hpp
 *
 *  Created on: Jan 23, 2015
 *      Author: masha
 */

#ifndef INCLUDE_HASH_TABLE_HPP_
#define INCLUDE_HASH_TABLE_HPP_

#include "hash_node.hpp"
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

namespace
{
template<class Key, class Value>
struct hash_data_table
{
    typedef Key key_type;
    typedef Value value_type;
    typedef std::size_t size_type;
    typedef hash_node<key_type, value_type> node_type;
    typedef unsigned int counter_type;

    node_type* m_table;
    size_type m_capacity;
    mutable std::atomic<unsigned int> m_refCounter;
};

class ref_guard
{
public:
    ref_guard(std::atomic<unsigned int>& c) :
            m_c(c)
    {

    }
    ~ref_guard()
    {
        --m_c;
    }
private:
    std::atomic<unsigned int> & m_c;
};
}

template<class Key, class Value, class Hash, class Pred, class Allocator>
class hash_table: private hash_table_base
{
public:
    typedef hash_table_base base_type;
    typedef hash_table<Key, Value, Hash, Pred, Allocator> this_type;
    typedef Key key_type;
    typedef Value value_type;
    typedef Hash hash_type;
    typedef Pred equal_predicate_type;
    typedef std::size_t size_type;
    typedef hash_data_table<key_type, value_type> table_type;
    typedef std::atomic<table_type*> atomic_table_ptr_type;
    typedef hash_node<key_type, value_type> node_type;
    typedef Allocator value_allocator_type;
    typedef typename Allocator::template rebind<key_type>::other key_allocator_type;
    typedef typename Allocator::template rebind<node_type>::other node_allocator_type;
    typedef typename table_type::counter_type counter_type;

    static const bool INTEGRAL_KEY = false;
    static const bool INTEGRAL_KEYVALUE = false;

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
    hash_table(const size_type reserve, key_allocator_type& key_allocator,
            value_allocator_type& value_allocator,
            node_allocator_type& node_allocator) :
            m_constTable(nullptr), m_mutableTable(nullptr), m_currTable(0), m_mutableOps(
                    0), base_type(reserve), m_key_allocator(key_allocator), m_value_allocator(
                    value_allocator), m_node_allocator(node_allocator)
    {
        table_type* ptr = &m_tables[m_currTable];
        init_table(ptr, m_capacity);
        m_constTable.store(ptr, std::memory_order_relaxed);
        m_mutableTable.store(ptr, std::memory_order_relaxed);
    }
    ~hash_table()
    {
        table_type* ptr = m_constTable.load(std::memory_order_relaxed);
        assert(ptr);
        assert(ptr == m_mutableTable.load(std::memory_order_relaxed));
        destroy_table(ptr);
    }
    bool find(const key_type & key, value_type & value) const
    {
        // attempt to make a wait free find

        hash_type hash_func;
        equal_predicate_type eq_func;

        std::size_t hash = hash_func(key);

        const table_type* ptr = acquire_table(m_constTable);
        const node_type* table = ptr->m_table;
        const size_type capacity = ptr->m_capacity;
        ref_guard table_guard(ptr->m_refCounter);

        size_type i = hash % capacity;

        for (;;advance_index(i, capacity))
        {
            const node_type& node = table[i];
            const hash_item_type item = node.get_hash();

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
                if (eq_func(key, *node.key()))
                {
                    // the item is being processed now, there is no reason to wait
                    return false;
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
                    scoped_ref_lock guard(node);
                    std::size_t state = node.get_state();
                    if (state == hash_item_type::allocated)
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
    bool insert(const key_type & key, Args&&... val)
    {
        check_watermark();

        hash_type hash_func;
        equal_predicate_type eq_func;

        std::size_t hash = hash_func(key);

        table_type* ptr = acquire_table(m_mutableTable);
        ++m_mutableOps;

        node_type* table = ptr->m_table;
        const size_type capacity = ptr->m_capacity;
        ref_guard table_guard(ptr->m_refCounter);
        ref_guard op_guard(m_mutableOps);

        size_type i = hash % capacity;

        for(;;advance_index(i, capacity))
        {
            node_type& node = table[i];
            const hash_item_type item = node.get_hash();

            switch (item.m_state)
            {
            case hash_item_type::unused:
            {
                // the slot is empty so try to use it
                hash_item_type new_item =
                { hash, hash_item_type::pending };

                if (node.atomic_cas_hash(item, new_item))
                {
                    m_value_allocator.construct(node.value(),
                            std::forward<Args>(val)...);
                    m_key_allocator.construct(node.key(), key);

                    m_size.fetch_add(1, std::memory_order_relaxed);
                    m_used.fetch_add(1, std::memory_order_relaxed);

                    node.set_state(hash_item_type::allocated);
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
            case hash_item_type::pending2:
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
                    hash_item_type new_item =
                    { hash, hash_item_type::pending2 };

                    if (node.atomic_cas_hash(item, new_item))
                    {
                        m_value_allocator.construct(node.value(),
                                std::forward<Args>(val)...);
                        m_size.fetch_add(1, std::memory_order_relaxed);
                        node.set_state(hash_item_type::allocated);
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
        }
        throw std::bad_alloc();
        return false;
    }

    bool erase(const key_type & key)
    {
        hash_type hash_func;
        equal_predicate_type eq_func;

        std::size_t hash = hash_func(key);

        table_type* ptr = acquire_table(m_mutableTable);
        ++m_mutableOps;

        node_type* table = ptr->m_table;
        const size_type capacity = ptr->m_capacity;
        ref_guard table_guard(ptr->m_refCounter);
        ref_guard op_guard(m_mutableOps);

        size_type i = hash % capacity;

        for(;;advance_index(i, capacity))
        {
            node_type& node = table[i];
            const hash_item_type item = node.get_hash();

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
                if (eq_func(key, *node.key()))
                {
                    // the item is being processed now, there is no reason to wait
                    return false;
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
                    // reset readiness
                    hash_item_type new_item =
                    { hash, hash_item_type::pending2 };

                    if (node.atomic_cas_hash(item, new_item))
                    {
                        // wait for pending finds
                        node.wait_for_release();
                        // destroy the node
                        m_value_allocator.destroy(node.value());
                        node.set_state(hash_item_type::touched);
                        m_size.fetch_sub(1, std::memory_order_relaxed);
                        std::atomic_thread_fence(std::memory_order_release);
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

    size_type size() const
    {
        return m_size;
    }
    size_type capacity() const
    {
        return m_capacity;
    }
private:
    static table_type* acquire_table(atomic_table_ptr_type & a_ptr)
    {
        table_type* ptr;

        for (;;)
        {
            ptr = a_ptr.load(std::memory_order_relaxed);
            if ( !ptr )
            {
                continue;
            }
            ++ptr->m_refCounter;
            table_type* ptr2 = a_ptr.load(std::memory_order_relaxed);
            if (ptr == ptr2)
            {
                break;
            }
            --ptr->m_refCounter;
        }
        return ptr;
    }
    static const table_type* acquire_table(
            const atomic_table_ptr_type & a_ptr)
    {
        table_type* ptr;

        for (;;)
        {
            ptr = a_ptr.load(std::memory_order_relaxed);
            if ( !ptr )
            {
                continue;
            }
            ++ptr->m_refCounter;
            table_type* ptr2 = a_ptr.load(std::memory_order_relaxed);
            if (ptr == ptr2)
            {
                break;
            }
            --ptr->m_refCounter;
        }
        return ptr;
    }
    void init_table(table_type* ptr, size_type capacity)
    {
        ptr->m_capacity = capacity;
        ptr->m_table = m_node_allocator.allocate(capacity);

        for (size_type i = 0; i < ptr->m_capacity; ++i)
        {
            m_node_allocator.construct(&ptr->m_table[i]);
        }
        ptr->m_refCounter.store(0, std::memory_order_relaxed);
    }
    void destroy_table(table_type* ptr)
    {
        counter_type refCount;
        do
        {
            refCount = ptr->m_refCounter.load(std::memory_order_relaxed);
        } while (refCount > 0);

        for (size_type i = 0; i < ptr->m_capacity; ++i)
        {
            node_type& node = ptr->m_table[i];
            hash_item_type item = node.get_hash();
            // pending and pending2 states are not allowed here
            assert(
                    item.m_state == hash_item_type::allocated
                            || item.m_state == hash_item_type::touched
                            || item.m_state == hash_item_type::unused);
            if (item.m_state == hash_item_type::allocated)
            {
                m_value_allocator.destroy(node.value());
                item.m_state = hash_item_type::touched;
            }
            if (item.m_state == hash_item_type::touched)
            {
                m_key_allocator.destroy(node.key());
            }
        }
        m_node_allocator.deallocate(ptr->m_table, m_capacity);
    }
    void check_watermark()
    {
        while ( m_used >= m_high_watermark )
        {
            table_type* ptr = m_mutableTable.load(std::memory_order_relaxed);
            if ( !ptr )
            {
                continue;
            }
            static constexpr table_type* null_ptr = nullptr;
            bool res = m_mutableTable.compare_exchange_weak(ptr, null_ptr, std::memory_order_relaxed);
            if ( !res )
            {
                continue;
            }
            // access captured
            advance_index(m_currTable, 2);

            table_type* next_ptr = &m_tables[m_currTable];
            init_table(next_ptr, m_capacity*2);

            // wait for all mutable ops
            while ( m_mutableOps.load(std::memory_order_relaxed) );

            rehash(ptr, next_ptr);

            // switch readers to the new table
            m_constTable.store(next_ptr, std::memory_order_relaxed);

            // wait for all const operations
            while ( ptr->m_refCounter.load(std::memory_order_relaxed) );

            // all readers finished so destroy data
            destroy_table(ptr);

            // now it is possible to release all waiting mutable operations
            m_capacity = next_ptr->m_capacity;
            size_type used = m_size.load(std::memory_order_relaxed);
            m_high_watermark = calcWatermark(m_capacity);
            m_used.store(used, std::memory_order_relaxed);
            m_mutableTable.store(next_ptr, std::memory_order_relaxed);
        }
    }
    // the function assumes exclusive access
    void rehash(const table_type* src, table_type* dst)
    {
        for (size_type i = 0; i < src->m_capacity; ++i)
        {
            node_type& node = src->m_table[i];
            const hash_item_type item = node.get_hash();

            if (item.m_state == hash_item_type::allocated)
            {
                key_type & key = *node.key();
                value_type & val = *node.value();

                insert_unique_key(dst, item.m_hash,
                        std::forward<key_type>(key),
                        std::forward<value_type>(val));
            }
        }
    }

    // simplified form of insert()
    // the function assumes:
    //    * exclusive access to the container
    //    * new key is unique
    //    * table has enough capacity to insert specified element
    void insert_unique_key(table_type* dst, const size_type hash, key_type && key,
            value_type && val)
    {
        const size_type capacity = dst->m_capacity;
        size_type i = hash % capacity;

        for(;;advance_index(i, capacity))
        {
            node_type& node = dst->m_table[i];
            hash_item_type item = node.get_hash();
            if (item.m_state == hash_item_type::unused)
            {
                node.set_item(hash, hash_item_type::allocated);

                m_key_allocator.construct(node.key(),
                        std::forward<key_type>(key));
                m_value_allocator.construct(node.value(),
                        std::forward<value_type>(val));
                break;
            }
        }
    }
private:

    table_type m_tables[2];
    atomic_table_ptr_type m_constTable;
    atomic_table_ptr_type m_mutableTable;
    size_type m_currTable;
    std::atomic<counter_type> m_mutableOps;

    // fields below do not participate in swap() operation
    key_allocator_type& m_key_allocator;
    value_allocator_type& m_value_allocator;
    node_allocator_type& m_node_allocator;
};

}

#endif /* INCLUDE_HASH_TABLE_HPP_ */
