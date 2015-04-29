/*
 * hash_map_data_adapter.hpp
 *
 *  Created on: Feb 10, 2015
 *      Author: masha
 */

#ifndef TESTS_HASH_MAP_DATA_ADAPTER_HPP_
#define TESTS_HASH_MAP_DATA_ADAPTER_HPP_

#include <cppbasics.hpp>
#include <xfunctional.hpp>

namespace lfds
{
namespace testing
{

template<typename T>
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
        typename make_hash<T>::type hasher;
        return hasher(m_t);
    }
public:
    T m_t;
};
}
template<typename T>
struct adapted_hash
{
public:
    typedef typename make_hash<T>::type hash_type;

    size_t operator()(const lfds::testing::adapter<T>& val) const
    {
        return m_hasher(val.m_t);
    }

private:
    hash_type m_hasher;
};

template<typename T>
struct make_hash<lfds::testing::adapter<T> >
{
    typedef adapted_hash<T> type;
};

}

template<typename T>
struct bad_hash: public std::unary_function<T, std::size_t>
{
    std::size_t operator()(const T&) const
    {
        return 0;
    }
};

template<typename T>
inline bool operator==(const lfds::testing::adapter<T> & a, T b)
{
    return a.m_t == b;
}

template<typename T>
inline bool operator==(T a, const lfds::testing::adapter<T> & b)
{
    return a == b.m_t;
}

#endif /* TESTS_HASH_MAP_DATA_ADAPTER_HPP_ */
