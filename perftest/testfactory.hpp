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

template<typename Test>
class PerfTestFactoryImpl: public IPerfTestFactory
{
public:
    PerfTestFactoryImpl(const char* group,
                        const char* name,
                        const char* param,
                        const char* units)
    {
        m_info.m_group = group;
        m_info.m_name = name;
        m_info.m_param = param;
        m_info.m_units = units;
        m_info.m_factory = this;
        PerfTestLocator::registerTest(&m_info);
    }

    // override
    IPerformanceTest* create() const
    {
        return new Test();
    }
private:
    PerfTestInfo m_info;
};

}
}

#endif /* PERFTEST_TESTFACTORY_HPP_ */
