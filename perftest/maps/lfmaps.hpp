/*
 * lfmaps.hpp
 *
 *  Created on: Mar 30, 2015
 *      Author: masha
 */

#ifndef PERFTEST_MAPS_LFMAPS_HPP_
#define PERFTEST_MAPS_LFMAPS_HPP_

#include <hash_map.hpp>
#include <hash_trie.hpp>
#include <xfunctional.hpp>

namespace lfds
{
namespace perftest
{
namespace maps
{
namespace adapter
{

template<typename Key, typename Value,
        typename Hash = typename make_hash<Key>::type,
        typename Allocator = std::allocator<Value>,
        lfds::memory_model::type MemModel = lfds::default_memory_model::value
        >
class hash_map
{
public:
    typedef std::equal_to<Key> equal_to_type;
    typedef lfds::hash_map<Key, Value, Hash, equal_to_type, Allocator, MemModel> collection_type;
    typedef typename collection_type::key_type key_type;
    typedef typename collection_type::mapped_type mapped_type;
    typedef typename collection_type::size_type size_type;

    static constexpr bool RESERVE_IMPLEMENTED = true;
    static constexpr bool ALLOCATOR_IMPLEMENTED = true;

    typedef Allocator allocator_type;
    typedef counted_allocator<allocator_type> counted_allocator_type;
    typedef hash_map<Key, Value, Hash, counted_allocator_type, MemModel> counted_map_type;

public:
    hash_map(size_type reserve = 0) :
            m_coll(reserve)
    {

    }

    bool insert(const key_type & key, const mapped_type & val)
    {
        return m_coll.insert(key, val);
    }

    bool find(const key_type & key, mapped_type & val) const
    {
        return m_coll.find(key, val);
    }
    bool erase(const key_type & key)
    {
        return m_coll.erase(key);
    }

private:
    collection_type m_coll;

};

template<typename Key, typename Value,
        typename Hash = typename make_hash<Key>::type,
        typename Allocator = std::allocator<Value>
        >
struct make_wise_hash_map
{
    typedef hash_map<Key, Value, Hash, Allocator, lfds::memory_model::wise> type;
};

template<typename Key, typename Value,
        typename Hash = typename make_hash<Key>::type,
        typename Allocator = std::allocator<Value>
        >
struct make_greedy_hash_map
{
    typedef hash_map<Key, Value, Hash, Allocator, lfds::memory_model::greedy> type;
};

template<typename Key, typename Value, int BFactor,
        typename Allocator = std::allocator<Value> >
class hash_trie
{
public:
    typedef lfds::hash_trie<Key, Value, BFactor, typename make_hash<Key>::type,
            std::equal_to<Key>, Allocator> collection_type;
    typedef typename collection_type::key_type key_type;
    typedef typename collection_type::mapped_type mapped_type;
    typedef typename collection_type::size_type size_type;

    static constexpr bool RESERVE_IMPLEMENTED = false;
    static constexpr bool ALLOCATOR_IMPLEMENTED = true;

    typedef Allocator allocator_type;
    typedef counted_allocator<allocator_type> counted_allocator_type;
    typedef hash_trie<Key, Value, BFactor, counted_allocator_type> counted_map_type;

public:
    hash_trie(size_type reserve = 0) :
            m_coll()
    {
        (void) reserve;
    }

    bool insert(const key_type & key, const mapped_type & val)
    {
        return m_coll.insert(key, val);
    }

    bool find(const key_type & key, mapped_type & val) const
    {
        return m_coll.find(key, val);
    }
    bool erase(const key_type & key)
    {
        return m_coll.erase(key);
    }

private:
    collection_type m_coll;

};

}
}
}
}

#endif /* PERFTEST_MAPS_LFMAPS_HPP_ */
