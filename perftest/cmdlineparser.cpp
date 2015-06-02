/*
 * cmdlineparser.cpp
 *
 *  Created on: Mar 24, 2015
 *      Author: masha
 */

#include <xtomic/aux/cppbasics.hpp>

#include "cmdlineparser.hpp"
#include "testfilter.hpp"
#include "testlocator.hpp"
#include "wildcard.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <cassert>
#include <cstring>

namespace xtomic
{
namespace perftest
{

namespace
{

typedef std::set<std::string> strs_type;

template<typename Accessor>
static strs_type get_all(Accessor acc)
{
    PerfTestLocator::size_type size = PerfTestLocator::getSize();
    strs_type result;
    for (id_type id = 0; id < size; ++id)
    {
        result.insert(acc(id));
    }
    return result;
}

struct getAllNames
{
    strs_type operator()() const
    {
        PerfTestLocator::get_test_name get_name;
        return get_all(get_name);
    }
};

struct getAllGroups
{
    strs_type operator()() const
    {
        PerfTestLocator::get_test_group get_group;
        return get_all(get_group);
    }
};

struct getAllFulls
{
    strs_type operator()() const
    {
        PerfTestLocator::get_test_full get_full;
        return get_all(get_full);
    }
};

template<typename GetAll, bool byWildCard>
class Validator
{
private:
    typedef typename strs_type::const_iterator iterator;
    typedef typename get_str_match<byWildCard>::type match_type;

public:
    Validator(const char* msg) :
            m_getter(),
            m_msg(msg)
    {

    }
    void operator()(const char* arg, const strs_type & coll) const
    {
        strs_type all = m_getter();
        iterator beg = coll.begin();
        iterator end = coll.end();
        iterator begOfAll = all.begin();
        iterator endOfAll = all.end();

        for (iterator j = beg; j != end; ++j)
        {
            const std::string & filter = *j;
            bool found = false;
            for (iterator i = begOfAll; !found && i != endOfAll; ++i)
            {
                const std::string & test = *i;
                found = found || match_type(test)(filter);
            }
            if (!found)
            {
                std::cerr << m_msg << " \"" << filter << "\" specified in: "
                        << arg << std::endl;
            }
        }
    }
private:
    GetAll m_getter;
    const char* m_msg;
}
;

}

static std::pair<std::string, const char*> parseParam(const char* arg)
{
    typedef std::pair<std::string, const char*> value_type;
    const char* pos = arg;
    while (*pos && *pos != '=')
    {
        ++pos;
    }
    const char* next = *pos ? pos + 1 : pos;
    return value_type(std::string(arg, pos), next);
}

static const char* s_sep = ",;";

static const char* skip(const char* s)
{
    while (*s && strchr(s_sep, *s) != nullptr)
        ++s;
    return s;
}

static void parseList(const char* val,
                      std::set<std::string> & res,
                      bool& invert)
{
    const char* pos = skip(val);
    if (*pos == '-')
    {
        invert = true;
        ++pos;
    }
    else
    {
        invert = false;
    }
    while (*pos)
    {
        const char* next = pos + 1;
        while (strchr(s_sep, *next) == nullptr)
            ++next;
        res.insert(std::string(pos, next));
        pos = skip(next);
    }
}

int CommandLineParser::m_duration = 10;

CommandLineParser::Command CommandLineParser::parse(const int argc,
                                                    const char** argv,
                                                    ids_type & tests)
{
    int i = 1;
    Command cmd = cmdError;

    // parse command
    for (; i < argc; ++i)
    {
        std::pair<std::string, const char*> pair = parseParam(argv[i]);
        if (pair.first == "--help")
        {
            cmd = cmdShowHelp;
            break;
        }
        else if (pair.first == "list-tests")
        {
            cmd = cmdListTests;
            break;
        }
        else if (pair.first == "run")
        {
            cmd = cmdRunTests;
            break;
        }
        else if (pair.first == "--duration")
        {
            char* end = nullptr;
            int val = strtol(pair.second, &end, 0);
            if (val <= 0 || end == pair.second || *end)
            {
                std::cerr << "invalid option: " << argv[i]
                        << ", positive integer number is expected" << std::endl;
                i = argc;
                break;
            }
            m_duration = val;
        }
        else
        {
            std::cerr << "invalid or unexpected parameter: " << argv[i]
                    << std::endl;
        }
    }
    // parse rest of command line
    switch (cmd)
    {
    case cmdError:
    case cmdShowHelp:
    case cmdListTests:
        break;
    case cmdRunTests:
        ++i;
        cmd = onRunTests(argc - i, argv + i, tests);
        break;
    default:
        assert(false);
    }
    return cmd;
}

void CommandLineParser::showHelp(const char* arg0)
{
    std::cout << "Usage:" << std::endl << arg0
            << " [--duration=<duration>] run "
            << "[--objects=[-]<objects>] [--groups=[-]<groups>] [--filter=[-]<filters>]"
            << std::endl << "or" << std::endl << arg0 << " list-tests"
            << std::endl << "or" << std::endl << arg0 << " --help" << std::endl
            << "where:" << std::endl << "    run - executes tests" << std::endl
            << "    <objects> - comma separated list of objects' names to test. Wild cards are supported."
            << std::endl
            << "    <groups> - comma separated list of objects' groups to run. Wild cards are supported."
            << std::endl
            << "    <filter> - comma separated list of fully qualified tests' names to run. Wild cards are supported."
            << std::endl
            << "    [-] - optional minus sign indicates that the filter is to be inverted"
            << std::endl << "    list-tests - display list of available tests"
            << std::endl << "    --help - show this message" << std::endl
            << "Note: fully qualified test's name consists of group, object and test separated by dot:"
            << std::endl << "      <group-name>.<object-name>.<test-name>"
            << std::endl;
}

template<typename Validator, typename Filter>
inline void processList(const Validator & validator,
                        const Filter & filter,
                        const char* argv,
                        const char* param,
                        ids_type & tests)
{
    bool invert = false;
    strs_type coll;
    parseList(param, coll, invert);
    validator(argv, coll);
    filter(tests, coll, invert);
}

CommandLineParser::Command CommandLineParser::onRunTests(const int argc,
                                                         const char** argv,
                                                         ids_type & tests)
{
    Validator<getAllNames, true> validateNames("unrecognized test name");
    Validator<getAllGroups, true> validateGroups("unrecognized group");
    Validator<getAllFulls, true> validateFilters(
            ("unrecognized fully qualified test name"));
    tests = Filter::getAllTests();

    std::string ereoneous;
    for (int i = 0; i < argc; ++i)
    {
        std::pair<std::string, const char*> pair = parseParam(argv[i]);
        if (pair.first == "--objects")
        {
            processList(validateNames, filter_by_name(), argv[i], pair.second,
                    tests);
        }
        else if (pair.first == "--groups")
        {
            processList(validateGroups, filter_by_group(), argv[i], pair.second,
                    tests);
        }
        else if (pair.first == "--filter")
        {
            processList(validateFilters, filter_by_full(), argv[i], pair.second,
                    tests);
        }
        else
        {
            std::cerr << "invalid or unexpected parameter: " << argv[i]
                    << std::endl;
            return cmdError;
        }
    }

    return cmdRunTests;
}

}
}
