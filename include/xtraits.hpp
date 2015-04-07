/*
 * xtraits.hpp
 *
 *  Created on: Apr 1, 2015
 *      Author: masha
 */

#ifndef INCLUDE_XTRAITS_HPP_
#define INCLUDE_XTRAITS_HPP_

#include "config.hpp"

#include "inttypes.h"

namespace lfds
{

template<typename T, T v>
struct integral_const
{
    typedef T value_type;
    typedef integral_const<T, v> type;
    static const value_type value = v;
};

template<typename T>
struct remove_const
{
    typedef T type;
};

template<typename T>
struct remove_const<T const>
{
    typedef T type;
};

template<typename T>
struct remove_volatile
{
    typedef T type;
};

template<typename T>
struct remove_volatile<T volatile>
{
    typedef T type;
};

template<typename T>
struct remove_unsigned
{
    typedef T type;
};

template<typename T>
struct remove_cv
{
    typedef typename remove_const<typename remove_volatile<T>::type>::type type;
};

template<typename T>
struct is_unqualified_pointer : public integral_const<bool, false>
{
};

template<typename T>
struct is_unqualified_pointer<T*> : public integral_const<bool, true>
{
};

template<typename T>
struct is_pointer : public integral_const<bool, is_unqualified_pointer<typename remove_cv<T>::type>::value>
{
};

template<typename T>
struct is_unqualified_integer : public integral_const<bool, false>
{
};

#define unquallified_int(t) \
        template<>\
        struct is_unqualified_integer<t> : public integral_const<bool, true> {};\
        template<>\
        struct is_unqualified_integer<unsigned t> : public integral_const<bool, true> {}


unquallified_int(char);
unquallified_int(short);
unquallified_int(int);
unquallified_int(long);
unquallified_int(long long);

#undef unquallified_int

template<>
struct is_unqualified_integer<signed char> : public integral_const<bool, true> {};

template<typename T>
struct is_integer : public integral_const<bool, is_unqualified_integer<typename remove_cv<T>::type>::value>
{
};

template<typename T>
struct is_unqualified_bool : public integral_const<bool, false>
{
};

template<>
struct is_unqualified_bool<bool> : public integral_const<bool, true>
{
};

template<typename T>
struct is_bool : public integral_const<bool, is_unqualified_bool<typename remove_cv<T>::type>::value>
{
};


template<typename T>
struct is_integral : public integral_const<bool,
    is_integer<T>::value || is_pointer<T>::value || is_bool<T>::value
    >
{
};

}

#endif /* INCLUDE_XTRAITS_HPP_ */
