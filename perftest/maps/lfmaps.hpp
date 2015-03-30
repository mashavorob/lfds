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

namespace lfds
{
namespace perftest
{
namespace maps
{
namespace adapter
{

template<class Key, class Value, class Hash = std::hash<Key>>
class hash_map
{
public:
    typedef lfds::hash_map<Key, Value, Hash> collection_type;
    typedef typename collection_type::key_type key_type;
    typedef typename collection_type::mapped_type mapped_type;
    typedef typename collection_type::size_type size_type;

    static constexpr bool RESERVE_IMPLEMENTED = true;

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


template<class Key, class Value, int BFactor>
class hash_trie
{
public:
    typedef lfds::hash_trie<Key, Value, BFactor> collection_type;
    typedef typename collection_type::key_type key_type;
    typedef typename collection_type::mapped_type mapped_type;
    typedef typename collection_type::size_type size_type;

    static constexpr bool RESERVE_IMPLEMENTED = false;

public:
    hash_trie(size_type reserve = 0) :
            m_coll()
    {
        (void)reserve;
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
