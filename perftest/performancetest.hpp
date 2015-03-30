/*
 * performancetest.hpp
 *
 *  Created on: Mar 23, 2015
 *      Author: masha
 */

#ifndef PERFTEST_PERFORMANCETEST_HPP_
#define PERFTEST_PERFORMANCETEST_HPP_

#include <string>

namespace lfds
{
namespace perftest
{

class IPerformanceTest
{
public:
    virtual ~IPerformanceTest();

    virtual double doTest() = 0;
};

class PerformanceTest
{
public:
    PerformanceTest(IPerformanceTest* impl);

    PerformanceTest(PerformanceTest && other);

    PerformanceTest(const PerformanceTest & other) = delete;

    ~PerformanceTest();

    IPerformanceTest* operator->()
    {
        return m_impl;
    }
private:
    IPerformanceTest* m_impl;
};

}
}

#endif /* PERFTEST_PERFORMANCETEST_HPP_ */
