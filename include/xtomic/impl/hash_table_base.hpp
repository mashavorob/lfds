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
#include <xtomic/aux/cppbasics.hpp>
#include <xtomic/quantum.hpp>

#include <cassert>
#include <cstddef>
#include <algorithm>

namespace xtomic
{

namespace
{
template<typename Value>
struct LinkedEnvelop: public Value
{

    typedef LinkedEnvelop<Value> this_type;
    typedef Value value_type;
    typedef std::size_t count_type;

    mutable xtomic::quantum<count_type> m_writeCount;

    this_type* m_link;

    LinkedEnvelop() :
            Value(),
            m_link(nullptr)
    {

    }
};

template<typename Value>
struct align_4_cas16 CountedEnvelop: public Value
{

    typedef CountedEnvelop<Value> this_type;
    typedef Value value_type;
    typedef std::size_t count_type;

    mutable xtomic::quantum<count_type> m_readCount;
    mutable xtomic::quantum<count_type> m_writeCount;

    CountedEnvelop() :
            Value(),
            m_readCount(0),
            m_writeCount(0)
    {

    }
};

template<typename Value>
struct align_4_cas16 CountedPtr
{
    typedef CountedPtr<Value> this_type;
    typedef Value value_type;
    typedef typename value_type::count_type count_type;
    typedef value_type* ptr_type;
    typedef const value_type* const_ptr_type;

    xtomic::quantum<ptr_type> m_ptr;
    mutable xtomic::quantum<count_type> m_count;

    CountedPtr(ptr_type ptr = nullptr, count_type count = 0) :
            m_ptr(ptr),
            m_count(count)
    {

    }

    ptr_type aquire()
    {
        bool res;
        ptr_type ptr;
        do
        {
            ptr = m_ptr.load(barriers::acquire);
            if (!ptr)
            {
                continue;
            }
            count_type count = m_count.load(barriers::acquire);

            this_type expected(ptr, count);
            this_type next(ptr, ++count);
            res = xtomic::atomic_cas(*this, expected, next);
        }
        while (!res);
        return ptr;
    }
    const_ptr_type aquire() const
    {
        return const_cast<this_type*>(this)->aquire();
    }
    bool tryReset(ptr_type expected, ptr_type ptr, this_type& prev)
    {
        prev.m_ptr.store(expected, barriers::release);
        prev.m_count.store(m_count.load(barriers::acquire), barriers::release);
        this_type next(ptr, 0);
        bool res = xtomic::atomic_cas(*this, prev, next);
        return res;
    }
    void reset(ptr_type expected, ptr_type ptr, this_type& prev)
    {
        bool res;
        do
        {
            res = tryReset(expected, ptr, prev);
        }
        while (!res);
    }
    void reset(this_type& expected, ptr_type ptr, this_type& prev)
    {
        reset(expected.m_ptr.load(barriers::relaxed), ptr, prev);
    }
    void waitFor(xtomic::quantum<count_type>& count)
    {
        count_type expected = m_count.load(barriers::acquire);
        count_type actual = count.load(barriers::acquire);
        while (actual != expected)
        {
            actual = count.load(barriers::acquire);
        }
    }
    void waitForReads()
    {
        ptr_type p = m_ptr.load(barriers::acquire);
        waitFor(p->m_readCount);
    }
    void waitForWrites()
    {
        ptr_type p = m_ptr.load(barriers::acquire);
        waitFor(p->m_writeCount);
    }
};

template<typename Base>
class const_table_guard
{
public:
    typedef const_table_guard<Base> this_type;
    typedef Base base_type;
    typedef typename base_type::table_type table_type;
public:
    const_table_guard(const base_type& base, const table_type*& ptr) :
            m_base(base),
            m_ptr(base.acquireConstTable())
    {
        ptr = m_ptr;
    }
    ~const_table_guard()
    {
        m_base.releaseConstTable(m_ptr);
    }
private:
    const base_type& m_base;
    const table_type* m_ptr;
};

template<typename Base>
class mutable_table_guard
{
public:
    typedef const_table_guard<Base> this_type;
    typedef Base base_type;
    typedef typename base_type::table_type table_type;
public:
    mutable_table_guard(base_type& base, table_type*& ptr) :
            m_base(base),
            m_ptr(base.acquireMutableTable())
    {
        ptr = m_ptr;
    }
    ~mutable_table_guard()
    {
        m_base.releaseMutableTable(m_ptr);
    }
private:
    base_type& m_base;
    table_type* m_ptr;
};

// the class reserves extra space for insert operation
class insert_guard
{
public:
    typedef std::size_t count_type;

    insert_guard() :
            m_count(0)
    {

    }
    count_type addRef() const
    {
        return ++m_count;
    }
    count_type release() const
    {
        return --m_count;
    }
    count_type get() const
    {
        return m_count.load(barriers::relaxed);
    }
private:
    mutable xtomic::quantum<count_type> m_count;
};
}

template<typename HashTable>
class root_hash_table
{
public:
    typedef root_hash_table<HashTable> this_type;
    typedef HashTable hash_table_type;
    typedef typename hash_table_type::table_type table_type; // aka raw_hash_table<node_type>
    typedef typename hash_table_type::node_type node_type;
    typedef typename hash_table_type::size_type size_type;
    typedef typename hash_table_type::allocator_type allocator_type;
    typedef typename allocator_type::template rebind<node_type>::other node_allocator_type;

    static const unsigned int HIGH_WATERMARK_MULT = 7;
    static const unsigned int HIGH_WATERMARK_DIV = 10;
    static const unsigned int MIN_CAPACITY = 20;

public:
    root_hash_table(hash_table_type & hashTable) :
            m_hashTable(hashTable)
    {

    }
private:
    root_hash_table(const this_type&); // = delete
    this_type& operator=(const this_type&); // = delete

public:
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
        return adjustCapacity(
                watermark * HIGH_WATERMARK_DIV / HIGH_WATERMARK_MULT) + 32; // 32 is good number to compensate concurrent insertions
    }

    void destroyNodes(table_type* ptr)
    {
        for (size_type i = 0; i < ptr->m_capacity; ++i)
        {
            node_type& node = ptr->m_table[i];
            m_hashTable.destroyNode_impl(node);
        }
        m_nodeAllocator.deallocate(ptr->m_table, ptr->m_capacity);
    }
    void initNodes(table_type* ptr, size_type capacity)
    {
        ptr->m_capacity = capacity;
        ptr->m_highWatermark = calcWatermark(capacity);
        ptr->m_size.store(0, barriers::relaxed);
        ptr->m_used.store(0, barriers::relaxed);
        ptr->m_table = m_nodeAllocator.allocate(capacity);
        for (size_type i = 0; i < capacity; ++i)
        {
            ::new ((void *) &ptr->m_table[i]) node_type();
        }
    }

protected:
    node_allocator_type m_nodeAllocator;
    hash_table_type& m_hashTable;
};

template<typename HashTable>
class greedy_hash_table_base: protected root_hash_table<HashTable>
{
public:
    typedef HashTable hash_table_type;
    typedef root_hash_table<HashTable> base_type;
    typedef typename hash_table_type::table_type table_type; // aka raw_hash_table<node_type>
    typedef typename hash_table_type::allocator_type allocator_type;
    typedef typename hash_table_type::size_type size_type;

    typedef LinkedEnvelop<table_type> linked_table_type;
    typedef typename allocator_type::template rebind<linked_table_type>::other table_allocator_type;
    typedef table_type* cookie_type;

private:
    typedef xtomic::quantum<linked_table_type*> linked_table_ptr_type;
    typedef xtomic::quantum<table_type*> table_ptr_type;
    typedef CountedPtr<linked_table_type> table_cptr_type;
public:

    greedy_hash_table_base(hash_table_type & hashTable,
                           const size_type watermark) :
            root_hash_table<HashTable>(hashTable)
    {
        const size_type capacity = base_type::calcCapacity(watermark);
        table_type* ptr = allocateTable(capacity);
        linked_table_type* envelop = static_cast<linked_table_type*>(ptr);
        m_constTable.store(ptr, barriers::relaxed);
        m_mutableTable.m_ptr.store(envelop, barriers::relaxed);
    }
    ~greedy_hash_table_base()
    {
        linked_table_type* ltable = m_usedTables.load(barriers::relaxed);
        while (ltable)
        {
            linked_table_type* next = ltable->m_link;
            base_type::destroyNodes(ltable);
            m_tableAllocator.destroy(ltable);
            m_tableAllocator.deallocate(ltable, 1);
            ltable = next;
        }
    }

    table_type* allocateTable(const size_type capacity)
    {
        linked_table_type* table = m_tableAllocator.allocate(1);
        // allocators did not support default constructors before c++11
        ::new (static_cast<void*>(table)) linked_table_type();
        base_type::initNodes(table, capacity);
        return table;
    }

    void deallocateTable(table_type* table)
    {
        linked_table_type* envelop = static_cast<linked_table_type*>(table);

        bool res;
        do
        {
            envelop->m_link = m_usedTables.load(barriers::relaxed);
            res = m_usedTables.atomic_cas(envelop->m_link, envelop);
        }
        while (!res);
    }
    const table_type* acquireConstTable() const
    {
        return m_constTable.load(barriers::relaxed);
    }
    void releaseConstTable(const table_type*) const
    {
    }
    table_type* acquireMutableTable()
    {
        return m_mutableTable.aquire();
    }
    void releaseMutableTable(table_type* ptr) const
    {
        const linked_table_type* envelop =
                static_cast<const linked_table_type*>(ptr);
        ++envelop->m_writeCount;
    }
    bool startRehashing(table_type*& ptr, cookie_type& cookie)
    {
        linked_table_type* envelop = m_mutableTable.m_ptr.load(
                barriers::relaxed);
        if (!envelop)
        {
            return false;
        }
        ptr = envelop;
        table_cptr_type state;
        bool res = m_mutableTable.tryReset(envelop, nullptr, state);
        if (res)
        {
            // wait for pending writes
            state.waitForWrites();
        }
        return res;
    }
    void finalizeRehashing(table_type* ptr, cookie_type)
    {
        m_constTable.store(ptr, barriers::relaxed);
        linked_table_type* envelop = static_cast<linked_table_type*>(ptr);
        envelop->m_writeCount.store(0, barriers::relaxed);
        m_mutableTable.m_count.store(0, barriers::relaxed);
        m_mutableTable.m_ptr.store(envelop, barriers::release);

    }
    void cancelRehashing(table_type* ptr)
    {
        linked_table_type* envelop = static_cast<linked_table_type*>(ptr);
        m_mutableTable.m_ptr.store(envelop, barriers::release);
    }
private:
    table_ptr_type m_constTable;
    table_cptr_type m_mutableTable;
    linked_table_ptr_type m_usedTables;

    table_allocator_type m_tableAllocator;
};

template<typename HashTable>
class wise_hash_table_base: protected root_hash_table<HashTable>
{
public:
    typedef HashTable hash_table_type;
    typedef root_hash_table<HashTable> base_type;
    typedef typename hash_table_type::table_type table_type; // aka raw_hash_table<node_type>
    typedef typename hash_table_type::allocator_type allocator_type;
    typedef typename hash_table_type::size_type size_type;

    typedef CountedEnvelop<table_type> counted_table_type;
    typedef CountedPtr<counted_table_type> table_ptr_type;
    typedef table_ptr_type cookie_type;
    typedef typename allocator_type::template rebind<counted_table_type>::other table_allocator_type;

    wise_hash_table_base(hash_table_type & hashTable, const size_type watermark) :
            root_hash_table<HashTable>(hashTable)
    {
        const size_type capacity = base_type::calcCapacity(watermark);
        counted_table_type* table =
                static_cast<counted_table_type*>(allocateTable(capacity));
        m_constTable.m_ptr.store(table, barriers::relaxed);
        m_mutableTable.m_ptr.store(table, barriers::relaxed);
    }
    ~wise_hash_table_base()
    {
    }

    table_type* allocateTable(const size_type capacity)
    {
        counted_table_type* table = m_tableAllocator.allocate(1);
        // allocators did not support default constructors before c++11
        ::new (static_cast<void*>(table)) counted_table_type();
        base_type::initNodes(table, capacity);
        return table;
    }

    void deallocateTable(table_type* ptr)
    {
        base_type::destroyNodes(ptr);
        counted_table_type* envelop = static_cast<counted_table_type*>(ptr);
        m_tableAllocator.destroy(envelop);
        m_tableAllocator.deallocate(envelop, 1);
    }
    const table_type* acquireConstTable() const
    {
        return m_constTable.aquire();
    }
    void releaseConstTable(const table_type* table) const
    {
        const counted_table_type* evelop =
                static_cast<const counted_table_type*>(table);
        ++evelop->m_readCount;
    }
    table_type* acquireMutableTable()
    {
        return m_mutableTable.aquire();
    }
    void releaseMutableTable(table_type* table) const
    {
        const counted_table_type* envelop =
                static_cast<const counted_table_type*>(table);
        ++envelop->m_writeCount;
    }
    bool startRehashing(table_type*& ptr, cookie_type & cookie)
    {
        counted_table_type* envelop = m_mutableTable.m_ptr.load(
                barriers::relaxed);
        if (!envelop)
        {
            return false;
        }
        ptr = envelop;
        bool res = m_mutableTable.tryReset(envelop, nullptr, cookie);
        if (res)
        {
            // wait for pending writes
            cookie.waitForWrites();
        }
        return res;
    }
    void finalizeRehashing(table_type* ptr, cookie_type & cookie)
    {
        cookie_type readCookie;

        counted_table_type* envelop = static_cast<counted_table_type*>(ptr);
        envelop->m_readCount.store(0, barriers::relaxed);
        envelop->m_writeCount.store(0, barriers::relaxed);
        m_constTable.reset(cookie, envelop, readCookie);
        m_mutableTable.m_count.store(0, barriers::relaxed);
        m_mutableTable.m_ptr.store(envelop, barriers::release);

        readCookie.waitForReads();
    }
    void cancelRehashing(table_type* ptr)
    {
        counted_table_type* envelop = static_cast<counted_table_type*>(ptr);
        m_mutableTable.m_ptr.store(envelop, barriers::release);
    }
private:
    table_ptr_type m_constTable;
    table_ptr_type m_mutableTable;

    table_allocator_type m_tableAllocator;
};

template<typename Base>
class hash_table_base: protected Base
{
private:
    typedef Base base_type;

protected:
    typedef const_table_guard<base_type> const_guard_type;
    typedef mutable_table_guard<base_type> mutable_guard_type;
    typedef insert_guard insert_guard_type;
    typedef ref_lock<const insert_guard> scoped_reserver_type;
    typedef typename base_type::cookie_type cookie_type;
public:
    typedef typename base_type::hash_table_type hash_table_type;
    typedef typename hash_table_type::key_type key_type;
    typedef typename hash_table_type::node_type node_type;
    typedef typename hash_table_type::allocator_type allocator_type;
    typedef typename hash_table_type::size_type size_type;
    typedef typename hash_table_type::table_type table_type;
    typedef typename hash_table_type::snapshot_type snapshot_type;

public:
    hash_table_base(hash_table_type & hashTable, size_type watermark) :
            base_type(hashTable, watermark)
    {

    }

public:
    void getSnapshot(snapshot_type & snapshot) const
    {
        snapshot_type tmp;
        {
            const table_type* ptr;
            const_guard_type guard(getBase(), ptr);
            tmp.reserve(ptr->m_size.load(barriers::relaxed));
            base_type::m_hashTable.getSnapshot_imp(*ptr, tmp);
        }
        snapshot.swap(tmp);
    }
    bool erase(const key_type & key)
    {
        table_type* ptr;
        mutable_guard_type guard(getBase(), ptr);

        return base_type::m_hashTable.erase_impl(*ptr, key);
    }
    void checkWatermark()
    {
        while (isAboveWatermark())
        {
            cookie_type cookie;
            table_type * ptr;
            bool res = base_type::startRehashing(ptr, cookie);
            if (!res)
            {
                continue;
            }

            size_type next_capacity = ptr->m_capacity * 2;
            table_type* next_ptr;

            try
            {
                next_ptr = base_type::allocateTable(next_capacity);
                base_type::m_hashTable.rehash_impl(*ptr, *next_ptr);
            }
            catch (...)
            {
                base_type::cancelRehashing(ptr);
                throw;
            }
            base_type::finalizeRehashing(next_ptr, cookie);
            base_type::deallocateTable(ptr);
        }
    }
    size_type getCapacity() const
    {
        const table_type* ptr;
        const_guard_type guard(getBase(), ptr);
        return ptr->m_highWatermark;
    }
    size_type size() const
    {
        const table_type* ptr;
        const_guard_type guard(getBase(), ptr);
        return ptr->m_size.load(barriers::relaxed);
    }
    size_type getUsed() const
    {
        const table_type* ptr;
        const_guard_type guard(getBase(), ptr);
        return ptr->m_used;
    }
    size_type getHighWatermark() const
    {
        const table_type* ptr;
        const_guard_type guard(getBase(), ptr);
        return ptr->m_highWatermark;
    }
    bool isAboveWatermark()
    {
        const table_type* ptr;
        const_guard_type guard(getBase(), ptr);
        return (ptr->m_used.load(barriers::relaxed)
                + m_concurrentInsertions.get()) >= ptr->m_highWatermark;
    }
protected:
    base_type& getBase()
    {
        return *static_cast<base_type*>(this);
    }
    const base_type& getBase() const
    {
        return *static_cast<const base_type*>(this);
    }
protected:
    insert_guard_type m_concurrentInsertions;
};

template<typename HashTable, bool greedy>
struct get_hash_table_family_type;

template<typename HashTable>
struct get_hash_table_family_type<HashTable, true>
{
    typedef greedy_hash_table_base<HashTable> type;
};

template<typename HashTable>
struct get_hash_table_family_type<HashTable, false>
{
    typedef wise_hash_table_base<HashTable> type;
};

template<typename HashTable, bool greedy>
struct get_hash_table_base_type
{
    typedef typename get_hash_table_family_type<HashTable, greedy>::type family_type;
    typedef hash_table_base<family_type> type;
};

}

#endif /* INCLUDE_HASH_TABLE_BASE_HPP_ */
