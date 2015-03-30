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
    PerfTestFactoryImpl(const char* group,
                        const char* name,
                        const char* displayName,
                        const char** labels,
                        const char* units)
    {
        m_info.m_group = group;
        m_info.m_name = name;
        m_info.m_displayName = displayName;
        m_info.m_labels = labels;
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
