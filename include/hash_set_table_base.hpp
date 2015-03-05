/*
 * hash_set_table_base.hpp
 *
 *  Created on: Mar 4, 2015
 *      Author: masha
 */

#ifndef INCLUDE_HASH_SET_TABLE_BASE_HPP_
#define INCLUDE_HASH_SET_TABLE_BASE_HPP_

#include "hash_table_base.hpp"

#include <vector>

namespace lfds
{
template<class HashTable>
class hash_set_table_base: public hash_table_base<HashTable>
{
public:

    typedef hash_set_table_base<HashTable> this_type;
    typedef hash_table_base<HashTable> base_type;

    typedef typename base_type::hash_table_type hash_table_type;
    typedef typename base_type::scoped_lock_type scoped_lock_type;
    typedef typename base_type::scoped_reserver_type scoped_reserver_type;
    typedef typename hash_table_type::key_type key_type;
    typedef typename hash_table_type::node_type node_type;
    typedef typename hash_table_type::allocator_type allocator_type;
    typedef typename hash_table_type::size_type size_type;
    typedef typename hash_table_type::table_type table_type; // aka raw_hash_table<node_type>

    typedef std::vector<key_type> snapshot_type;


public:
    hash_set_table_base(hash_table_type & hashTable, size_type reserve) :
            base_type(hashTable, reserve)
    {
    }
public:
    bool find(const key_type & key) const
    {
        // attempt to make a wait free find
        scoped_lock_type guard(base_type::m_constTable);
        const table_type* ptr = base_type::m_constTable.m_ptr.load(
                std::memory_order_relaxed);

        return base_type::m_hashTable.find_impl(*ptr, key);
    }
    template<class ... Args>
    bool insert(const key_type & key)
    {
        // the reserver prevents overwhelming by big number of concurrent insertions
        scoped_reserver_type reserve(base_type::m_concurrentInsertions);

        base_type::check_watermark();

        table_type* ptr = base_type::acquire_table(base_type::m_mutableTable);
        scoped_lock_type guard(base_type::m_mutableTable, false);

        return base_type::m_hashTable.insert_impl(*ptr, key);
    }
};
}

#endif /* INCLUDE_HASH_SET_TABLE_BASE_HPP_ */
