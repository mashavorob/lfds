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

    double doTest() const = 0;
};

class PerformanceTest
{
public:
    PerformanceTest(IPerformanceTest* impl,
         const char* units,
         const char* displayName,
         const char* group);

    PerformanceTest(PerformanceTest && other);

    PerformanceTest(const PerformanceTest & other) = delete;

    ~PerformanceTest();

    const IPerformanceTest* operator->() const
    {
        return m_impl;
    }
    const std::string & getUnits() const
    {
        return m_units;
    }
    const std::string & getDisplayName() const
    {
        return m_displayName;
    }
    const std::string & getGroup() const
    {
        return m_group;
    }

private:
    const IPerformanceTest* m_impl;
    std::string m_units;            // measure units
    std::string m_displayName;      // test's display name
    std::string m_group;            // test's group name
};

}
}

#endif /* PERFTEST_PERFORMANCETEST_HPP_ */
