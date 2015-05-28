/*
 * xfunctional.hpp
 *
 *  Created on: Apr 1, 2015
 *      Author: masha
 */

#ifndef INCLUDE_XFUNCTIONAL_HPP_
#define INCLUDE_XFUNCTIONAL_HPP_

#include <xtomic/config.hpp>

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


#if LFDS_USE_CPP11

#include <xtomic/impl/xfunctional-modern.hpp>

#else

#include <xtomic/impl/xfunctional-obsolete.hpp>

#endif

#endif /* INCLUDE_XFUNCTIONAL_HPP_ */
