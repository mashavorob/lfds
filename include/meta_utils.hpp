/*
 * meta_utils.hpp
 *
 *  Created on: Feb 11, 2015
 *      Author: masha
 */

#ifndef INCLUDE_META_UTILS_HPP_
#define INCLUDE_META_UTILS_HPP_

#include "inttypes.hpp"
#include "xtraits.hpp"

namespace lfds
{

template<int A, int B>
struct get_min : public integral_const<int, ((A < B) ? A : B)>
{
};

template<int A, int B>
struct get_max : public integral_const<int, ((A > B) ? A : B)>
{
};

template<int A, int B>
struct is_greater : public integral_const<bool, (A > B)>
{
};

template<class A, class B>
struct is_greater_size : public is_greater<sizeof(A), sizeof(B)>
{
};

}

#endif /* INCLUDE_META_UTILS_HPP_ */
