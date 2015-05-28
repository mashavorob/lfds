/*
 * hash_set_table_base.hpp
 *
 *  Created on: Mar 4, 2015
 *      Author: masha
 */

#ifndef INCLUDE_HASH_SET_TABLE_BASE_HPP_
#define INCLUDE_HASH_SET_TABLE_BASE_HPP_

#include "hash_table_base.hpp"
#include <xtomic/aux/cppbasics.hpp>

#include <vector>

namespace xtomic
{

template<typename BaseHashTable>
class hash_set_table_base: public BaseHashTable
{
public:

    typedef hash_set_table_base<BaseHashTable> this_type;
    typedef BaseHashTable base_type;

    typedef typename base_type::hash_table_type hash_table_type;
    typedef typename hash_table_type::key_type key_type;
    typedef typename hash_table_type::node_type node_type;
    typedef typename hash_table_type::allocator_type allocator_type;
    typedef typename hash_table_type::size_type size_type;
    typedef typename hash_table_type::table_type table_type; // aka raw_hash_table<node_type>

    typedef std::vector<key_type> snapshot_type;

private:
    typedef typename base_type::const_guard_type const_guard_type;
    typedef typename base_type::mutable_guard_type mutable_guard_type;
    typedef typename base_type::insert_guard_type insert_guard_type;
    typedef typename base_type::scoped_reserver_type scoped_reserver_type;

public:
    hash_set_table_base(hash_table_type & hashTable, size_type reserve) :
            base_type(hashTable, reserve)
    {
    }
public:
    bool find(const key_type & key) const
    {
        const table_type* ptr;
        const_guard_type guard(base_type::getBase(), ptr);

        return base_type::m_hashTable.find_impl(*ptr, key);
    }
    bool insert(const key_type & key)
    {
        // the reserver prevents overwhelming by big number of concurrent insertions
        scoped_reserver_type reserve(base_type::m_concurrentInsertions);

        base_type::checkWatermark();

        table_type* ptr;
        mutable_guard_type guard(base_type::getBase(), ptr);

        return base_type::m_hashTable.insert_impl(*ptr, key);
    }
};

template<typename HashTable, bool greedy>
struct get_hash_set_table_base_type
{
    typedef typename get_hash_table_base_type<HashTable, greedy>::type base_type;
    typedef hash_set_table_base<base_type> type;
};
}

#endif /* INCLUDE_HASH_SET_TABLE_BASE_HPP_ */
