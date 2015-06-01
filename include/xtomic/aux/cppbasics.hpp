/*
 * cppbasics.hpp
 *
 *  Created on: Mar 31, 2015
 *      Author: masha
 */

///
/// @file cppbasics.hpp
///
/// @brief Basic C++ macros for compatibility with C++98.
///
/// The file implements helper macro definitions including ones to mimic such great thing from
/// C++11 like variadic template arguments in C++98. Certainly these macros do not extend C++98
/// but allow you to spend less effort writing portable code.
///

#ifndef INCLUDE_CPPBASICS_HPP_
#define INCLUDE_CPPBASICS_HPP_

/// @cond HIDDEN_SYMBOLS
#include <xtomic/config.hpp>
/// @endcond

///
/// @defgroup cpp11_specifics C++11 Specifics
///
/// @brief Uniform using of C++11 specific directives when C++98 is used.
///
/// Macro definitions that provide uniform using for new C++11 features:
/// - `std::forward<>()`, `std::move()`
/// - alignment directives
///
/// Macro-definitions provide compatibility with Eclipse CDT Luna (it seems the IDE has troubles with
/// support `alignas` directive).
///
/// @{

#if XTOMIC_USE_CPP11

/// @cond HIDDEN_SYMBOLS
#define _std_forward_impl(ARGS, args) std::forward<ARGS>(args)...
#define _std_move_impl(v) std::move(v)
/// @endcond

#else

/// @cond HIDDEN_SYMBOLS
#define _std_forward_impl(ARGS, args) args
#define _std_move_impl(v) v
/// @endcond

#define nullptr 0        ///< defines `nullptr` when C++98 is used.
#define constexpr const  ///< defines `constexpr` when C++98 is used.

#endif // XTOMIC_USE_CPP11

///
/// @brief Mimics std::forward<>() when C++98 is used.
///
/// Example:
///
/// @code
/// #if C++11
/// template<typename ... Args>
/// bool insert(const key_type & key, Args&&... val)
/// #else
/// bool insert(const key_type & key, const mapped_type& val)
/// #endif
/// {
///     return m_hash_table_base.insert(key, false, std_forward(Args, val));
/// }
/// @endcode
///
/// When C++11 is used then the codesnipet will expand to:
///
/// @code
/// template<typename ... Args>
/// bool insert(const key_type & key, Args&&... val)
/// {
///     return m_hash_table_base.insert(key, false, std::forward<Args>(val)...);
/// }
/// @endcode
///
/// and when C++98:
///
/// @code
/// bool insert(const key_type & key, const mapped_type& val)
/// {
///     return m_hash_table_base.insert(key, false, val);
/// }
/// @endcode
///
#define std_forward(ARGS, args) _std_forward_impl(ARGS, args)

///
/// @def std_move(v)
///
/// @brief Mimics std::move() when C++98 is used.
///
/// Example:
///
/// @code
/// class Foo
/// {
/// public:
/// #if C++11
///    Foo(Foo&& other);
/// #else
///    Foo(Foo& other);
/// #endif
///    ...
/// };
///
/// ...
/// Foo dummy( std_move(other) );
/// ...
/// @endcode
///
#define std_move(v) _std_move_impl(v)

/// @brief Mimics C++11 alignas( expression ) form.
#define align_by(size) __attribute__((aligned((size))))

/// @brief Mimics C++11 alignas( type-id ) form.
#define align_as(type) __attribute__((aligned(__alignof(type))))

/// @brief Shortcut to declare alignment for 16-bytes CAS operation (8-byte CAS on 32 bit platform)
#define align_4_cas16 align_by(sizeof(void*)*2)
/// @}

#endif /* INCLUDE_CPPBASICS_HPP_ */
