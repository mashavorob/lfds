/*
 * async.hpp
 *
 *  Created on: Mar 31, 2015
 *      Author: masha
 */

#ifndef INCLUDE_XTOMIC_HPP_
#define INCLUDE_XTOMIC_HPP_

#include "config.hpp"

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

//
// GCC prior v4.4 does not enforce SW barriers on atomic operations
// so we have to do it manually.
//
// So functions atomic_prologue()/atomic_prologue() have different implementations
// depending on GCC version: thread fence for GCC prior 4.4 and void otherwise.

inline void atomic_prologue();

inline void atomic_epilogue();

class atomic_guard
{
public:
    atomic_guard()
    {
        atomic_prologue();
    }
    ~atomic_guard()
    {
        atomic_epilogue();
    }
};

}

#if LFDS_USE_CPP11

#include <xtomic-modern.hpp>

#else

#include <xtomic-obsolete.hpp>

#endif




#endif /* INCLUDE_XTOMIC_HPP_ */
