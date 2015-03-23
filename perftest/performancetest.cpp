/*
 * performancetest.cpp
 *
 *  Created on: Mar 23, 2015
 *      Author: masha
 */

#include "performancetest.hpp"

namespace lfds
{
namespace perftest
{

IPerformanceTest::~IPerformanceTest()
{

}

PerformanceTest::PerformanceTest(IPerformanceTest* impl,
           const char* units,
           const char* displayName,
           const char* group) :
        m_impl(impl),
        m_units(units),
        m_displayName(displayName),
        m_group(group)
{

}

PerformanceTest::PerformanceTest(PerformanceTest && other) :
        m_impl(other.m_impl),
        m_units(other.m_units),
        m_displayName(other.m_displayName),
        m_group(other.m_group)
{
    other.m_impl = nullptr;
}

PerformanceTest::~PerformanceTest()
{
    delete m_impl;
}

}
}
