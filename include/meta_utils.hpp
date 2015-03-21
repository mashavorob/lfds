/*
 * meta_utils.hpp
 *
 *  Created on: Feb 11, 2015
 *      Author: masha
 */

#ifndef INCLUDE_META_UTILS_HPP_
#define INCLUDE_META_UTILS_HPP_

#include <cstdint>

namespace lfds
{
namespace
{

template<int size>
struct get_int_by_size;

template<>
struct get_int_by_size<1>
{
    typedef int8_t type;
};

template<>
struct get_int_by_size<2>
{
    typedef int16_t type;
};

template<>
struct get_int_by_size<4>
{
    typedef int32_t type;
};

template<>
struct get_int_by_size<8>
{
    typedef int64_t type;
};

template<int size>
struct get_uint_by_size;

template<>
struct get_uint_by_size<1>
{
    typedef uint8_t type;
};

template<>
struct get_uint_by_size<2>
{
    typedef int16_t type;
};

template<>
struct get_uint_by_size<4>
{
    typedef uint32_t type;
};

template<>
struct get_uint_by_size<8>
{
    typedef uint64_t type;
};

template<int A, int B>
struct get_min
{
    static const int value = (A < B) ? A : B;
};

template<int A, int B>
struct get_max
{
    static const int value = (A > B) ? A : B;
};

template<int A, int B>
struct is_greater
{
    static const bool value = A > B;
};

template<class A, class B>
struct is_greater_size
{
    static const bool value = is_greater<sizeof(A), sizeof(B)>::value;
};

}
}

#endif /* INCLUDE_META_UTILS_HPP_ */
