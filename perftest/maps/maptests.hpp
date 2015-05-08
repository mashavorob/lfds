/*
 * averagetime.hpp
 *
 *  Created on: Mar 26, 2015
 *      Author: masha
 */

#ifndef PERFTEST_MAPS_MAPTESTS_HPP_
#define PERFTEST_MAPS_MAPTESTS_HPP_

#include <numbers.hpp>
#include <testmultithread.hpp>
#include <timeutils.hpp>
#include <testmem.hpp>
#include <vector>
#include <iostream>

namespace lfds
{
namespace perftest
{
namespace maps
{

static const unsigned int TYPICAL_SIZE = static_cast<unsigned int>(5e5);
static const unsigned int NUMBER_OF_REPETITIONS = static_cast<unsigned int>(30e6);

template<typename Map, bool InitialReserve, unsigned int Repetitions = TYPICAL_SIZE>
class AvgInsertTester
{
public:
    static const unsigned int count = Repetitions;

    typedef Map collection_type;
    typedef typename collection_type::key_type key_type;
    typedef typename collection_type::mapped_type mapped_type;
    typedef typename collection_type::size_type size_type;
    typedef random_generator<key_type> random_generator_type;
    typedef std::vector<key_type> vector_type;

public:
    AvgInsertTester() :
            m_coll(InitialReserve ? Repetitions + 32 : 0)
    {
        random_generator_type gen;
        gen(count, m_data);
    }

    void operator()()
    {
        typename vector_type::const_iterator i = m_data.begin();
        typename vector_type::const_iterator end = m_data.end();
        mapped_type val;
        for (; i != end; ++i)
        {
            m_coll.insert(*i, val);
        }
    }

private:
    vector_type m_data;
    collection_type m_coll;
};

template<typename Map, bool InitialReservation, unsigned int Repetitions =
        TYPICAL_SIZE>
class MaxInsertTester
{
public:
    static const unsigned int count = Repetitions;

    typedef Map collection_type;
    typedef typename collection_type::key_type key_type;
    typedef typename collection_type::mapped_type mapped_type;
    typedef random_generator<key_type> random_generator_type;
    typedef std::vector<key_type> vector_type;

public:
    MaxInsertTester() :
            m_val(),
            m_data(),
            m_coll(InitialReservation ? count : 0)
    {
        random_generator_type gen;
        gen(count, m_data);
        m_pos = m_data.begin();
    }

    void operator()()
    {
        m_coll.insert(*m_pos, m_val);
        ++m_pos;
    }

private:
    mapped_type m_val;
    vector_type m_data;
    collection_type m_coll;
    typename vector_type::const_iterator m_pos;
};

template<typename Map, unsigned int Repetitions = TYPICAL_SIZE>
class AvgEraseTester
{
public:
    static const unsigned int count = Repetitions;

    typedef Map collection_type;
    typedef typename collection_type::key_type key_type;
    typedef typename collection_type::mapped_type mapped_type;
    typedef random_generator<key_type> random_generator_type;
    typedef std::vector<key_type> vector_type;

public:
    AvgEraseTester() :
            m_coll(count)
    {
        random_generator_type gen;
        gen(count, m_data);

        typename vector_type::const_iterator i = m_data.begin();
        typename vector_type::const_iterator end = m_data.end();
        mapped_type val;
        for (; i != end; ++i)
        {
            m_coll.insert(*i, val);
        }

    }

    void operator()()
    {
        typename vector_type::const_iterator i = m_data.begin();
        typename vector_type::const_iterator end = m_data.end();
        for (; i != end; ++i)
        {
            m_coll.erase(*i);
        }
    }

private:
    vector_type m_data;
    collection_type m_coll;
};

template<typename Map, unsigned int Repetitions = NUMBER_OF_REPETITIONS, unsigned int Size = TYPICAL_SIZE>
class AvgFindTester
{
public:
    static const unsigned int count = Repetitions;
    static const unsigned int size = Size;

    typedef Map collection_type;
    typedef typename collection_type::key_type key_type;
    typedef typename collection_type::mapped_type mapped_type;
    typedef random_generator<key_type> random_generator_type;
    typedef std::vector<key_type> vector_type;

public:
    AvgFindTester() :
            m_coll(size)
    {
        random_generator_type gen;
        gen(size, m_data);

        typename vector_type::const_iterator i = m_data.begin();
        typename vector_type::const_iterator end = m_data.end();
        mapped_type val;
        for (; i != end; ++i)
        {
            m_coll.insert(*i, val);
        }

    }

    void operator()() const
    {
        typename vector_type::const_iterator beg = m_data.begin();
        typename vector_type::const_iterator end = m_data.end();
        typename vector_type::const_iterator i = beg;
        mapped_type val;
        unsigned int c = count;
        while ( --c )
        {
            if (++i == end)
            {
                i = beg;
            }
            const key_type key = *i;
            bool res = m_coll.find(key, val);
            if (!res)
            {
                std::cerr << "This code is normally unreachable, "
                        << "its only purpose is to make a reference to " << val
                        << std::endl;
            }
        }
    }

private:
    vector_type m_data;
    collection_type m_coll;
};

template<typename Map>
class MtInsertNoiser: public IThreadTest
{
public:
    typedef Map collection_type;
    typedef typename collection_type::key_type key_type;
    typedef typename collection_type::mapped_type mapped_type;
    typedef random_generator<key_type> random_generator_type;

    typedef typename IThreadTest::size_type size_type;
    typedef typename IThreadTest::FLAGS FLAGS;

    static constexpr int QUIET_PERIOD_NS = 1000000;
    static constexpr int NOISE_PACK = 1;

public:
    MtInsertNoiser(collection_type& coll) :
            m_coll(coll)
    {

    }

    void execute(const volatile FLAGS& flags,
                 size_type & count,
                 double & duration)
    {
        timespec time_to_sleep =
        { 0, QUIET_PERIOD_NS };
        timespec buff;

        mapped_type val;

        while (!flags.start)
            ;

        while (!flags.stop)
        {
            for (int i = 0; i < NOISE_PACK; ++i)
            {
                m_coll.insert(m_gen(), val);
            }
            nanosleep(&time_to_sleep, &buff);
        }
        count = 0;
        duration = 0;
    }

private:
    collection_type& m_coll;
    random_generator_type m_gen;
};

template<typename Map>
class MtAvgFindWorker: public IThreadTest
{
public:
    typedef Map collection_type;
    typedef typename collection_type::key_type key_type;
    typedef typename collection_type::mapped_type mapped_type;
    typedef random_generator<key_type> random_generator_type;
    typedef std::vector<key_type> vector_type;

    typedef typename IThreadTest::size_type size_type;
    typedef typename IThreadTest::FLAGS FLAGS;

    static constexpr unsigned SAMPLE_SIZE = 10000;

public:
    MtAvgFindWorker(collection_type& coll) :
            m_coll(coll)
    {
        random_generator_type gen;
        gen(SAMPLE_SIZE, m_sample);
    }

    void initSample(const vector_type & data)
    {
        size_type size = SAMPLE_SIZE;
        if (size > data.size())
        {
            size = data.size();
        }

        typename vector_type::const_iterator beg = data.begin();
        typename vector_type::const_iterator end = beg + size;
        vector_type sample(beg, end);
        m_sample.swap(sample);
    }

    void execute(const volatile FLAGS& flags,
                 size_type & count,
                 double & duration)
    {
        timespec startpoint;
        timespec endpoint;

        mapped_type val;

        typename vector_type::const_iterator begin = m_sample.begin();
        typename vector_type::const_iterator end = m_sample.end();
        typename vector_type::const_iterator i = begin;

        size_type c = 0;

        while (!flags.start)
            ;

        clock_gettime(CLOCK_MONOTONIC, &startpoint);
        while (!flags.stop)
        {
            const key_type &key = *i;
            if (++i == end)
            {
                i = begin;
            }
            bool res = m_coll.find(key, val);
            if (!res)
            {
                std::cerr << "This code is normally unreachable, "
                        << "its only purpose is to make a reference to " << val
                        << std::endl;
            }
            ++c;
        }
        clock_gettime(CLOCK_MONOTONIC, &endpoint);

        timespec diff = endpoint - startpoint;

        count = c;
        duration = seconds(diff);
    }

private:
    collection_type& m_coll;
    vector_type m_sample;
};

template<typename Map>
class MtMaxFindWorker: public IThreadTest
{
public:
    typedef Map collection_type;
    typedef typename collection_type::key_type key_type;
    typedef typename collection_type::mapped_type mapped_type;
    typedef random_generator<key_type> random_generator_type;
    typedef std::vector<key_type> vector_type;

    typedef typename IThreadTest::size_type size_type;
    typedef typename IThreadTest::FLAGS FLAGS;

    static constexpr unsigned SAMPLE_SIZE = 10000;

public:
    MtMaxFindWorker(collection_type& coll) :
            m_coll(coll)
    {
        random_generator_type gen;
        gen(SAMPLE_SIZE, m_sample);
    }

    void initSample(const vector_type & data)
    {
        size_type size = SAMPLE_SIZE;
        if (size > data.size())
        {
            size = data.size();
        }

        typename vector_type::const_iterator beg = data.begin();
        typename vector_type::const_iterator end = beg + size;
        vector_type sample(beg, end);
        m_sample.swap(sample);
    }

    void execute(const volatile FLAGS& flags,
                 size_type & count,
                 double & duration)
    {
        timespec startpoint;
        timespec endpoint;
        timespec max_diff =
        { 0, 0 };

        mapped_type val;

        typename vector_type::const_iterator begin = m_sample.begin();
        typename vector_type::const_iterator end = m_sample.end();
        typename vector_type::const_iterator i = begin;

        while (!flags.start)
            ;

        while (!flags.stop)
        {
            const key_type &key = *i;
            if (++i == end)
            {
                i = begin;
            }
            clock_gettime(CLOCK_MONOTONIC, &startpoint);
            bool res = m_coll.find(key, val);
            if (!res)
            {
                std::cerr << "This code is normally unreachable, "
                        << "its only purpose is to make a reference to " << val
                        << std::endl;
            }
            clock_gettime(CLOCK_MONOTONIC, &endpoint);
            timespec diff = endpoint - startpoint;
            if ((diff.tv_sec > max_diff.tv_sec)
                    || (diff.tv_sec == max_diff.tv_sec
                            && diff.tv_nsec > max_diff.tv_nsec))
            {
                max_diff = diff;
            }
        }

        count = 1;
        duration = seconds(max_diff);
    }

private:
    collection_type& m_coll;
    vector_type m_sample;
};

class MtAvgAggregator: public ITestAggregator
{
public:
    typedef ITestAggregator::size_type size_type;

    MtAvgAggregator() :
            m_count(0),
            m_duration(0)
    {

    }
    void collect(size_type count, double duration)
    {
        m_count += count;
        m_duration += duration;
    }

    double yield() const
    {
        return m_duration / static_cast<double>(m_count);
    }

private:
    size_type m_count;
    double m_duration;
};

class MtMaxAggregator: public ITestAggregator
{
public:
    typedef ITestAggregator::size_type size_type;

    MtMaxAggregator() :
            m_first(true),
            m_duration(0)
    {

    }
    void collect(size_type, double duration)
    {
        if (m_first || duration > m_duration)
        {
            m_duration = duration;
            m_first = false;
        }
    }

    double yield() const
    {
        return m_duration;
    }

private:
    bool m_first;
    double m_duration;
};

template<typename Map, unsigned int Size>
class MtTestBase
{
public:
    static const unsigned int SIZE = Size;

    typedef Map collection_type;
    typedef typename collection_type::key_type key_type;
    typedef typename collection_type::mapped_type mapped_type;
    typedef random_generator<key_type> random_generator_type;
    typedef std::vector<key_type> vector_type;

public:
    MtTestBase() :
            m_coll(Size)
    {
        random_generator_type gen;
        gen(SIZE, m_data);

        typename vector_type::const_iterator i = m_data.begin();
        typename vector_type::const_iterator end = m_data.end();

        mapped_type val;

        for (; i != end; ++i)
        {
            m_coll.insert(*i, val);
        }
    }

protected:
    collection_type m_coll;
    vector_type m_data;
};

template<typename Map, typename Noiser, typename Worker, typename Aggregator,
        int Multiplier, unsigned int Size>
class MtTestImpl: public MultiThreadTest, private MtTestBase<Map, Size>
{
public:
    typedef MtTestImpl<Map, Noiser, Worker, Aggregator, Multiplier, Size> this_type;
    typedef MtTestBase<Map, Size> base_type;
    typedef Noiser noiser_type;
    typedef Worker worker_type;
    typedef Aggregator aggregator_type;
    typedef typename base_type::collection_type collection_type;
    typedef typename base_type::key_type key_type;
    typedef typename base_type::mapped_type mapped_type;
    typedef typename base_type::vector_type vector_type;
public:
    MtTestImpl() :
            base_type(),
            m_noiser(base_type::m_coll),
            m_worker(base_type::m_coll),
            m_aggregator()
    {
        m_worker.initSample(base_type::m_data);
        MultiThreadTest::addThread(&m_noiser);
        MultiThreadTest::addThread(&m_worker);
        MultiThreadTest::setAggregator(&m_aggregator);
        MultiThreadTest::setMult(static_cast<double>(Multiplier));
    }
private:
    MtTestImpl(const this_type&); // = delete;
    this_type& operator=(const this_type&); // = delete;
private:
    noiser_type m_noiser;
    worker_type m_worker;
    aggregator_type m_aggregator;
};

}
}
}

#endif /* PERFTEST_MAPS_MAPTESTS_HPP_ */
