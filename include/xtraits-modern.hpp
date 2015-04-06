/*
 * xtraits_new.hpp
 *
 *  Created on: Apr 1, 2015
 *      Author: masha
 */

#ifndef INCLUDE_XTRAITS_MODERN_HPP_
#define INCLUDE_XTRAITS_MODERN_HPP_

#include <type_traits>

namespace lfds
{

template<typename T>
struct is_integral
{
    enum
    {
        value = std::is_integral<T>::value
    };
};



}



#endif /* INCLUDE_XTRAITS_MODERN_HPP_ */
