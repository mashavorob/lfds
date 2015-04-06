/*
 * testaveragetime.hpp
 *
 *  Created on: Mar 25, 2015
 *      Author: masha
 */

#ifndef PERFTEST_TESTAVERAGETIME_HPP_
#define PERFTEST_TESTAVERAGETIME_HPP_

#include "stopwatch.hpp"
#include "performancetest.hpp"

namespace lfds
{
namespace perftest
{

template<class Tester, unsigned int Multiplier>
class AverageOpTimeTest: public IPerformanceTest
{
public:
    typedef Tester tester_type;

    static const unsigned int count = tester_type::count;
    static const unsigned int mult = Multiplier;

    double doTest()
    {
        tester_type tester;
        Stopwatch<tester_type> stopwatch;

        const double duration = stopwatch(tester);
        const double performance = duration/static_cast<double>(count)*static_cast<double>(mult);

        return performance;
    }
};

}
}

#endif /* PERFTEST_TESTAVERAGETIME_HPP_ */
