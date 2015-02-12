/*
 * hash_map_data_adapter.hpp
 *
 *  Created on: Feb 10, 2015
 *      Author: masha
 */

#ifndef TESTS_HASH_MAP_DATA_ADAPTER_HPP_
#define TESTS_HASH_MAP_DATA_ADAPTER_HPP_

#include <functional>

namespace lfds
{
namespace testing
{

template<class T>
class adapter
{
public:
    adapter() :
            m_t()
    {

    }
    adapter(T t) :
            m_t(t)
    {

    }
    adapter(const adapter<T> & other) :
            m_t(other.m_t)
    {

    }
    bool operator==(const adapter<T> & other) const
    {
        return m_t == other.m_t;
    }
    std::size_t hash() const
    {
        static const std::hash<T> hasher;
        return hasher(m_t);
    }
public:
    T m_t;
};
}
}

namespace std
{
template<>
struct hash<lfds::testing::adapter<int> > : public unary_function<
        lfds::testing::adapter<int>, std::size_t>
{
    size_t operator()(const lfds::testing::adapter<int>& val) const
    {
        return val.hash();
    }
};
}

template<class T>
struct bad_hash: public std::unary_function<T, std::size_t>
{
    std::size_t operator()(const T&) const
    {
        return 0;
    }
};

template<class T>
inline bool operator==(const lfds::testing::adapter<T> & a, T b)
{
    return a.m_t == b;
}

template<class T>
inline bool operator==(T a, const lfds::testing::adapter<T> & b)
{
    return a == b.m_t;
}

#endif /* TESTS_HASH_MAP_DATA_ADAPTER_HPP_ */
