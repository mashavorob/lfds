/*
 * timeutils.h
 *
 *  Created on: Mar 25, 2015
 *      Author: masha
 */

#ifndef PERFTEST_TIMEUTILS_HPP_
#define PERFTEST_TIMEUTILS_HPP_

#include <ctime>

namespace xtomic
{
namespace perftest
{

const long int NS_PER_SEC = static_cast<long int>(1e9);

timespec operator-(const timespec &a, const timespec &b);

double seconds(const timespec &a);

}
}

#endif /* PERFTEST_TIMEUTILS_HPP_ */
