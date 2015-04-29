/*
 * testmem.cpp
 *
 *  Created on: Mar 30, 2015
 *      Author: masha
 */

#include "testmem.hpp"
#include "testallocator.hpp"

namespace lfds
{
namespace perftest
{

double MemConsumptionTest::doTest()
{
    std::size_t cbBefore = allocator_base::getAllocated();
    m_tester->doTest();
    std::size_t cbAfter = allocator_base::getAllocated();

    const double MbSize = 1024. * 1024.;
    const double memSize = static_cast<double>(cbAfter - cbBefore) / MbSize;
    return memSize * m_mult;
}

}
}
