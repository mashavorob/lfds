/*
 * my-noncopyable.hpp
 *
 *  Created on: Apr 14, 2015
 *      Author: masha
 */

#ifndef UTILS_UTILS_MY_NONCOPYABLE_HPP_
#define UTILS_UTILS_MY_NONCOPYABLE_HPP_

#include <xtomic/aux/cppbasics.hpp>

namespace xtomic
{
namespace my
{

class noncopyable
{
private:
    noncopyable(const noncopyable&)
#if LFDS_USE_CPP11
    = delete
#endif
    ;
    noncopyable& operator=(const noncopyable&)
#if LFDS_USE_CPP11
    = delete
#endif
    ;
};

}
}
#endif /* UTILS_UTILS_MY_NONCOPYABLE_HPP_ */
