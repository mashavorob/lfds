/*
 * xfunctional-modern.hpp
 *
 *  Created on: Apr 1, 2015
 *      Author: masha
 */

#ifndef INCLUDE_XFUNCTIONAL_MODERN_HPP_
#define INCLUDE_XFUNCTIONAL_MODERN_HPP_

#include <functional>

namespace lfds
{

template<class T>
struct get_hash
{
    typedef std::hash<T> type;
};

}

#endif /* INCLUDE_XFUNCTIONAL_MODERN_HPP_ */
