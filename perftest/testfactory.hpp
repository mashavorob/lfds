/*
 * testfactory.hpp
 *
 *  Created on: Mar 23, 2015
 *      Author: masha
 */

#ifndef PERFTEST_TESTFACTORY_HPP_
#define PERFTEST_TESTFACTORY_HPP_

#include "testlocator.hpp"

namespace lfds
{
namespace perftest
{

class IPerformanceTest;

class IPerfTestFactory
{
public:
    virtual ~IPerfTestFactory();

    virtual IPerformanceTest* create() const = 0;
};

template<class Test>
class PerfTestFactoryImpl: public IPerfTestFactory
{
public:
    PerfTestFactoryImpl(PerfTestInfo& info)
    {
        PerfTestLocator::registerTest(&info);
    }

    // override
    IPerformanceTest* create() const
    {
        return new Test();
    }
};

}
}

#endif /* PERFTEST_TESTFACTORY_HPP_ */
