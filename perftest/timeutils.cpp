/*
 * timeutils.cpp
 *
 *  Created on: Mar 25, 2015
 *      Author: masha
 */

#include "timeutils.hpp"

namespace lfds
{
namespace perftest
{

timespec operator-(const timespec &a, const timespec &b)
{
    timespec res = a;

    if ( res.tv_nsec < b.tv_nsec )
    {
        --res.tv_sec;
        res.tv_nsec += NS_PER_SEC;
    }

    res.tv_sec -= b.tv_sec;
    res.tv_nsec -= b.tv_nsec;
    return res;
}

double seconds(const timespec &a)
{
    return static_cast<double>(a.tv_sec) + static_cast<double>(a.tv_nsec)/static_cast<double>(NS_PER_SEC);
}

}
}


