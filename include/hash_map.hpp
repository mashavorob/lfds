/*
 * hash_map
 *
 *  Created on: Jan 28, 2015
 *      Author: masha
 */

#ifndef INCLUDE_HASH_MAP_HPP_
#define INCLUDE_HASH_MAP_HPP_

#include "memory_model.hpp"
#include "hash_map_table.hpp"
#include "hash_map_table_integral_pair.hpp"
#include "hash_map_table_integral_value.hpp"
#include "hash_map_table_integral_key.hpp"
#include "hash_map_table_simplified.hpp"
#include "hash_map_table_base.hpp"
#include "xtraits.hpp"
#include "xfunctional.hpp"
#include "cppbasics.hpp"

#include <functional>

namespace lfds
{

//
// some meta classes to specify hash table implementation from Key and Value types
//
namespace
{

template<typename Key, typename Value, bool = (sizeof(Key) < sizeof(Value))>
struct dummy_hash_tuple
{
    Key m_key;
    Value m_value;
    int8_t m_dummy;
};

template<typename Key, typename Value>
struct dummy_hash_tuple<Key, Value, true>
{
    Key m_key;
    int8_t m_dummy;
    Value m_value;
};

template<typename Key, typename Value>
struct is_integral_pair
{
    enum
    {
        value = is_integral<Key>::value && is_integral<Value>::value
                && (sizeof(dummy_hash_tuple<Key, Value> ) <= 16)
    };
};

template<typename Key, typename Value,
        typename Hash = typename make_hash<Key>::type,
        typename Pred = std::equal_to<Key>, typename Allocator = std::allocator<
                Value>, bool IntegralKey = is_integral<Key>::value,
        bool IntegralValue = is_integral<Value>::value, bool IntegralKeyValue =
                is_integral_pair<Key, Value>::value>
struct hash_table_traits
{
    typedef lfds::hash_map_table<Key, Value, Hash, Pred, Allocator> type;
};

template<typename Key, typename Value, typename Hash, typename Pred,
        typename Allocator, bool IntegralKey, bool IntegralValue>
struct hash_table_traits<Key, Value, Hash, Pred, Allocator, IntegralKey,
        IntegralValue, true>
{
    typedef lfds::hash_map_table_integral_pair<Key, Value, Hash, Pred, Allocator> type;
};

template<typename Key, typename Value, typename Hash, typename Pred,
        typename Allocator, bool IntegralKey>
struct hash_table_traits<Key, Value, Hash, Pred, Allocator, IntegralKey, true,
        false>
{
    typedef lfds::hash_map_table_integral_value<Key, Value, Hash, Pred,
            Allocator> type;
};

template<typename Key, typename Value, typename Hash, typename Pred,
        typename Allocator>
struct hash_table_traits<Key, Value, Hash, Pred, Allocator, true, false, false>
{
    typedef lfds::hash_map_table_integral_key<Key, Value, Hash, Pred, Allocator> type;
};

template<typename Key, typename Value,
        typename Hash = typename make_hash<Key>::type,
        typename Pred = std::equal_to<Key>, typename Allocator = std::allocator<
                Value>, bool IntegralKey = is_integral<Key>::value,
        bool IntegralValue = is_integral<Value>::value,
        bool TooBig = is_greater<sizeof(std::pair<Key, Value>),
                2 * sizeof(void*)>::value>
struct simple_hash_table_traits;

template<typename Key, typename Value, typename Hash, typename Pred,
        typename Allocator>
struct simple_hash_table_traits<Key, Value, Hash, Pred, Allocator, true, true,
        false>
{
    typedef hash_map_table_simplified<Key, Value, Hash, Pred, Allocator> type;
};

}

//
// Hash map
//
template<typename Key, typename Value,
        typename Hash = typename make_hash<Key>::type,
        typename Pred = std::equal_to<Key>, typename Allocator = std::allocator<
                Value>, memory_model::type MemModel =
                default_memory_model::value>
class hash_map
{
public:

    typedef hash_map<Key, Value, Hash, Pred, Allocator, MemModel> this_type;
    typedef hash_table_traits<Key, Value, Hash, Pred, Allocator> hash_table_traits_type;

    typedef typename hash_table_traits_type::type hash_table_type;
    typedef typename hash_table_type::key_type key_type;
    typedef typename hash_table_type::mapped_type mapped_type;
    typedef typename hash_table_type::hash_func_type hash_func_type;
    typedef typename hash_table_type::equal_predicate_type equal_predicate_type;
    typedef typename hash_table_type::allocator_type allocator_type;
    typedef typename hash_table_type::size_type size_type;
    typedef typename hash_table_type::value_type value_type;
    typedef typename hash_table_type::snapshot_type snapshot_type;

    static constexpr bool INTEGRAL_KEY = hash_table_type::INTEGRAL_KEY;
    static constexpr bool INTEGRAL_VALUE = hash_table_type::INTEGRAL_VALUE;
    static constexpr bool INTEGRAL_KEYVALUE = hash_table_type::INTEGRAL_KEYVALUE;
    static constexpr memory_model::type MEMORY_MODEL = MemModel;
private:
    hash_map(const this_type&); // = delete;
    this_type& operator=(const this_type&); // = delete;

public:
    hash_map(const size_type initialCapacity = 0) :
            m_hash_table(),
            m_hash_table_base(m_hash_table, initialCapacity)
    {

    }
    void getSnapshot(snapshot_type & snapshot) const
    {
        m_hash_table_base.getSnapshot(snapshot);
    }
    bool find(const key_type & key, mapped_type & value) const
    {
        return m_hash_table_base.find(key, value);
    }
    const mapped_type find(const key_type & key) const
    {
        mapped_type value = mapped_type();
        m_hash_table_base.find(key, value);
        return value;
    }
#if LFDS_USE_CPP11
    template<typename ... Args>
    bool insert(const key_type & key, Args&&... val)
#else
    bool insert(const key_type & key, const mapped_type& val)
#endif
    {
        return m_hash_table_base.insert(key, false, std_forward(Args, val));
    }
#if LFDS_USE_CPP11
    template<typename ... Args>
    bool insertOrUpdate(const key_type & key, Args&&... val)
#else
    bool insertOrUpdate(const key_type & key, const mapped_type& val)
#endif
    {
        return m_hash_table_base.insert(key, true, std_forward(Args, val));
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
    //typedef hash_map_table_base<hash_table_type> hash_table_base_type;
    typedef hash_map_table_base_traits<hash_table_type, MemModel> hash_map_table_base_traits_type;
    typedef typename hash_map_table_base_traits_type::type hash_table_base_type;

    hash_table_type m_hash_table;
    hash_table_base_type m_hash_table_base;
};

template<typename Key, typename Value, typename Hash, typename Pred,
        typename Allocator>
class hash_map<Key, Value, Hash, Pred, Allocator, memory_model::simplified>
{
public:

    typedef hash_map<Key, Value, Hash, Pred, Allocator, memory_model::simplified> this_type;
    typedef simple_hash_table_traits<Key, Value, Hash, Pred, Allocator> hash_table_traits_type;

    typedef typename hash_table_traits_type::type hash_table_type;
    typedef typename hash_table_type::key_type key_type;
    typedef typename hash_table_type::mapped_type mapped_type;
    typedef typename hash_table_type::hash_func_type hash_func_type;
    typedef typename hash_table_type::equal_predicate_type equal_predicate_type;
    typedef typename hash_table_type::allocator_type allocator_type;
    typedef typename hash_table_type::size_type size_type;
    typedef typename hash_table_type::value_type value_type;
    typedef typename hash_table_type::snapshot_type snapshot_type;

    static constexpr bool INTEGRAL_KEY = hash_table_type::INTEGRAL_KEY;
    static constexpr bool INTEGRAL_VALUE = hash_table_type::INTEGRAL_VALUE;
    static constexpr bool INTEGRAL_KEYVALUE = hash_table_type::INTEGRAL_KEYVALUE;
    static constexpr memory_model::type MEMORY_MODEL = memory_model::simplified;
private:
    hash_map(const this_type&); // = delete;
    this_type& operator=(const this_type&); // = delete;

public:
    hash_map(const size_type initialCapacity = 0) :
            m_hash_table(),
            m_hash_table_base(m_hash_table, initialCapacity)
    {

    }
    void getSnapshot(snapshot_type & snapshot) const
    {
        m_hash_table_base.getSnapshot(snapshot);
    }
    bool find(const key_type & key, mapped_type & value) const
    {
        return m_hash_table_base.find(key, value);
    }
    const mapped_type find(const key_type & key) const
    {
        mapped_type value = mapped_type();
        m_hash_table_base.find(key, value);
        return value;
    }
#if LFDS_USE_CPP11
    template<typename ... Args>
    bool insert(const key_type & key, Args&&... val)
#else
    bool insert(const key_type & key, const mapped_type& val)
#endif
    {
        return m_hash_table_base.insert(key, false, std_forward(Args, val));
    }
#if LFDS_USE_CPP11
    template<typename ... Args>
    bool insertOrUpdate(const key_type & key, Args&&... val)
#else
    bool insertOrUpdate(const key_type & key, const mapped_type& val)
#endif
    {
        return m_hash_table_base.insert(key, true, std_forward(Args, val));
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
    //typedef hash_map_table_base<hash_table_type> hash_table_base_type;
    typedef hash_map_table_base_traits<hash_table_type, memory_model::greedy> hash_map_table_base_traits_type;
    typedef typename hash_map_table_base_traits_type::type hash_table_base_type;

    hash_table_type m_hash_table;
    hash_table_base_type m_hash_table_base;
};

template<typename Key, typename Value,
        typename Hash = typename make_hash<Key>::type,
        typename Pred = std::equal_to<Key>, typename Allocator = std::allocator<
                Value> >
struct make_wise_hash_map
{
    typedef hash_map<Key, Value, Hash, Pred, Allocator, memory_model::wise> type;
};

template<typename Key, typename Value,
        typename Hash = typename make_hash<Key>::type,
        typename Pred = std::equal_to<Key>, typename Allocator = std::allocator<
                Value> >
struct make_greedy_hash_map
{
    typedef hash_map<Key, Value, Hash, Pred, Allocator, memory_model::greedy> type;
};

template<typename Key, typename Value,
        typename Hash = typename make_hash<Key>::type,
        typename Pred = std::equal_to<Key>, typename Allocator = std::allocator<
                Value> >
struct make_simplified_hash_map
{
    typedef hash_map<Key, Value, Hash, Pred, Allocator, memory_model::simplified> type;
};

}

#endif /* INCLUDE_HASH_MAP_HPP_ */
