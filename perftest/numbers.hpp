/*
 * numbers.hpp
 *
 *  Created on: Mar 26, 2015
 *      Author: masha
 */

#ifndef PERFTEST_NUMBERS_HPP_
#define PERFTEST_NUMBERS_HPP_

#include <inttypes.hpp>
#include <xfunctional.hpp>
#include <cppbasics.hpp>

#include <cstdlib>
#include <ctime>
#include <set>
#include <vector>

namespace lfds
{
namespace perftest
{

template<class T>
class dummy_wrapper
{
public:
    typedef T type;
    typedef dummy_wrapper<T> this_type;
    dummy_wrapper()
    {

    }
    dummy_wrapper(T t) :
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
struct dummy_hash
{
    size_t operator()(const dummy_wrapper<T> & val) const
    {
        return m_hasher(val);
    }
private:
    typename getHash<T>::type m_hasher;
};

namespace {

class rand_base
{
public:
    rand_base()
    {
        init();
    }
private:
    class SRand
    {
    public:
        SRand()
        {
            srand(time(nullptr));
        }
    };
private:
    static void init()
    {
        static SRand sr;
    }
};

template<class T>
class random_generator_4 : private rand_base
{
public:
    T operator()() const
    {
        return static_cast<T>(rand());
    }
};

template<class T>
class random_generator_8 : private rand_base
{
private:
    union DUMMY
    {
        struct {
            int a,b;
        } s;
        T t;
    };
public:
    T operator()() const
    {
        DUMMY d;
        d.s.a = rand();
        d.s.b = rand();
        return d.t;
    }
};

template<class T, int = sizeof(T)>
struct get_integral_random_generator
{
    typedef random_generator_4<T> type;
};

template<class T>
struct get_integral_random_generator<T, 8>
{
    typedef random_generator_8<T> type;
};


template<class T>
class random_generator_complex
{
private:
    typedef typename T::type inner_type;
    typedef typename get_integral_random_generator<inner_type>::type generator_type;
public:
    T operator()() const
    {
        return static_cast<T>(m_gen());
    }
private:
    generator_type m_gen;

};

template<class T>
struct get_random_generator
{
    typedef random_generator_complex<T> type;
};

template<>
struct get_random_generator<int8_t>
{
    typedef get_integral_random_generator<int8_t>::type type;
};
template<>
struct get_random_generator<uint8_t>
{
    typedef get_integral_random_generator<uint8_t>::type type;
};

template<>
struct get_random_generator<int16_t>
{
    typedef get_integral_random_generator<int16_t>::type type;
};

template<>
struct get_random_generator<uint16_t>
{
    typedef get_integral_random_generator<uint16_t>::type type;
};

template<>
struct get_random_generator<int32_t>
{
    typedef get_integral_random_generator<int32_t>::type type;
};

template<>
struct get_random_generator<uint32_t>
{
    typedef get_integral_random_generator<uint32_t>::type type;
};
template<>
struct get_random_generator<int64_t>
{
    typedef get_integral_random_generator<int64_t>::type type;
};

template<>
struct get_random_generator<uint64_t>
{
    typedef get_integral_random_generator<uint64_t>::type type;
};

}

template<class T>
class random_generator
{
private:
    typedef typename get_random_generator<T>::type type;
public:
    T operator()() const
    {
        return m_gen();
    }

    void operator()(const std::size_t count, std::vector<T> & res) const
    {
        std::set<T> uniques;
        while ( uniques.size() < count )
        {
            uniques.insert(m_gen());
        }
        res.insert(res.begin(), uniques.begin(), uniques.end());
    }
private:
    type m_gen;
};
}
}

#endif /* PERFTEST_NUMBERS_HPP_ */
