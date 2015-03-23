/*
 * testlocator.cpp
 *
 *  Created on: Mar 23, 2015
 *      Author: masha
 */

#include "testlocator.hpp"
#include "performancetest.hpp"
#include "testfactory.hpp"

#include <exception>


namespace lfds
{
namespace perftest
{

PerfTestInfo* PerfTestLocator::m_link = nullptr;
static const char* const all = "all";

PerfTestLocator::PerfTestLocator()
{

}

void PerfTestLocator::registerTest(PerfTestInfo* info)
{
    info->m_link = m_link;
    m_link = info;
}

PerfTestLocator::collection_type PerfTestLocator::getGroups() const
{
    collection_type coll;

    PerfTestInfo* link = m_link;
    while ( link )
    {
        coll.insert(link->m_group);
        link = link->m_link;
    }
    return coll;
}

PerfTestLocator::collection_type PerfTestLocator::getTests(const std::string & group) const
{
    collection_type coll;

    PerfTestInfo* link = m_link;
    while ( link )
    {
        if ( matchGroup(group, link) )
        {
            coll.insert(link->m_name);
        }
        link = link->m_link;
    }
    return coll;
}

PerfTestLocator::collection_type PerfTestLocator::getLabels(const std::string & group, const std::string & test) const
{
    collection_type coll;

    PerfTestInfo* link = m_link;
    while ( link )
    {
        if ( group == link->m_group && test == link->m_name )
        {
            for ( int i = 0; link->m_labels[i]; ++i )
            {
                coll.insert(link->m_labels[i]);
            }
            break;
        }
        link = link->m_link;
    }
    return coll;
}

PerfTestLocator::collection_type PerfTestLocator::select(const std::string & group, const PerfTestLocator::collection_type & labels) const
{
    collection_type coll;

    PerfTestInfo* link = m_link;
    while ( link )
    {
        if ( matchGroup(group, link) )
        {
            for ( int i = 0; link->m_labels[i]; ++i )
            {
                if ( labels.empty() || labels.find(link->m_labels[i]) != labels.end() )
                {
                    coll.insert(link->m_labels[i]);
                }
            }
        }
    }
    return coll;
}

PerformanceTest PerfTestLocator::getTest(const std::string & group, const std::string & test) const
{
    PerfTestInfo* link = m_link;
    while ( link )
    {
        if ( group == link->m_group && test == link->m_name )
        {
            IPerformanceTest* impl = link->m_factory->create();
            return PerformanceTest(impl, link->m_units, link->m_displayName, link->m_group);
        }
        link = link->m_link;
    }
    throw std::out_of_range();
    return PerformanceTest(nullptr, nullptr, nullptr, nullptr);
}

bool PerfTestLocator::matchGroup(const std::string & group, const PerfTestInfo* info)
{
    return ( group == all || group == info->m_group );
}

}

}
