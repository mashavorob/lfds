/*
 * hash_utils.hpp
 *
 *  Created on: Jan 22, 2015
 *      Author: masha
 */

#ifndef INCLUDE_HASH_UTILS_HPP_
#define INCLUDE_HASH_UTILS_HPP_

#include <functional>

namespace lfds
{

// Code from boost
template<class T>
inline void hash_combine(std::size_t& seed, T const& v)
{
    seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

}

#endif /* INCLUDE_HASH_UTILS_HPP_ */
