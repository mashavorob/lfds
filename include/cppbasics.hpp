/*
 * cppbasics.hpp
 *
 *  Created on: Mar 31, 2015
 *      Author: masha
 */

#ifndef INCLUDE_CPPBASICS_HPP_
#define INCLUDE_CPPBASICS_HPP_

#include "config.hpp"

#if LFDS_USE_CPP11

#define std_forward(ARGS, args)  std::forward<ARGS>(args)...
#define std_move(v)  std::move(v)

#else

#define std_forward(ARGS, args)  args
#define std_move(v)  v

#define nullptr 0
#define constexpr const

#endif // LFDS_USE_CPP11



#endif /* INCLUDE_CPPBASICS_HPP_ */
