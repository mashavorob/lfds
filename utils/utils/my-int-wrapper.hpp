/*
 * my-int-wrapper.hpp
 *
 *  Created on: Apr 16, 2015
 *      Author: masha
 */

#ifndef UTILS_UTILS_MY_INT_WRAPPER_HPP_
#define UTILS_UTILS_MY_INT_WRAPPER_HPP_

#include <xfunctional.hpp>
#include <cppbasics.hpp>

namespace lfds
{
namespace my
{

template<class T>
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

template<class T>
struct int_wrapper_hash
{
    size_t operator()(const int_wrapper<T> & val) const
    {
        return m_hasher(val);
    }
private:
    typename getHash<T>::type m_hasher;
};

}
}
#endif /* UTILS_UTILS_MY_INT_WRAPPER_HPP_ */
