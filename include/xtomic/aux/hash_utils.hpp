/*
 * hash_utils.hpp
 *
 *  Created on: Jan 22, 2015
 *      Author: masha
 */

#ifndef INCLUDE_HASH_UTILS_HPP_
#define INCLUDE_HASH_UTILS_HPP_

#include <xtomic/aux/xfunctional.hpp>

namespace xtomic
{

/// @brief Helper function to combine hash codes (taken from from boost).
template<typename T>
inline void hash_combine(std::size_t& seed, T const& v)
{
    typedef typename make_hash<T>::type hash_type;
    seed ^= hash_type<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

}

#endif /* INCLUDE_HASH_UTILS_HPP_ */
