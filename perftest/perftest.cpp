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

#include <cppbasics.hpp>


#include <cassert>
#include <iostream>
#include <cmath>

using namespace lfds::perftest;

template<class Pred>
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

template<class Pred>
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

template<class Pred>
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
    void operator()(group_type::const_iterator iter)
    {
        const std::string & name = iter->first;
        std::cout << "    name: " << name << ": " << std::endl;
    }

    void operator()(groups_type::const_iterator iter)
    {
        const std::string & name = iter->first;
        std::cout << "Group: " << name << ": " << std::endl;
    }
};

struct Display: public Generic
{
    void operator()(ids_type::const_iterator iter)
    {
        const PerfTestLocator& locator = PerfTestLocator::getInstance();

        const id_type id = *iter;
        const char* displayName = locator.getTestDisplayName(id);
        const char** labels = locator.getTestLabels(id);

        std::cout << "        parameter:" << displayName << std::endl;
        std::cout << "        labels: ";

        if (labels[0])
        {
            std::cout << labels[0];
            for (int i = 1; labels[i]; ++i)
            {
                std::cout << ", " << labels[i];
            }
        }
        std::cout << std::endl << std::endl;
    }
    void operator()(group_type::const_iterator iter)
    {
        const std::string & name = iter->first;
        std::cout << "    name: " << name << ": " << std::endl;
    }

    void operator()(groups_type::const_iterator iter)
    {
        const std::string & name = iter->first;
        std::cout << "Group: " << name << ": " << std::endl;
    }
};

struct RunTest: public Generic
{
    void operator()(ids_type::const_iterator iter)
    {
        const PerfTestLocator& locator = PerfTestLocator::getInstance();

        const id_type id = *iter;
        const char* displayName = locator.getTestDisplayName(id);
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
        const std::string & name = iter->first;
        std::cout << "    name: " << name << ": " << std::endl;
    }

    void operator()(groups_type::const_iterator iter)
    {
        const std::string & name = iter->first;
        std::cout << "Group: " << name << ": " << std::endl;
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

    Display display;
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
    LFDS_VERSION_MAJOR << "." << LFDS_VERSION_MINOR << " Performance Test"
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

