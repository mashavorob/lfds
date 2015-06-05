/*
 * queuetest.hpp
 *
 *  Created on: Jun 2, 2015
 *      Author: masha
 */

#ifndef PERFTEST_QUEUES_QUEUETEST_HPP_
#define PERFTEST_QUEUES_QUEUETEST_HPP_

#include "timeutils.hpp"
#include "performancetest.hpp"
#include "cmdlineparser.hpp"

#include <pthread.h>
#include <math.h>

namespace xtomic
{
namespace perftest
{
namespace queues
{

static const unsigned int TYPICAL_SIZE = static_cast<unsigned int>(1000);

template<typename Queue, unsigned MaxSize = TYPICAL_SIZE>
class BandwithTester: public IPerformanceTest
{
private:
    typedef BandwithTester<Queue, MaxSize> this_type;

public:
    static const unsigned int maxSize = 1000;
    static const unsigned quietTime = 1;

    typedef Queue collection_type;

    typedef typename collection_type::value_type value_type;
    typedef typename collection_type::size_type size_type;

public:
    BandwithTester() :
            m_coll(maxSize*2),
            m_run(false),
            m_stop(false),
            m_popCount(0)
    {
    }
    // overrides
private:
    virtual double doTest()
    {
        m_run = false;
        m_stop = false;

        pthread_t poper = 0;
        pthread_t pusher = 0;
        void* arg = reinterpret_cast<void*>(this);

        pthread_create(&pusher, 0, &pushFunc, arg);
        pthread_create(&poper, 0, &popFunc, arg);

        timespec quiet =
        { quietTime, 0 };

        timespec runtime =
        { CommandLineParser::getDuration(), 0 };

        nanosleep(&quiet, nullptr);

        timespec start, end;

        clock_gettime(CLOCK_MONOTONIC, &start);

        m_run = true;
        nanosleep(&runtime, nullptr);

        m_stop = true;
        clock_gettime(CLOCK_MONOTONIC, &end);

        pthread_join(poper, 0);
        pthread_join(pusher, 0);

        timespec diff = end - start;
        double duration = seconds(diff);
        double count = static_cast<double>(m_popCount);
        double perf = count/duration/1.e6;
        return normalize(perf);
    }
private:
    static double normalize(const double val)
    {
        return (val < 100 ) ? normalize_small(val) : normalize_big(val);
    }
    static double normalize_big(const double val)
    {
        if ( val < 100 )
        {
            return round(val);
        }
        return normalize_big(val/10)*10;
    }
    static double normalize_small(const double val)
    {
        if ( val > 100 )
        {
            return round(val);
        }
        return normalize_small(val*10)/10;
    }
private:
    static void* popFunc(void* arg)
    {
        this_type* pThis = reinterpret_cast<this_type*>(arg);

        std::size_t popCount = 0;
        // wait for start
        while (!pThis->m_run)
            ;

        while (!pThis->m_stop)
        {
            value_type v;
            if (pThis->m_coll.pop(v))
            {
                ++popCount;
            }
        }
        pThis->m_popCount = popCount;
        return 0;
    }

    static void* pushFunc(void* arg)
    {
        this_type* pThis = reinterpret_cast<this_type*>(arg);

        // wait for start
        while (!pThis->m_run)
            ;

        value_type v = value_type();
        while (!pThis->m_stop)
        {
            if (pThis->m_coll.size() >= maxSize)
            {
                continue;
            }
            pThis->m_coll.push(v);
        }
        return 0;
    }
private:
    BandwithTester(const this_type&);
    this_type& operator=(const this_type&);

private:
    collection_type m_coll;
    volatile bool m_run;
    volatile bool m_stop;
    volatile std::size_t m_popCount;
};

}
}
}

#endif /* PERFTEST_QUEUES_QUEUETEST_HPP_ */
