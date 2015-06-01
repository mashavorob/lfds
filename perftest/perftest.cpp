/*
 * perftest.cpp
 *
 *  Created on: Mar 23, 2015
 *      Author: masha
 */

#include "testlocator.hpp"
#include "testtree.hpp"
#include "testfilter.hpp"
#include "cmdlineparser.hpp"

#include <xtomic/aux/cppbasics.hpp>

#include <cassert>
#include <iostream>
#include <cmath>

using namespace xtomic::perftest;

template<typename Pred>
void forEachItem(const ids_type & ids, Pred pred)
{
    ids_type::const_iterator beg = ids.begin();
    ids_type::const_iterator end = ids.end();

    const PerfTestLocator& locator = PerfTestLocator::getInstance();

    for (ids_type::const_iterator i = beg; i != end; ++i)
    {
        pred(i);
    }
}

template<typename Pred>
void forEachName(const group_type & group, Pred pred)
{
    group_type::const_iterator beg = group.begin();
    group_type::const_iterator end = group.end();

    for (group_type::const_iterator i = beg; i != end; ++i)
    {
        pred(i);
        const ids_type & ids = i->second;
        forEachItem(ids, pred);
    }
}

template<typename Pred>
void forEachGroup(const groups_type & groups, Pred pred)
{
    groups_type::const_iterator beg = groups.begin();
    groups_type::const_iterator end = groups.end();

    for (groups_type::const_iterator i = beg; i != end; ++i)
    {
        pred(i);
        const group_type & group = i->second;
        forEachName(group, pred);
    }
}

struct Generic
{
    void operator()(ids_type::const_iterator iter)
    {
        const PerfTestLocator& locator = PerfTestLocator::getInstance();

        const id_type id = *iter;
        const char* paramName = locator.getTestParam(id);

        std::cout << "        parameter:" << paramName << std::endl;
    }
    void operator()(group_type::const_iterator iter)
    {
        const std::string & name = iter->first;
        std::cout << "    object: " << name << std::endl;
    }

    void operator()(groups_type::const_iterator iter)
    {
        const std::string & name = iter->first;
        std::cout << "group: " << name << std::endl;
    }
};

struct RunTest: public Generic
{
    void operator()(ids_type::const_iterator iter)
    {
        const PerfTestLocator& locator = PerfTestLocator::getInstance();

        const id_type id = *iter;
        const char* displayName = locator.getTestParam(id);
        const char* units = locator.getTestUnits(id);
        PerformanceTest test;
        locator.getTest(id, test);

        std::cout << "        " << displayName << ": running\r";
        std::cout.flush();

        const double res = normalize(test->doTest());

        std::cout << "        " << displayName << ": " << res << " " << units
                << "        " << std::endl;
    }
    void operator()(group_type::const_iterator iter)
    {
        (*static_cast<Generic*>(this))(iter);
    }

    void operator()(groups_type::const_iterator iter)
    {
        (*static_cast<Generic*>(this))(iter);
    }

    static double normalize(double val)
    {
        if (fabs(val) > 10)
        {
            val = round(val);
        }
        else if (fabs(val) > 5)
        {
            val = round(val * 10) / 10.;
        }
        else if (fabs(val) > 1)
        {
            val = round(val * 100) / 100.;
        }
        else
        {
            val = round(val * 1000) / 1000.;
        }
        return val;
    }

};

void listTests()
{
    groups_type groups = TestTree(Filter::getAllTests()).get();

    Generic display;
    forEachGroup(groups, display);
}

void runTests(const ids_type & ids)
{
    groups_type groups = TestTree(ids).get();

    RunTest run;
    forEachGroup(groups, run);
}

int main(int argc, const char** argv)
{
    std::cout << "Lock Free Data Structures v" <<
    XTOMIC_VERSION_MAJOR << "." << XTOMIC_VERSION_MINOR << " Performance Test"
            << std::endl;

    ids_type ids;
    CommandLineParser::Command cmd = CommandLineParser::parse(argc, argv, ids);
    switch (cmd)
    {
    case CommandLineParser::cmdShowHelp:
        CommandLineParser::showHelp(argv[0]);
        break;
    case CommandLineParser::cmdListTests:
        listTests();
        break;
    case CommandLineParser::cmdRunTests:
        runTests(ids);
        break;
    case CommandLineParser::cmdError:
        CommandLineParser::showHelp(argv[0]);
        return 1;
    default:
        assert(false);
    }

    return 0;
}

