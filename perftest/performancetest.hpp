/*
 * performancetest.hpp
 *
 *  Created on: Mar 23, 2015
 *      Author: masha
 */

#ifndef PERFTEST_PERFORMANCETEST_HPP_
#define PERFTEST_PERFORMANCETEST_HPP_

#include <xtomic/aux/cppbasics.hpp>

#include <string>

namespace xtomic
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
    PerformanceTest(IPerformanceTest* impl = nullptr);

    void attach(IPerformanceTest* impl);

    ~PerformanceTest();

    IPerformanceTest* operator->()
    {
        return m_impl;
    }
private:
    PerformanceTest(PerformanceTest & other); // = delete

private:
    IPerformanceTest* m_impl;
};

}
}

#endif /* PERFTEST_PERFORMANCETEST_HPP_ */
