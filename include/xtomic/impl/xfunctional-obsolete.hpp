/*
 * xfunctional-obsolete.hpp
 *
 *  Created on: Apr 1, 2015
 *      Author: masha
 */

#ifndef INCLUDE_XFUNCTIONAL_OBSOLETE_HPP_
#define INCLUDE_XFUNCTIONAL_OBSOLETE_HPP_

#include <tr1/functional>

namespace xtomic
{

template<typename T>
struct make_hash
{
    typedef std::tr1::hash<T> type;
};

}

#endif /* INCLUDE_XFUNCTIONAL_OBSOLETE_HPP_ */
