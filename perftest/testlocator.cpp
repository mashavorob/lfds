/*
 * testlocator.cpp
 *
 *  Created on: Mar 23, 2015
 *      Author: masha
 */

#include "testlocator.hpp"
#include "performancetest.hpp"
#include "testfactory.hpp"

#include <stdexcept>


namespace lfds
{
namespace perftest
{

PerfTestInfo* PerfTestLocator::m_link = nullptr;

PerfTestLocator::PerfTestLocator()
{

}

const PerfTestLocator& PerfTestLocator::getInstance()
{
    static const PerfTestLocator instance;

    return instance;
}

void PerfTestLocator::registerTest(PerfTestInfo* info)
{
    info->m_id = getSize();
    info->m_link = m_link;
    m_link = info;
}

PerformanceTest PerfTestLocator::getTest(const id_type id) const
{
    return PerformanceTest(at(id)->m_factory->create());
}

const PerfTestInfo* PerfTestLocator::at(const id_type id) const
{
    if ( id >= getSize() )
    {
        throw std::out_of_range("PerfTestLocator::at()");
    }
    PerfTestInfo* link = m_link;
    while ( link )
    {
        if ( link->m_id == id )
        {
            break;
        }
        link = link->m_link;
    }
    return link;
}

}

}
