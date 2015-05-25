/*
 * my-rand.hpp
 *
 *  Created on: Apr 14, 2015
 *      Author: masha
 */

#ifndef UTILS_UTILS_MY_RAND_HPP_
#define UTILS_UTILS_MY_RAND_HPP_

#include <xtomic/impl/xtraits.hpp>
#include <xtomic/aux/cppbasics.hpp>

#include <set>
#include <vector>
#include <cstdlib>
#include <ctime>

namespace lfds
{
namespace my
{

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

template<typename T>
class random_generator_4 : private rand_base
{
public:
    T operator()() const
    {
        return static_cast<T>(rand());
    }
};

template<typename T>
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

template<typename T, int = sizeof(T)>
struct get_integral_random_generator_type
{
    typedef random_generator_4<T> type;
};

template<typename T>
struct get_integral_random_generator_type<T, 8>
{
    typedef random_generator_8<T> type;
};

template<typename T>
class random_generator_complex
{
private:
    typedef typename T::type inner_type;
    typedef typename get_integral_random_generator_type<inner_type>::type generator_type;
public:
    T operator()() const
    {
        return static_cast<T>(m_gen());
    }
private:
    generator_type m_gen;

};

template<typename T, bool Integer = is_integer<T>::value >
struct select_random_generator_type;

template<typename T>
struct select_random_generator_type<T, true>
{
    typedef typename get_integral_random_generator_type<T>::type type;
};

template<typename T>
struct select_random_generator_type<T, false>
{
    typedef random_generator_complex<T> type;
};


template<typename T>
struct get_random_generator_type
{
    typedef typename select_random_generator_type<T>::type type;
};

}

template<typename T>
class random_generator
{
private:
    typedef typename get_random_generator_type<T>::type type;
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
#endif /* UTILS_UTILS_MY_RAND_HPP_ */
