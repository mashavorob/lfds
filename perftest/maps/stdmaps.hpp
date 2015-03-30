/*
 * stdmap.hpp
 *
 *  Created on: Mar 26, 2015
 *      Author: masha
 */

#ifndef PERFTEST_STDMAP_HPP_
#define PERFTEST_STDMAP_HPP_

#include <testsync.hpp>

#include <map>
#include <unordered_map>
#include <pthread.h>

namespace lfds
{
namespace perftest
{
namespace maps
{

namespace adapter
{

typedef lfds::perftest::sync::mutex mutex_type;
typedef lfds::perftest::sync::guard guard_type;

namespace
{
template<class Key, class Value, bool Unordered>
struct get_map_type;

template<class Key, class Value>
struct get_map_type<Key, Value, false>
{
    typedef std::map<Key, Value> type;
};

template<class Key, class Value>
struct get_map_type<Key, Value, true>
{
    typedef std::unordered_map<Key, Value> type;
};

template<class Map, bool Unordered>
struct reserver;


template<class Map>
struct reserver<Map, false>
{
    typedef Map map_type;
    typedef typename map_type::size_type size_type;
    static void reserve(map_type&, const size_type)
    {

    }
};

template<class Map>
struct reserver<Map, true>
{
    typedef Map map_type;
    typedef typename map_type::size_type size_type;
    static void reserve(map_type & map, const size_type size)
    {
        map.reserve(size);
    }
};

}

template<class Key, class Value, bool Unordered>
class stdmap
{
public:
    typedef typename get_map_type<Key, Value, Unordered>::type collection_type;
    typedef typename collection_type::key_type key_type;
    typedef typename collection_type::mapped_type mapped_type;
    typedef typename collection_type::size_type size_type;

    static constexpr bool RESERVE_IMPLEMENTED = Unordered;

public:

    stdmap(size_type reserve = 0)
    {
        typedef reserver<collection_type, Unordered> reserver_type;
        reserver_type::reserve(m_coll, reserve);
    }

    bool insert(const key_type & key, const mapped_type & val)
    {
        guard_type guard(m_mutex);
        return m_coll.insert(std::make_pair(key, val)).second;
    }

    bool find(const key_type & key, mapped_type & val) const
    {
        guard_type guard(m_mutex);
        typename collection_type::const_iterator pos = m_coll.find(key);
        if (pos == m_coll.end())
        {
            return false;
        }
        val = pos->second;
        return true;
    }
    bool erase(const key_type & key)
    {
        guard_type guard(m_mutex);
        typename collection_type::iterator pos = m_coll.find(key);
        if (pos == m_coll.end())
        {
            return false;
        }
        m_coll.erase(pos);
        return true;
    }

private:
    mutex_type m_mutex;
    collection_type m_coll;
};

}
}
}
}

#endif /* PERFTEST_STDMAP_HPP_ */
