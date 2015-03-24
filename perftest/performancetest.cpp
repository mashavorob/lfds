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

PerformanceTest::PerformanceTest(IPerformanceTest* impl) :
        m_impl(impl)
{

}

PerformanceTest::PerformanceTest(PerformanceTest && other) :
        m_impl(other.m_impl)
{
    other.m_impl = nullptr;
}

PerformanceTest::~PerformanceTest()
{
    delete m_impl;
}

}
}
