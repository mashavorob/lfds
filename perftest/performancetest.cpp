/*
 * performancetest.cpp
 *
 *  Created on: Mar 23, 2015
 *      Author: masha
 */

#include "performancetest.hpp"

#include <xtomic/aux/cppbasics.hpp>

namespace xtomic
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

void PerformanceTest::attach(IPerformanceTest* impl)
{
    delete m_impl;
    m_impl = impl;
}

PerformanceTest::~PerformanceTest()
{
    delete m_impl;
}

}
}
