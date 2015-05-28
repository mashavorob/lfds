/*
 * mttestaveragetime.hpp
 *
 *  Created on: Mar 25, 2015
 *      Author: masha
 */

#ifndef PERFTEST_TESTMULTITHREAD_HPP_
#define PERFTEST_TESTMULTITHREAD_HPP_

#include "performancetest.hpp"
#include <vector>
#include <utility>

namespace xtomic
{
namespace perftest
{

class IThreadTest
{
public:

    struct FLAGS
    {
        bool start;
        bool stop;
    };

    typedef std::size_t size_type;

    virtual ~IThreadTest()
    {

    }

    virtual void execute(const volatile FLAGS& flags,
                         size_type & count,
                         double & duration) = 0;
};

class ITestAggregator
{
public:
    typedef std::size_t size_type;

    virtual ~ITestAggregator()
    {
    }

    virtual void collect(size_type count, double duration) = 0;

    virtual double yield() const = 0;
};

class MultiThreadTest: public IPerformanceTest
{
public:

    static const unsigned quiet_time = 1;

    typedef std::size_t size_type;
    typedef IThreadTest::FLAGS flags_type;

    struct RESULT
    {
        size_type count;
        double duration;

        RESULT() :
                count(0),
                duration(0)
        {

        }
    };

    struct ARGS
    {
        const volatile flags_type * flags;
        IThreadTest * impl;
        RESULT results;
        ARGS(const volatile flags_type* f, IThreadTest* i) :
                flags(f),
                impl(i)
        {

        }
    };

    typedef std::pair<pthread_t, ARGS> thread_type;
    typedef std::vector<IThreadTest*> collection_type;
    typedef std::vector<thread_type> threads_type;
    typedef void *(*routine_type)(void *);

private:
    class Inserter
    {
    public:
        Inserter(const volatile flags_type & flags, threads_type & threads);
        void operator()(IThreadTest* impl);
    private:
        const volatile flags_type & m_flags;
        threads_type & m_threads;
    };

    struct Starter
    {
        Starter(routine_type routine) :
                m_routine(routine)
        {

        }
        void operator()(thread_type &thread)
        {
            pthread_create(&thread.first, nullptr, m_routine,
                    static_cast<void*>(&thread.second));
        }

    private:
        routine_type m_routine;
    };

    struct Joiner
    {
        void operator()(thread_type &thread)
        {
            pthread_join(thread.first, nullptr);
        }
    };

public:
    MultiThreadTest() :
            m_aggregator(nullptr),
            m_mult(1)
    {
    }

    void addThread(IThreadTest* impl)
    {
        m_threads.push_back(impl);
    }
    void setMult(const double mult)
    {
        m_mult = mult;
    }
    void setAggregator(ITestAggregator* agg)
    {
        m_aggregator = agg;
    }

public:
    // override
    double doTest();

private:
    static void* runner(void* args);

private:
    collection_type m_threads;
    ITestAggregator* m_aggregator;
    double m_mult;
};

}
}

#endif /* PERFTEST_TESTMULTITHREAD_HPP_ */
