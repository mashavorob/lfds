/*
 * testmtaveragetime.cpp
 *
 *  Created on: Mar 25, 2015
 *      Author: masha
 */

#include "cmdlineparser.hpp"

#include <algorithm>
#include <ctime>
#include "testmultithread.hpp"

namespace xtomic
{
namespace perftest
{

MultiThreadTest::Inserter::Inserter(const volatile flags_type & flags,
                                    threads_type & threads) :
        m_flags(flags),
        m_threads(threads)
{

}

void MultiThreadTest::Inserter::operator()(IThreadTest* impl)
{
    ARGS args(&m_flags, impl);
    m_threads.push_back(std::make_pair(pthread_t(), args));
}

double MultiThreadTest::doTest()
{
    volatile flags_type flags =
    { false, false };

    threads_type threads;

    std::for_each(m_threads.begin(), m_threads.end(), Inserter(flags, threads));
    std::for_each(threads.begin(), threads.end(), Starter(&runner));

    timespec quiet =
    { quiet_time, 0 };
    timespec runtime =
    { CommandLineParser::getDuration(), 0 };
    nanosleep(&quiet, nullptr);

    flags.start = true;
    nanosleep(&runtime, nullptr);
    flags.stop = true;
    std::for_each(threads.begin(), threads.end(), Joiner());

    threads_type::const_iterator beg = threads.begin();
    threads_type::const_iterator end = threads.end();

    for (threads_type::const_iterator i = beg; i != end; ++i)
    {
        const RESULT & res = i->second.results;
        m_aggregator->collect(res.count, res.duration);
    }

    const double perf = m_aggregator->yield() * m_mult;
    return perf;
}

void* MultiThreadTest::runner(void* raw_args)
{
    ARGS* args = reinterpret_cast<ARGS*>(raw_args);

    args->impl->execute(*args->flags, args->results.count,
            args->results.duration);
    pthread_exit(nullptr);
    return nullptr;
}

}
}

