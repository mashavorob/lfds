/*
 * stopwatch.hpp
 *
 *  Created on: Mar 25, 2015
 *      Author: masha
 */

#ifndef PERFTEST_STOPWATCH_HPP_
#define PERFTEST_STOPWATCH_HPP_

#include "timeutils.hpp"

namespace xtomic
{
namespace perftest
{

template<typename Operation>
struct Stopwatch
{
    double operator()(Operation & op) const
    {
        timespec start, end;

        clock_gettime(CLOCK_MONOTONIC, &start);
        op();
        clock_gettime(CLOCK_MONOTONIC, &end);

        timespec diff = end - start;
        return seconds(diff);
    }
};

}
}

#endif /* PERFTEST_STOPWATCH_HPP_ */
