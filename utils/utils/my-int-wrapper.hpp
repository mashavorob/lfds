/*
 * my-int-wrapper.hpp
 *
 *  Created on: Apr 16, 2015
 *      Author: masha
 */

#ifndef UTILS_UTILS_MY_INT_WRAPPER_HPP_
#define UTILS_UTILS_MY_INT_WRAPPER_HPP_

#include <xtomic/aux/xfunctional.hpp>
#include <xtomic/aux/cppbasics.hpp>

namespace lfds
{
namespace my
{

template<typename T>
class int_wrapper
{
public:
    typedef T type;
    typedef int_wrapper<T> this_type;
    int_wrapper()
    {

    }
    int_wrapper(T t) :
            m_t(t)
    {

    }

    operator T() const
    {
        return m_t;
    }
    bool operator<(const this_type & other) const
    {
        return m_t < other.m_t;
    }
    bool operator<=(const this_type & other) const
    {
        return m_t <= other.m_t;
    }
    bool operator==(const this_type & other) const
    {
        return m_t == other.m_t;
    }
    bool operator>=(const this_type & other) const
    {
        return m_t >= other.m_t;
    }
    bool operator>(const this_type & other) const
    {
        return m_t > other.m_t;
    }

    this_type operator+(const this_type & other) const
    {
        return m_t + other.m_t;
    }
    this_type operator-(const this_type & other) const
    {
        return m_t - other.m_t;
    }
    this_type operator*(const this_type & other) const
    {
        return m_t * other.m_t;
    }
    this_type operator/(const this_type & other) const
    {
        return m_t / other.m_t;
    }
    this_type& operator+=(const this_type & other)
    {
        m_t += other.m_t;
        return *this;
    }
    this_type& operator-=(const this_type & other)
    {
        m_t += other.m_t;
        return *this;
    }
    this_type& operator*=(const this_type & other)
    {
        m_t += other.m_t;
        return *this;
    }
    this_type& operator++()
    {
        --m_t;
        return *this;
    }
    this_type& operator--()
    {
        --m_t;
        return *this;
    }
    this_type operator++(int)
    {
        return m_t++;
    }
    this_type operator--(int)
    {
        return m_t--;
    }

private:
    T m_t;
};

template<typename T>
struct int_wrapper_hash
{
    size_t operator()(const int_wrapper<T> & val) const
    {
        return m_hasher(val);
    }
private:
    typename lfds::make_hash<T>::type m_hasher;
};

template<typename T>
struct make_hash
{
    typedef typename lfds::make_hash<T>::type type;
};

template<typename T>
struct make_hash< int_wrapper<T> >
{
    typedef int_wrapper_hash<T> type;
};


template<typename T>
struct remove_wrapper
{
    typedef T type;
};

template<typename T>
struct remove_wrapper<lfds::my::int_wrapper<T> >
{
    typedef T type;
};

template<typename T, typename Int>
struct make_value_type
{
    typedef Int type;
};

template<typename T, typename Int>
struct make_value_type<lfds::my::int_wrapper<T>, Int>
{
    typedef lfds::my::int_wrapper<Int> type;
};

}
}
#endif /* UTILS_UTILS_MY_INT_WRAPPER_HPP_ */
