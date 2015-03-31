/*
 * testtree.cpp
 *
 *  Created on: Mar 24, 2015
 *      Author: masha
 */

#include "testtree.hpp"
#include "testlocator.hpp"

#include <algorithm>
#include <utility>

namespace lfds
{
namespace perftest
{

namespace {

class Predicate
{
public:
    Predicate(groups_type &groups) : m_groups(groups), m_locator(PerfTestLocator::getInstance())
    {

    }

    void operator()(const id_type id)
    {
        const char* groupName = m_locator.getTestGroup(id);
        const char* name = m_locator.getTestName(id);
        group_type & group = m_groups[groupName];
        ids_type & ids = group[name];
        ids.insert(id);
    }
private:
    groups_type & m_groups;
    const PerfTestLocator & m_locator;
};

}

TestTree::TestTree(const ids_type & plainSet)
{
    std::for_each(plainSet.begin(), plainSet.end(), Predicate(m_groups));
}

}}


