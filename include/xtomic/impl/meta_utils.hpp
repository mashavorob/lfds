/*
 * meta_utils.hpp
 *
 *  Created on: Feb 11, 2015
 *      Author: masha
 */

#ifndef INCLUDE_META_UTILS_HPP_
#define INCLUDE_META_UTILS_HPP_

#include <xtomic/aux/inttypes.hpp>
#include "xtraits.hpp"

namespace xtomic
{

template<int A, int B>
struct get_min: public integral_const<int, ((A < B) ? A : B)>
{
};

template<int A, int B>
struct get_max: public integral_const<int, ((A > B) ? A : B)>
{
};

template<int A, int B>
struct is_greater: public integral_const<bool, (A > B)>
{
};

template<typename A, typename B>
struct is_greater_size: public is_greater<sizeof(A), sizeof(B)>
{
};

template<bool Value>
struct get_static_assert_size
{
    static const int value = 1;
};

template<>
struct get_static_assert_size<false>
{
    static const int value = -1;
};

template<bool Value>
struct simple_static_assert
{
    char buff[get_static_assert_size<Value>::value];
};

}

#endif /* INCLUDE_META_UTILS_HPP_ */
