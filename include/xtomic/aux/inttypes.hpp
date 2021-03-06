/*
 * inttypes.hpp
 *
 *  Created on: Mar 31, 2015
 *      Author: masha
 */

/// @file inttypes.hpp
///
/// @brief Types and meta-types for integer types.
///
/// The header provides type definitions and meta-type definitions for better using integer types.
///

#ifndef INCLUDE_INTTYPES_HPP_
#define INCLUDE_INTTYPES_HPP_

namespace xtomic
{

typedef char int8_t;                        ///< signed 1-byte (8-bit) integer.
typedef short int int16_t;                  ///< signed 2-byte (16-bit) integer.
typedef int int32_t;                        ///< signed 4-byte (32-bit) integer.
typedef long long int int64_t;              ///< signed 8-byte (64-bit) integer.

typedef unsigned char uint8_t;              ///< unsigned 1-byte (8-bit) integer.
typedef unsigned short int uint16_t;        ///< unsigned 2-byte (16-bit) integer.
typedef unsigned int uint32_t;              ///< unsigned 4-byte (32-bit) integer.
typedef unsigned long long int uint64_t;    ///< unsigned 8-byte (64-bit) integer.

///
/// @brief Instantiate integer of specified size.
///
/// Meta structure that instantiates signed integer of specified size in bytes.
/// Supported sizes: 1-byte (8-bit), 2-bytes (16-bit), 4-bytes (32-bits), 8-bytes (64-bits).
///
/// Example:
/// @code
/// typedef get_int_by_size<8>::type signed_qword;
/// typedef get_int_by_size<1>::type signed_byte;
/// @endcode
///
template<int size>
struct get_int_by_size;

/// @cond HIDDEN_SYMBOLS
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
/// @endcond

///
/// @brief Instantiate unsigned integer of specified size.
///
/// Meta structure that instantiates unsigned integer of specified size in bytes.
/// Supported sizes: 1-byte (8-bit), 2-bytes (16-bit), 4-bytes (32-bits), 8-bytes (64-bits).
///
/// Example:
/// @code
/// typedef get_int_by_size<8>::type qword_t;
/// typedef get_uint_by_size<1>::type byte_t;
/// @endcode
///
template<int size>
struct get_uint_by_size;

/// @cond HIDDEN_SYMBOLS
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
/// @endcond

}

#endif /* INCLUDE_INTTYPES_HPP_ */
