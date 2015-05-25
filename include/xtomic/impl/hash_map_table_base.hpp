/*
 * hash_map_table_base.hpp
 *
 *  Created on: Mar 4, 2015
 *      Author: masha
 */

#ifndef INCLUDE_HASH_MAP_TABLE_BASE_HPP_
#define INCLUDE_HASH_MAP_TABLE_BASE_HPP_

#include "hash_table_base.hpp"
#include <xtomic/aux/cppbasics.hpp>

#include <vector>
#include <utility>

namespace lfds
{

template<typename HashTableBase>
class hash_map_table_base: public HashTableBase
{
public:

    typedef hash_map_table_base<HashTableBase> this_type;
    typedef HashTableBase base_type;

    typedef typename base_type::hash_table_type hash_table_type;
    typedef typename hash_table_type::key_type key_type;
    typedef typename hash_table_type::mapped_type mapped_type;
    typedef typename hash_table_type::node_type node_type;
    typedef typename hash_table_type::allocator_type allocator_type;
    typedef typename hash_table_type::size_type size_type;
    typedef typename hash_table_type::table_type table_type; // aka raw_hash_table<node_type>

    typedef std::pair<key_type, mapped_type> value_type;
    typedef std::vector<value_type> snapshot_type;

private:
    typedef typename base_type::const_guard_type const_guard_type;
    typedef typename base_type::mutable_guard_type mutable_guard_type;
    typedef typename base_type::insert_guard_type insert_guard_type;
    typedef typename base_type::scoped_reserver_type scoped_reserver_type;

public:
    hash_map_table_base(hash_table_type & hashTable, size_type reserve) :
            base_type(hashTable, reserve)
    {
    }
public:
    bool find(const key_type & key, mapped_type & value) const
    {
        const table_type* ptr;
        const_guard_type guard(base_type::getBase(), ptr);

        return base_type::m_hashTable.find_impl(*ptr, key, value);
    }
#if LFDS_USE_CPP11
    template<typename ... Args>
    bool insert(const key_type & key, const bool updateIfExists, Args&&... val)
#else // LFDS_USE_CPP11
    bool insert(const key_type & key, const bool updateIfExists, const mapped_type& val)
#endif // LFDS_USE_CPP11
    {
        // the lock prevents overwhelming by big number of concurrent insertions
        scoped_reserver_type reserver(base_type::m_concurrentInsertions);

        base_type::checkWatermark();

        table_type* ptr;
        mutable_guard_type guard(base_type::getBase(), ptr);

        return base_type::m_hashTable.insert_impl(*ptr, key, updateIfExists,
                std_forward(Args, val));
    }
};

template<typename HashTable, bool greedy>
struct get_hash_map_table_base_type
{
    typedef typename get_hash_table_base_type<HashTable, greedy>::type base_type;
    typedef hash_map_table_base<base_type> type;
};

}

#endif /* INCLUDE_HASH_MAP_TABLE_BASE_HPP_ */
