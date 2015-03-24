/*
 * cmdlineparser.hpp
 *
 *  Created on: Mar 24, 2015
 *      Author: masha
 */

#ifndef PERFTEST_CMDLINEPARSER_HPP_
#define PERFTEST_CMDLINEPARSER_HPP_

#include "testtypes.hpp"

namespace lfds
{
namespace perftest
{

class CommandLineParser
{
public:

    enum Command {
        cmdShowHelp,
        cmdListTests,
        cmdRunTests,
        cmdError
    };

    static Command parse(const int argc, const char** argv, ids_type & tests);

    static void showHelp(const char* arg0);

    static int getDuration() // seconds
    {
        return m_duration;
    }
private:
    static Command onRunTests(const int argc, const char** argv, ids_type & tests);

private:
    static int m_duration;
};

}
}
#endif /* PERFTEST_CMDLINEPARSER_HPP_ */
