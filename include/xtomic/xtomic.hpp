/*
 * async.hpp
 *
 *  Created on: Mar 31, 2015
 *      Author: masha
 */

#ifndef INCLUDE_XTOMIC_HPP_
#define INCLUDE_XTOMIC_HPP_

#include <xtomic/config.hpp>

namespace lfds
{

namespace barriers
{
enum erelaxed
{
    relaxed
};
enum eacquire
{
    acquire
};
enum erelease
{
    release
};
enum efull
{
    full
};
}

}

#if LFDS_USE_CPP11

#include "impl/xtomic-modern.hpp"

#else

#include "impl/xtomic-obsolete.hpp"

#endif

#endif /* INCLUDE_XTOMIC_HPP_ */
