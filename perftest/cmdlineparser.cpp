/*
 * cmdlineparser.cpp
 *
 *  Created on: Mar 24, 2015
 *      Author: masha
 */

#include "cmdlineparser.hpp"
#include "testfilter.hpp"
#include "testlocator.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <cassert>
#include <cstring>

namespace lfds
{
namespace perftest
{

namespace
{

typedef std::set<std::string> strs_type;

template<class Accessor>
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

struct getAllLabels
{
    strs_type operator()() const
    {
        const PerfTestLocator & locator = PerfTestLocator::getInstance();
        PerfTestLocator::size_type size = PerfTestLocator::getSize();
        strs_type result;
        for (id_type id = 0; id < size; ++id)
        {
            const char** labels = locator.getTestLabels(id);
            for (int i = 0; labels[i]; ++i)
            {
                result.insert(labels[i]);
            }
        }
        return result;
    }
};

template<typename GetAll>
class Validator
{
private:
    typedef typename strs_type::const_iterator iterator;

public:
    Validator(GetAll getter = GetAll()) :
            m_getter(),
            m_all(),
            m_initialized(false)
    {

    }
    void operator()(const char* arg, const char* msg, const strs_type & coll)
    {
        if ( !m_initialized )
        {
            m_all = m_getter();
            m_initialized = true;
        }
        iterator beg = coll.begin();
        iterator end = coll.end();
        iterator endOfAll = m_all.end();

        for ( iterator i = beg; i != end; ++i )
        {
            const std::string & s = *i;
            iterator pos = m_all.find(s);
            if ( pos == endOfAll )
            {
                std::cerr << msg << s << " in: " << arg << std::endl;
            }
        }
    }
private:
    GetAll m_getter;
    strs_type m_all;
    bool m_initialized;
};

}

static std::pair<std::string, const char*> parseParam(const char* arg)
{
    const char* pos = arg;
    while (*pos && *pos != '=')
        ++pos;
    const char* next = *pos ? pos + 1 : pos;
    return std::make_pair(std::string(arg, pos), next);
}

static const char* s_sep = " \t,\'\"";

static const char* skip(const char* s)
{
    while (*s && strchr(s_sep, *s) != nullptr)
        ++s;
    return s;
}

static void parseList(const char* val, std::set<std::string> & res)
{
    const char* pos = skip(val);
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
            << "[--tests=<list>] [--groups=<groups>] [--labels=<labels>]"
            << std::endl << "or" << std::endl << arg0 << " list-tests"
            << std::endl << "or" << std::endl << arg0 << " --help" << std::endl
            << "where:" << std::endl << "    run - executes tests" << std::endl
            << "    <list> - comma separated list of tests\' names to run"
            << std::endl
            << "    <groups> - comma separated list of tests\' groups to run"
            << std::endl
            << "    <labels> - comma separated list of tests\' labels to run"
            << std::endl << "    list-tests - display list of available tests"
            << std::endl << "    --help - show this message" << std::endl;
}

CommandLineParser::Command CommandLineParser::onRunTests(const int argc,
                                                         const char** argv,
                                                         ids_type & tests)
{
    Validator<getAllNames> validateNames;
    Validator<getAllGroups> validateGroups;
    Validator<getAllLabels> validateLabels;
    strs_type names;
    strs_type groups;
    std::vector<strs_type> labels;
    std::string ereoneous;
    for (int i = 0; i < argc; ++i)
    {
        std::pair<std::string, const char*> pair = parseParam(argv[i]);
        if (pair.first == "--tests")
        {
            parseList(pair.second, names);
            validateNames(argv[i], "unrecognized test name ", names);
        }
        else if (pair.first == "--groups")
        {
            parseList(pair.second, groups);
            validateGroups(argv[i], "unrecognized group ", groups);
        }
        else if (pair.first == "--labels")
        {
            strs_type buff;
            parseList(pair.second, buff);
            validateLabels(argv[i], "unrecognized label ", buff);
            labels.push_back(buff);
        }
        else
        {
            std::cerr << "invalid or unexpected parameter: " << argv[i]
                    << std::endl;
            return cmdError;
        }
    }

    tests = Filter::getAllTests();
    Filter::byName(tests, names);
    Filter::byGroup(tests, groups);
    std::vector<strs_type>::const_iterator i = labels.begin();
    std::vector<strs_type>::const_iterator end = labels.end();

    for ( ; i != end; ++i )
    {
        const strs_type & buff = *i;
        Filter::byLabels(tests, buff);
    }

    return cmdRunTests;
}

}
}
