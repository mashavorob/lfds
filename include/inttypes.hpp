/*
 * inttypes.hpp
 *
 *  Created on: Mar 31, 2015
 *      Author: masha
 */

#ifndef INCLUDE_INTTYPES_HPP_
#define INCLUDE_INTTYPES_HPP_

namespace lfds
{

typedef char int8_t;
typedef short int int16_t;
typedef int int32_t;
typedef long long int int64_t;

typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long int uint64_t;

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

}

#endif /* INCLUDE_INTTYPES_HPP_ */
