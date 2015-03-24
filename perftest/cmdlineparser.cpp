/*
 * cmdlineparser.cpp
 *
 *  Created on: Mar 24, 2015
 *      Author: masha
 */

#include "cmdlineparser.hpp"
#include "testfilter.hpp"

#include <iostream>
#include <string>
#include <utility>
#include <cassert>
#include <cstring>

namespace lfds
{
namespace perftest
{

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
    while ( *pos )
    {
        const char* next = pos + 1;
        while (strchr(s_sep, *next) == nullptr) ++next;
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
    for ( ; i < argc; ++i )
    {
        std::pair<std::string, const char*> pair = parseParam(argv[i]);
        if ( pair.first == "--help" )
        {
            cmd = cmdShowHelp;
            break;
        }
        else if ( pair.first == "list-tests" )
        {
            cmd = cmdListTests;
            break;
        }
        else if ( pair.first == "run" )
        {
            cmd = cmdRunTests;
            break;
        }
        else if ( pair.first == "--duration" )
        {
            char* end = nullptr;
            int val = strtol(pair.second, &end, 0);
            if ( val <= 0 || end == pair.second || *end )
            {
                std::cerr << "invalid option: " << argv[i] << ", positive integer number is expected"
                        << std::endl;
                i = argc;
                break;
            }
            m_duration = val;
        }
        else
        {
            std::cerr << "invalid or unexpected parameter: " << argv[i] << std::endl;
        }
    }
    // parse rest of command line
    switch ( cmd )
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

CommandLineParser::Command CommandLineParser::onRunTests(const int argc, const char** argv, ids_type & tests)
{
    std::set<std::string> names;
    std::set<std::string> groups;
    std::set<std::string> labels;
    for ( int i = 0; i < argc; ++i )
    {
        std::pair<std::string, const char*> pair = parseParam(argv[i]);
        if ( pair.first == "--tests" )
        {
            parseList(pair.second, names);
        }
        else if ( pair.first == "--groups" )
        {
            parseList(pair.second, groups);
        }
        else if ( pair.first == "--labels" )
        {
            parseList(pair.second, labels);
        }
        else
        {
            std::cerr << "invalid or unexpected parameter: " << argv[i] << std::endl;
            return cmdError;
        }
    }

    tests = Filter::getAllTests();
    Filter::byName(tests, names);
    Filter::byGroup(tests, groups);
    Filter::byLabels(tests, labels);
    return cmdRunTests;
}

}
}
