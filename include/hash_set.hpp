/*
 * hash_set.hpp
 *
 *  Created on: Feb 12, 2015
 *      Author: masha
 */

#ifndef INCLUDE_HASH_SET_HPP_
#define INCLUDE_HASH_SET_HPP_

#include "hash_set_table.hpp"
#include "hash_set_table_base.hpp"
#include "hash_set_table_integral_key.hpp"
#include "hash_map.hpp"
#include "xtraits.hpp"
#include "xfunctional.hpp"

namespace lfds
{

namespace
{

// hash_set is an adapter for hash_map
template<class T, class Hash, class Pred, class Allocator, bool = is_integral<T>::value>
struct hash_set_table_selector;

template<class T, class Hash, class Pred, class Allocator>
struct hash_set_table_selector<T, Hash, Pred, Allocator, false>
{
    typedef hash_set_table<T, Hash, Pred, Allocator> type;
};

template<class T, class Hash, class Pred, class Allocator>
struct hash_set_table_selector<T, Hash, Pred, Allocator, true>
{
    typedef hash_set_table_integral_key<T, Hash, Pred, Allocator> type;
};
}

// hash_set is an adapter for hash_map
template<class T, class Hash = typename get_hash<T>::type, class Pred = std::equal_to<T>,
        class Allocator = std::allocator<T> >
class hash_set
{
public:
    typedef hash_set<T, Hash, Pred, Allocator> this_type;
    typedef typename hash_set_table_selector<T, Hash, Pred, Allocator>::type hash_table_type;
    typedef typename hash_table_type::size_type size_type;
    typedef typename hash_table_type::key_type key_type;
    typedef typename hash_table_type::hash_func_type hash_func_type;
    typedef typename hash_table_type::equal_predicate_type equal_predicate_type;
    typedef typename hash_table_type::allocator_type allocator_type;
    typedef typename hash_table_type::snapshot_type snapshot_type;

    static constexpr int INTEGRAL = hash_table_type::INTEGRAL;

    hash_set(size_type initialCapacity = 0) :
        m_hash_table(), m_hash_table_base(m_hash_table, initialCapacity)
    {
    }
    void getSnapshot(snapshot_type & snapshot) const
    {
        m_hash_table_base.getSnapshot(snapshot);
    }
    bool find(const key_type & key) const
    {
        return m_hash_table_base.find(key);
    }
    bool insert(const key_type & key)
    {
        return m_hash_table_base.insert(key);
    }
    bool erase(const key_type & key)
    {
        return m_hash_table_base.erase(key);
    }
    size_type size() const
    {
        return m_hash_table_base.size();
    }
    size_type capacity() const
    {
        return m_hash_table_base.capacity();
    }

private:
    typedef hash_set_table_base<hash_table_type> hash_table_base_type;

    hash_table_type m_hash_table;
    hash_table_base_type m_hash_table_base;
};

}

#endif /* INCLUDE_HASH_SET_HPP_ */
