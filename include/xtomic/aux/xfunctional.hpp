/*
 * xfunctional.hpp
 *
 *  Created on: Apr 1, 2015
 *      Author: masha
 */

#ifndef INCLUDE_XFUNCTIONAL_HPP_
#define INCLUDE_XFUNCTIONAL_HPP_

#include <xtomic/config.hpp>

#if LFDS_USE_CPP11

#include <xtomic/impl/xfunctional-modern.hpp>

#else

#include <xtomic/impl/xfunctional-obsolete.hpp>

#endif

#endif /* INCLUDE_XFUNCTIONAL_HPP_ */
