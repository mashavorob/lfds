/*
 * xfunctional.hpp
 *
 *  Created on: Apr 1, 2015
 *      Author: masha
 */

/// @file xfunctional.hpp
///
/// @brief The file implements bridge for uniform using C++11 functors in C++98.
///
/// C++11 makes available some new functors that were defined under tr1 namespace in C++98.
///

#ifndef INCLUDE_XFUNCTIONAL_HPP_
#define INCLUDE_XFUNCTIONAL_HPP_

/// @cond HIDDEN_SYMBOLS
#include <xtomic/config.hpp>
/// @endcond

///
/// @brief Helper meta class for using stl::hash<>
///
/// The helper allows uniformly instantiate stl::hash regarding of C++ version:
///
/// - `std::hash<>` when C++11 is used.
/// - `std::tr1::hash<T>` when old compiler is used.
///
/// Example:
/// @code
/// typedef xtomic::make_hash<std::string>::type string_hash_type;
/// @endcode
///
template<typename T>
struct make_hash;


#if XTOMIC_USE_CPP11

#include <xtomic/impl/xfunctional-modern.hpp>

#else

#include <xtomic/impl/xfunctional-obsolete.hpp>

#endif

#endif /* INCLUDE_XFUNCTIONAL_HPP_ */
