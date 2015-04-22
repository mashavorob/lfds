/*
 * hash_table_base.hpp
 *
 *  Created on: Feb 6, 2015
 *      Author: masha
 */

#ifndef INCLUDE_HASH_TABLE_BASE_HPP_
#define INCLUDE_HASH_TABLE_BASE_HPP_

#include "ref_ptr.hpp"
#include "ref_lock.hpp"
#include "xtomic.hpp"
#include "cppbasics.hpp"

#include <cassert>
#include <cstddef>

namespace lfds
{

// basic constants for all hash tables
template<class HashTable>
class hash_table_base
{
public:
    static const unsigned int HIGH_WATERMARK_MULT = 5;
    static const unsigned int HIGH_WATERMARK_DIV = 10;
    static const unsigned int MIN_CAPACITY = 20;

    typedef hash_table_base<HashTable> this_type;

    typedef HashTable hash_table_type;
    typedef typename hash_table_type::key_type key_type;
    typedef typename hash_table_type::node_type node_type;
    typedef typename hash_table_type::allocator_type allocator_type;
    typedef typename hash_table_type::size_type size_type;
    typedef typename hash_table_type::table_type table_type; // aka raw_hash_table<node_type>
    typedef typename hash_table_type::snapshot_type snapshot_type;

    typedef ref_ptr<table_type> table_ref_ptr_type;
    typedef typename table_ref_ptr_type::size_type counter_type;
    typedef typename table_ref_ptr_type::atomic_ptr_type atomic_table_ptr_type;
    typedef typename allocator_type::template rebind<node_type>::other node_allocator_type;

    // the class reserves extra space for insert operation
    class insert_counter
    {
    public:
        insert_counter() :
                m_count(0)
        {

        }
        counter_type addRef() const
        {
            return ++m_count;
        }
        counter_type release() const
        {
            return --m_count;
        }
        counter_type get() const
        {
            return m_count.load(barriers::relaxed);
        }
    private:
        mutable xtomic<counter_type> m_count;
    };

    typedef ref_lock<const table_ref_ptr_type> scoped_lock_type;
    typedef ref_lock<const insert_counter> scoped_reserver_type;

    static size_type adjustCapacity(size_type capacity)
    {
        return std::max(capacity, static_cast<size_type>(MIN_CAPACITY));
    }
    static size_type calcWatermark(size_type capacity)
    {
        return adjustCapacity(capacity) * HIGH_WATERMARK_MULT
                / HIGH_WATERMARK_DIV;
    }
    static size_type calcCapacity(size_type watermark)
    {
        return adjustCapacity(watermark * HIGH_WATERMARK_DIV
                / HIGH_WATERMARK_MULT) + 32; // 32 is good number to compensate concurrent insertions
    }

    //
    // template<class CompatibleKey, Key>
    // struct CompatiblePredicate : public binary_function<CompatibleKey, Key, bool>
    // {
    //      bool operator()(const CompatibleKey & key, const Key & value) const;
    //      ...
    // };

private:
    hash_table_base(const this_type&); // = delete;
    this_type& operator=(const this_type&); // = delete;
public:
    hash_table_base(hash_table_type & hashTable, size_type reserve) :
            m_constTable(), m_mutableTable(), m_currTable(0), m_hashTable(
                    hashTable)
    {
        table_type* ptr = &m_tables[m_currTable];
        init_table(ptr, calcCapacity(reserve));
        m_constTable.m_ptr.store(ptr, barriers::relaxed);
        m_mutableTable.m_ptr.store(ptr, barriers::relaxed);
    }
    ~hash_table_base()
    {
        table_type* ptr = m_constTable.m_ptr.load(barriers::relaxed);
        assert(ptr);
        assert(ptr == m_mutableTable.m_ptr.load(barriers::relaxed));
        destroy_table(ptr);
    }
public:
    void getSnapshot(snapshot_type & snapshot) const
    {
        snapshot_type tmp;
        {
            // attempt to make a wait free find
            scoped_lock_type guard(m_constTable);
            const table_type* ptr = m_constTable.m_ptr.load(
                    barriers::relaxed);

            tmp.reserve(ptr->m_size.load(barriers::relaxed));
            m_hashTable.getSnapshot_imp(*ptr, tmp);
        }
        snapshot.swap(tmp);
    }
    bool erase(const key_type & key)
    {
        table_type* ptr = acquire_table(m_mutableTable);
        scoped_lock_type guard(m_mutableTable, false);
        return m_hashTable.erase_impl(*ptr, key);
    }
    static table_type* acquire_table(table_ref_ptr_type & r_ptr)
    {
        table_type * ptr;

        do
        {
            ptr = r_ptr.m_ptr.load(barriers::relaxed);
        } while (!ptr);
        ++r_ptr.m_refCount;
        table_type* ptr2 = r_ptr.m_ptr.load(barriers::relaxed);

        while (ptr != ptr2)
        {
            --r_ptr.m_refCount;
            do
            {
                ptr = r_ptr.m_ptr.load(barriers::relaxed);
            } while (!ptr);
            ++r_ptr.m_refCount;
            ptr2 = r_ptr.m_ptr.load(barriers::relaxed);
        }
        return ptr;
    }
    void init_table(table_type* ptr, size_type capacity)
    {
        ptr->m_capacity = capacity;
        ptr->m_highWatermark = calcWatermark(capacity);
        ptr->m_size.store(0, barriers::relaxed);
        ptr->m_used.store(0, barriers::relaxed);
        ptr->m_table = m_node_allocator.allocate(capacity);
        for (size_type i = 0; i < capacity; ++i)
        {
            ::new((void *)&ptr->m_table[i]) node_type();
        }
    }
    void destroy_table(table_type* ptr)
    {
        for (size_type i = 0; i < ptr->m_capacity; ++i)
        {
            node_type& node = ptr->m_table[i];
            m_hashTable.destroyNode_impl(node);
        }
        m_node_allocator.deallocate(ptr->m_table, ptr->m_capacity);
    }

    void check_watermark()
    {
        bool success;
        table_type * ptr;

        const table_ref_ptr_type zero_ptr;

        while (isAboveWatermark())
        {
            ptr = m_mutableTable.m_ptr.load(barriers::relaxed);
            table_ref_ptr_type expected(ptr, 0);
            success = ptr != nullptr
                    && m_mutableTable.atomic_cas(expected, zero_ptr);

            if (!success)
            {
                // try all over again
                continue;
            }

            // access captured
            constexpr int numTables = sizeof(m_tables) / sizeof(m_tables[0]);
            if (++m_currTable >= numTables)
            {
                m_currTable -= numTables;
            }

            table_type* next_ptr = &m_tables[m_currTable];
            init_table(next_ptr, ptr->m_capacity * 2);

            m_hashTable.rehash_impl(*ptr, *next_ptr);

            // switch readers to the new table
            table_ref_ptr_type new_ptr(next_ptr, 0);
            do
            {
                table_ref_ptr_type expected(ptr, 0);
                success = m_constTable.atomic_cas(expected, new_ptr);
            } while (!success);

            // all readers finished so destroy data
            destroy_table(ptr);

            do
            {
                success = m_mutableTable.atomic_cas(zero_ptr, new_ptr);
            } while (!success);
        }
    }
    size_type getCapacity() const
    {
        scoped_lock_type guard(m_constTable);
        const table_type* ptr = m_constTable.m_ptr.load(
                barriers::relaxed);
        return ptr->m_capacity;
    }
    size_type size() const
    {
        scoped_lock_type guard(m_constTable);
        const table_type* ptr = m_constTable.m_ptr.load(
                barriers::relaxed);
        return ptr->m_size.load(barriers::relaxed);
    }
    size_type getUsed() const
    {
        scoped_lock_type guard(m_constTable);
        const table_type* ptr = m_constTable.m_ptr.load(
                barriers::relaxed);
        return ptr->m_used;
    }
    size_type getHighWatermark() const
    {
        scoped_lock_type guard(m_constTable);
        const table_type* ptr = m_constTable.m_ptr.load(
                barriers::relaxed);
        return ptr->m_highWatermark;
    }
    bool isAboveWatermark()
    {
        scoped_lock_type guard(m_constTable);
        const table_type* ptr = m_constTable.m_ptr.load(
                barriers::relaxed);
        return (ptr->m_used.load(barriers::relaxed) + m_concurrentInsertions.get())
                >= ptr->m_highWatermark;
    }
protected:
    table_ref_ptr_type m_constTable;
    table_ref_ptr_type m_mutableTable;
    insert_counter m_concurrentInsertions;
    hash_table_type& m_hashTable;

private:
    table_type m_tables[2];
    size_type m_currTable;
    node_allocator_type m_node_allocator;
};
}

#endif /* INCLUDE_HASH_TABLE_BASE_HPP_ */
