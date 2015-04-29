/*
 * demo_set.cpp
 *
 *  Created on: Feb 16, 2015
 *      Author: masha
 */

#include "data_adapter.hpp"
#include "set_wrapper.hpp"

#include <queue.hpp>
#include <hash_set.hpp>

#include <atomic>
#include <cstdlib>
#include <ctime>
#include <mutex>
#include <cassert>
#include <vector>
#include <functional>
#include <chrono>
#include <thread>
#include <iostream>
#include <set>

static const unsigned SET_TEST_SIZE = 4000;
static const unsigned SET_TEST_NUM_THREADS = 2;
static const bool ALLOW_CONCURENT_MODIFICATIONS = true;

template<typename Data>
struct get_random_data
{
    Data operator()() const;
};

template<>
struct get_random_data<int>
{
    get_random_data()
    {
        srand(time(nullptr));
    }
    int operator()() const
    {
        int k = rand();
        return k;
    }
};

template<typename Queue, typename Pred>
struct fill_queue
{
    typedef Queue queue_type;
    typedef typename queue_type::value_type value_type;
    typedef value_type key_type;
    typedef std::set<key_type> set_type;

    void operator()(queue_type & q)
    {
        value_type v;
        set_type set;
        bool res;
        do
        {
            v = m_pred();
            if (!set.insert(v).second)
            {
                // unique keys only allowed
                continue;
            }
            res = q.push(v);
        } while (res);
    }
private:
    Pred m_pred;
};

template<typename Data, typename Set>
class BenchmarkBase
{
public:
    // wait free data queue to pass inserted data to deleter thread
    typedef lfds::queue<Data, lfds::Queue::FixedSize> queue_type;
    typedef Set set_type;
    typedef typename set_type::key_type key_type;
    typedef get_random_data<Data> randomizer_type;
    typedef fill_queue<queue_type, randomizer_type> filler_type;
    typedef std::size_t size_type;

    const static unsigned int SIZE_HIGH = SET_TEST_SIZE;
    const static unsigned int SIZE_LOW = SIZE_HIGH * 17 / 20; // (85%=85/100=17/20)

    static void reset()
    {
        getStartFlag().store(false, std::memory_order_relaxed);
        getStopFlag().store(false, std::memory_order_relaxed);
    }
    static void start()
    {
        getStartFlag().store(true, std::memory_order_release);
    }
    static void stop()
    {
        getStopFlag().store(true, std::memory_order_release);
    }
protected:
    static queue_type & getErasedQueue()
    {
        static queue_type q(SIZE_HIGH);
        static std::once_flag flag;
        static filler_type filler;

        std::call_once(flag, filler, std::ref(q));

        return q;
    }
    static queue_type & getInsertedQueue()
    {
        static queue_type q(SIZE_HIGH);
        return q;
    }
    static bool isStarted()
    {
        return getStartFlag().load(std::memory_order_relaxed);
    }
    static bool isStopped()
    {
        return getStopFlag().load(std::memory_order_relaxed);
    }
private:
    static std::atomic_bool & getStartFlag()
    {
        static std::atomic_bool val(0);
        return val;
    }
    static std::atomic_bool & getStopFlag()
    {
        static std::atomic_bool val(0);
        return val;
    }
};

struct result_type
{
    typedef std::size_t size_type;

    size_type m_result;
};

template<typename Data, typename Set>
class Inserter: public BenchmarkBase<Data, Set>, public result_type
{
public:
    typedef BenchmarkBase<Data, Set> base_type;
    typedef typename base_type::set_type set_type;
    typedef typename base_type::queue_type queue_type;
    typedef typename result_type::size_type size_type;

    static const unsigned SIZE_HIGH = base_type::SIZE_HIGH;

    void operator()(set_type & m)
    {
        queue_type & erased = base_type::getErasedQueue();
        queue_type & inserted = base_type::getInsertedQueue();
        size_type count = 0;

        while (!base_type::isStopped())
        {
            Data data;
            if (m.size() < SIZE_HIGH && erased.pop(data))
            {
                bool set_insert_res = m.insert(data);
                assert(set_insert_res);
                bool queue_insert_res = inserted.push(data);
                assert(queue_insert_res);
                ++count;
            }
        }
        m_result = count;
    }
};

template<typename Data, typename Set>
class Deleter: public BenchmarkBase<Data, Set>, public result_type
{
public:
    typedef BenchmarkBase<Data, Set> base_type;
    typedef typename base_type::set_type set_type;
    typedef typename base_type::queue_type queue_type;
    typedef typename result_type::size_type size_type;

    static const unsigned SIZE_LOW = base_type::SIZE_LOW;

    void operator()(set_type & m)
    {
        queue_type & erased = base_type::getErasedQueue();
        queue_type & inserted = base_type::getInsertedQueue();
        size_type count = 0;

        while (!base_type::isStarted())
            ;
        while (!base_type::isStopped())
        {
            Data data;
            if (m.size() > SIZE_LOW && inserted.pop(data))
            {
                bool set_erase_res = m.erase(data);
                assert(set_erase_res);
                bool queue_erase_res = erased.push(data);
                assert(queue_erase_res);
                ++count;
            }
        }
        m_result = count;
    }
};

template<typename Data, typename Set>
class Finder: public BenchmarkBase<Data, Set>, public result_type
{
public:
    typedef BenchmarkBase<Data, Set> base_type;
    typedef typename base_type::set_type set_type;
    typedef typename base_type::key_type key_type;
    typedef typename base_type::randomizer_type randomizer_type;
    typedef std::vector<key_type> collection_type;
    typedef typename collection_type::const_iterator const_iterator;
    typedef typename result_type::size_type size_type;

    static const size_type SIZE = 10000;
public:

    Finder()
    {
        randomizer_type randomizer;

        for (size_type i = 0; i < SIZE; ++i)
        {
            Data data = randomizer();
            m_keys.push_back(data);
        }
    }

    void operator()(set_type & m)
    {

        size_type count = 0;
        const_iterator beg = m_keys.begin();
        const_iterator end = m_keys.end();
        const_iterator iter = beg;

        while (!base_type::isStarted())
            ;
        while (!base_type::isStopped())
        {
            const key_type & key = *iter;
            m.find(key);
            ++count;
            if (++iter == end)
            {
                iter = beg;
            }
        }
        m_result = count;
    }
private:
    collection_type m_keys;
};

template<typename Data, typename Set>
class Benchmark: public BenchmarkBase<Data, Set>
{
public:
    typedef BenchmarkBase<Data, Set> base_type;
    typedef typename base_type::size_type size_type;
    typedef typename base_type::set_type set_type;
    typedef Deleter<Data, Set> deleter_type;
    typedef Inserter<Data, Set> inserter_type;
    typedef Finder<Data, Set> finder_type;

    static const int DURATION_SECONDS = 10;

public:
    double operator()(size_type thread_count) const
    {
        typedef std::chrono::duration<double> seconds_type;
        typedef std::chrono::high_resolution_clock clock_type;
        typedef clock_type::duration duration_type;
        typedef clock_type::time_point time_point_type;

        deleter_type deleter;
        inserter_type inserter;
        std::vector<finder_type> finders(thread_count, finder_type());

        base_type::reset();

        set_type m;

        std::vector<std::thread> threads;

        if (ALLOW_CONCURENT_MODIFICATIONS)
        {
            threads.push_back(std::thread(std::ref(deleter), std::ref(m)));
        }
        threads.push_back(std::thread(std::ref(inserter), std::ref(m)));

        for (finder_type & finder : finders)
        {
            threads.push_back(std::thread(std::ref(finder), std::ref(m)));
        }

        const seconds_type duration(DURATION_SECONDS);

        const seconds_type quiet(1); // quiet time 1 sec just for fun
        std::this_thread::sleep_for(quiet);

        base_type::start();

        time_point_type start_point = clock_type::now();
        std::this_thread::sleep_for(duration);
        base_type::stop();
        time_point_type end_point = clock_type::now();

        // stop all threads
        for (auto & t : threads)
        {
            t.join();
        }

        size_type count = 0;
        for (finder_type & finder : finders)
        {
            count += finder.m_result;
        }

        // return average time
        seconds_type duration_seconds =
                std::chrono::duration_cast<seconds_type>(
                        end_point - start_point);

        double res = static_cast<double>(duration_seconds.count())
                / static_cast<double>(count);
        res = res / static_cast<double>(thread_count);
        return res;
    }
};

void DemoSet()
{
    typedef int data_type;
    typedef data_adapter<data_type> key_type;

    typedef lfds::hash_set<int> lf_set_type_integral_key;
    typedef lfds::hash_set<key_type> lf_set_type;
    typedef std_set_wrapper<key_type> std_set_type;
    typedef std_unordered_set_wrapper<key_type> std_unordered_set_type;

    typedef Benchmark<data_type, lf_set_type_integral_key> lf_set_benchmark_type_integral_key;
    typedef Benchmark<data_type, lf_set_type> lf_set_benchmark_type;
    typedef Benchmark<data_type, std_set_type> std_set_benchmark_type;
    typedef Benchmark<data_type, std_unordered_set_type> std_unorderd_benchmark_type;

    const std::size_t num_threads = SET_TEST_NUM_THREADS;

    lf_set_benchmark_type_integral_key bm1_integral_key;
    lf_set_benchmark_type bm1;
    std_set_benchmark_type bm2;
    std_unorderd_benchmark_type bm3;

    std::cout << "benchmark lock free hash set (integral key): ";
    std::cout.flush();
    double r1 = bm1_integral_key(num_threads);
    std::cout << static_cast<int>(r1 * 1e9) << " ns per find" << std::endl;

    std::cout << "benchmark lock free hash set (generic): ";
    std::cout.flush();
    double r1_generic = bm1(num_threads);
    std::cout << static_cast<int>(r1_generic * 1e9) << " ns per find"
            << std::endl;

    std::cout << "           benchmark std set: ";
    std::cout.flush();
    double r2 = bm2(num_threads);
    std::cout << static_cast<int>(r2 * 1e9) << " ns per find ("
            << static_cast<int>(r2 / r1) << " times slower)" << std::endl;

    std::cout << " benchmark std unordered_set: ";
    std::cout.flush();
    double r3 = bm3(num_threads);
    std::cout << static_cast<int>(r3 * 1e9) << " ns per find ("
            << static_cast<int>(r3 / r1) << " times slower)" << std::endl;
}

