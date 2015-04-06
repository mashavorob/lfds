/*
 * stdmap.hpp
 *
 *  Created on: Mar 26, 2015
 *      Author: masha
 */

#ifndef PERFTEST_STDMAP_HPP_
#define PERFTEST_STDMAP_HPP_

#include <testsync.hpp>
#include <testallocator.hpp>
#include <xfunctional.hpp>
#include <cppbasics.hpp>

#include <pthread.h>
#include <map>
#if LFDS_USE_CPP11
#include <unordered_map>
#else
#include <tr1/unordered_map>
#endif

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
template<class Key, class Value, class Allocator, bool Unordered>
struct get_map_type;

template<class Key, class Value, class Allocator>
struct get_map_type<Key, Value, Allocator, false>
{
    typedef std::map<Key, Value, std::less<Key>, Allocator> type;
};

template<class Key, class Value, class Allocator>
struct get_map_type<Key, Value, Allocator, true>
{
    typedef typename get_hash<Key>::type hash_type;
#if LFDS_USE_CPP11
    typedef std::unordered_map<Key, Value, hash_type, std::equal_to<Key>, Allocator> type;
#else
    typedef std::tr1::unordered_map<Key, Value, hash_type, std::equal_to<Key>, Allocator> type;
#endif
};

template<bool Unordered>
struct get_reserve_implemented
{
    enum
    {
        value = false
    };
};

#if LFDS_USE_CPP11
template<>
struct get_reserve_implemented<true>
{
    enum
    {
        value = true
    };
};
#endif

template<class Map, bool Unordered, bool = get_reserve_implemented<Unordered>::value >
struct reserver
{
    typedef Map map_type;
    typedef typename map_type::size_type size_type;
    static void reserve(map_type&, const size_type)
    {

    }
};

template<class Map, bool Unordered>
struct reserver<Map, Unordered, true>
{
    typedef Map map_type;
    typedef typename map_type::size_type size_type;
    static void reserve(map_type& map, const size_type size)
    {
        map.reserve(size);
    }
};

}

template<class Key, class Value, bool Unordered, class Allocator = std::allocator<std::pair<const Key, Value> > >
class stdmap
{
public:
    typedef typename get_map_type<Key, Value, Allocator, Unordered>::type collection_type;
    typedef typename collection_type::key_type key_type;
    typedef typename collection_type::mapped_type mapped_type;
    typedef typename collection_type::size_type size_type;

    static constexpr bool RESERVE_IMPLEMENTED = Unordered;
    static constexpr bool ALLOCATOR_IMPLEMENTED = get_reserve_implemented<Unordered>::value;

    typedef Allocator allocator_type;
    typedef counted_allocator<allocator_type> counted_allocator_type;
    typedef stdmap<Key, Value, Unordered, counted_allocator_type> counted_map_type;

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
