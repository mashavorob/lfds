/*
 * testmaxtime.cpp
 *
 *  Created on: Mar 25, 2015
 *      Author: masha
 */

#include "testmaxtime.hpp"
#include "testaveragetime.hpp"
#include <ctime>

namespace xtomic
{
namespace perftest
{
namespace
{

class ClockTester
{
public:
    static const unsigned int count = static_cast<unsigned int>(1e6);

    void operator()() const
    {
        timespec dummy;

        for (unsigned int i = 0; i < count; ++i)
        {
            clock_gettime(CLOCK_MONOTONIC, &dummy);
        }
    }
};

typedef AverageOpTimeTest<ClockTester, 1> clock_pref_count;

}

const double get_timer_perf::value = clock_pref_count().doTest();

}
}

