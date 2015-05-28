/*
 * buffer_traits.hpp
 *
 *  Created on: Dec 15, 2014
 *      Author: masha
 */

#ifndef INCLUDE_BUFFER_TRAITS_HPP_
#define INCLUDE_BUFFER_TRAITS_HPP_

#include "fixed_buffer.hpp"
#include "dynamic_buffer.hpp"

namespace xtomic
{

template<typename T, typename Allocator, bool FixedSize>
struct buffer_traits;

template<typename T, typename Allocator>
struct buffer_traits<T, Allocator, true>
{
    typedef fixed_buffer<T, Allocator> type;
};

template<typename T, typename Allocator>
struct buffer_traits<T, Allocator, false>
{
    typedef dynamic_buffer<T, Allocator> type;
};

}

#endif /* INCLUDE_BUFFER_TRAITS_HPP_ */
