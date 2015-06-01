/*
 * hash_utils.hpp
 *
 *  Created on: Jan 22, 2015
 *      Author: masha
 */

/// @file hash_utils.hpp
///
/// @brief Hash related helpers.
///
/// The file provides convenient way of building hash functions for complex objects.
///

#ifndef INCLUDE_HASH_UTILS_HPP_
#define INCLUDE_HASH_UTILS_HPP_

#include <xtomic/aux/xfunctional.hpp>

namespace xtomic
{

///
/// @brief Helper function to combine hash codes (mimics similar function from boost library).
///
/// The function allows easily build hash functions for complex objects. Example:
///
/// @code
/// struct FOO
/// {
///     std::string a;
///     int         b;
///     ...
/// };
///
/// struct FOO_hasher
/// {
///     std::size_t operator()(const FOO& arg) const
///     {
///         std::size_t seed = 0;
///         hash_combine(seed, arg.a);
///         hash_combine(seed, arg.b);
///         ...
///         return seed;
///     }
/// };
/// @endcode
///
template<typename T, typename Hash = make_hash<T>::type>
inline void hash_combine(std::size_t& seed, T const& v)
{
    typedef typename make_hash<T>::type hash_type;
    seed ^= hash_type<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

}

#endif /* INCLUDE_HASH_UTILS_HPP_ */
