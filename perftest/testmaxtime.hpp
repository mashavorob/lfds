/*
 * testmaxtime.hpp
 *
 *  Created on: Mar 25, 2015
 *      Author: masha
 */

#ifndef PERFTEST_TESTMAXTIME_HPP_
#define PERFTEST_TESTMAXTIME_HPP_


#include "stopwatch.hpp"
#include "performancetest.hpp"

namespace lfds
{
namespace perftest
{

struct get_timer_perf
{
    static const double value;
};

template<class Tester, unsigned int Multiplier>
class MaximumOpTimeTest: public IPerformanceTest
{
public:
    typedef Tester tester_type;

    static const unsigned int count = tester_type::count;
    static const unsigned int mult = Multiplier;

    double doTest()
    {
        tester_type tester;
        Stopwatch<tester_type> stopwatch;

        double max_time = stopwatch(tester);

        for ( unsigned int i = 0; i < count; ++i )
        {
            const double t = stopwatch(tester);
            if ( t > max_time )
            {
                max_time = t;
            }
        }

        max_time -= get_timer_perf::value;

        const double performance = max_time*static_cast<double>(mult);
        return performance;
    }
};

}
}


#endif /* PERFTEST_TESTMAXTIME_HPP_ */
