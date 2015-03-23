/*
 * testlocator.hpp
 *
 *  Created on: Mar 23, 2015
 *      Author: masha
 */

#ifndef PERFTEST_TESTLOCATOR_HPP_
#define PERFTEST_TESTLOCATOR_HPP_

#include "performancetest.hpp"

#include <set>
#include <string>

namespace lfds
{
namespace perftest
{

class IPerfTestFactory;

struct PerfTestInfo
{
    PerfTestInfo* m_link;

    const char* m_name;
    const char* m_displayName;
    const char* m_group;
    const char** m_labels;
    const char* m_units;

    IPerfTestFactory* m_factory;
};

class PerfTestLocator
{
private:
    PerfTestLocator();
public:

    typedef std::set<std::string> collection_type;

    const PerfTestLocator& getInstance();

    static void registerTest(PerfTestInfo* info);

    collection_type getGroups() const;

    collection_type getTests(const std::string & group) const;

    collection_type getLabels(const std::string & group, const std::string & test) const;

    collection_type select(const std::string & group, const collection_type & labels) const;

    PerformanceTest getTest(const std::string & group, const std::string & test) const;
private:
    static bool matchGroup(const std::string & group, const PerfTestInfo* info);
private:
    static PerfTestInfo* m_link;
};

}
}


#endif /* PERFTEST_TESTLOCATOR_HPP_ */
