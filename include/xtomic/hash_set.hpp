/*
 * hash_set.hpp
 *
 *  Created on: Feb 12, 2015
 *      Author: masha
 */

#ifndef INCLUDE_HASH_SET_HPP_
#define INCLUDE_HASH_SET_HPP_

#include "hash_map.hpp"
#include "impl/memory_model.hpp"
#include "impl/hash_set_table.hpp"
#include "impl/hash_set_table_base.hpp"
#include "impl/hash_set_table_integral_key.hpp"
#include "impl/xtraits.hpp"
#include "aux/xfunctional.hpp"

namespace lfds
{

namespace
{

// hash_set is an adapter for hash_map
template<typename T, typename Hash, typename Pred, typename Allocator, bool =
        is_integral<T>::value>
struct hash_set_table_selector;

template<typename T, typename Hash, typename Pred, typename Allocator>
struct hash_set_table_selector<T, Hash, Pred, Allocator, false>
{
    typedef hash_set_table<T, Hash, Pred, Allocator> type;
};

template<typename T, typename Hash, typename Pred, typename Allocator>
struct hash_set_table_selector<T, Hash, Pred, Allocator, true>
{
    typedef hash_set_table_integral_key<T, Hash, Pred, Allocator> type;
};
}

// hash_set is an adapter for hash_map
template<typename T, typename Hash = typename make_hash<T>::type,
        typename Pred = std::equal_to<T>,
        typename Allocator = std::allocator<T>, memory_model::type MemModel =
                default_memory_model::value>
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
    static constexpr memory_model::type MEMORY_MODEL = MemModel;

    hash_set(size_type initialCapacity = 0) :
            m_hash_table(),
            m_hash_table_base(m_hash_table, initialCapacity)
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
    size_type getCapacity() const
    {
        return m_hash_table_base.getCapacity();
    }

private:
    //typedef hash_set_table_base<hash_table_type> hash_table_base_type;
    typedef hash_set_table_base_traits<hash_table_type, MemModel> hash_set_table_base_traits_type;
    typedef typename hash_set_table_base_traits_type::type hash_table_base_type;

    hash_table_type m_hash_table;
    hash_table_base_type m_hash_table_base;
};

template<typename T, typename Hash = typename make_hash<T>::type,
        typename Pred = std::equal_to<T>, typename Allocator = std::allocator<T> >
struct make_wise_hash_set
{
    typedef hash_set<T, Hash, Pred, Allocator, memory_model::wise> type;
};

template<typename T, typename Hash = typename make_hash<T>::type,
        typename Pred = std::equal_to<T>, typename Allocator = std::allocator<T> >
struct make_greedy_hash_set
{
    typedef hash_set<T, Hash, Pred, Allocator, memory_model::greedy> type;
};

}

#endif /* INCLUDE_HASH_SET_HPP_ */
