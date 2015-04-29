/*
 * demo_hash_map.cpp
 *
 *  Created on: Feb 1, 2015
 *      Author: masha
 */

#include "data_adapter.hpp"
#include "map_wrapper.hpp"

#include <queue.hpp>
#include <hash_map.hpp>
#include <hash_trie.hpp>

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

static const unsigned MAP_TEST_SIZE = 40000;
static const unsigned MAP_TEST_NUM_THREADS = 2;
static const bool ALLOW_CONCURRENT_MODIFICATIONS = true;

template<typename Data>
struct get_random_data
{
    Data operator()() const;
};

template<>
struct get_random_data<std::pair<int, int> >
{
    get_random_data()
    {
        srand(time(nullptr));
    }
    std::pair<int, int> operator()() const
    {
        int k = rand();
        return std::make_pair(k, -k);
    }
};

template<typename Queue, typename Pred>
struct fill_queue
{
    typedef Queue queue_type;
    typedef typename queue_type::value_type value_type;
    typedef typename value_type::first_type key_type;
    typedef std::set<key_type> set_type;

    void operator()(queue_type & q)
    {
        value_type v;
        set_type set;
        bool res;
        do
        {
            v = m_pred();
            if (!set.insert(v.first).second)
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

template<typename Data, typename Map>
class BenchmarkBase
{
public:
    // wait free data queue to pass inserted data to deleter thread
    typedef lfds::queue<Data, lfds::Queue::FixedSize> queue_type;
    typedef Map map_type;
    typedef typename map_type::key_type key_type;
    typedef typename map_type::mapped_type mapped_type;
    typedef get_random_data<Data> randomizer_type;
    typedef fill_queue<queue_type, randomizer_type> filler_type;
    typedef std::size_t size_type;

    const static unsigned int SIZE_HIGH = MAP_TEST_SIZE;
    const static unsigned int SIZE_LOW = SIZE_HIGH*17/20; // (85%=85/100=17/20)

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

template<typename Data, typename Map>
class Inserter: public BenchmarkBase<Data, Map>, public result_type
{
public:
    typedef BenchmarkBase<Data, Map> base_type;
    typedef typename base_type::map_type map_type;
    typedef typename base_type::queue_type queue_type;
    typedef typename result_type::size_type size_type;

    static const unsigned SIZE_HIGH = base_type::SIZE_HIGH;

    void operator()(map_type & m)
    {
        queue_type & erased = base_type::getErasedQueue();
        queue_type & inserted = base_type::getInsertedQueue();
        size_type count = 0;

        while (!base_type::isStopped())
        {
            Data data;
            if (m.size() < SIZE_HIGH && erased.pop(data))
            {
                bool map_insert_res = m.insert(data.first, data.second);
                assert(map_insert_res);
                bool queue_insert_res = inserted.push(data);
                assert(queue_insert_res);
                ++count;
            }
        }
        m_result = count;
    }
};

template<typename Data, typename Map>
class Deleter: public BenchmarkBase<Data, Map>, public result_type
{
public:
    typedef BenchmarkBase<Data, Map> base_type;
    typedef typename base_type::map_type map_type;
    typedef typename base_type::queue_type queue_type;
    typedef typename result_type::size_type size_type;

    static const unsigned SIZE_LOW = base_type::SIZE_LOW;

    void operator()(map_type & m)
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
                bool map_erase_res = m.erase(data.first);
                assert(map_erase_res);
                bool queue_erase_res = erased.push(data);
                assert(queue_erase_res);
                ++count;
            }
        }
        m_result = count;
    }
};

template<typename Data, typename Map>
class Finder: public BenchmarkBase<Data, Map>, public result_type
{
public:
    typedef BenchmarkBase<Data, Map> base_type;
    typedef typename base_type::map_type map_type;
    typedef typename base_type::key_type key_type;
    typedef typename base_type::mapped_type mapped_type;
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
            m_keys.push_back(data.first);
        }
    }

    void operator()(map_type & m)
    {

        size_type count = 0;
        const_iterator beg = m_keys.begin();
        const_iterator end = m_keys.end();
        const_iterator iter = beg;
        mapped_type val;

        while (!base_type::isStarted())
            ;
        while (!base_type::isStopped())
        {
            const key_type & key = *iter;
            m.find(key, val);
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

template<typename Data, typename Map>
class Benchmark: public BenchmarkBase<Data, Map>
{
public:
    typedef BenchmarkBase<Data, Map> base_type;
    typedef typename base_type::size_type size_type;
    typedef typename base_type::map_type map_type;
    typedef Deleter<Data, Map> deleter_type;
    typedef Inserter<Data, Map> inserter_type;
    typedef Finder<Data, Map> finder_type;

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

        map_type m;

        std::vector<std::thread> threads;

        if ( ALLOW_CONCURRENT_MODIFICATIONS )
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

void DemoMap()
{
    typedef std::pair<int, int> data_type;
    typedef data_adapter<int> key_type;
    typedef data_adapter<int> mapped_type;

    typedef lfds::hash_map<int, int> lf_map_type_integral_pair;
    typedef lfds::hash_map<int, mapped_type> lf_map_type_integral_key;
    typedef lfds::hash_map<key_type, mapped_type> lf_map_type;
    typedef lfds::hash_trie<int, int> lf_trie_type;
    typedef std_map_wrapper<key_type, mapped_type> std_map_type;
    typedef std_unordered_map_wrapper<key_type, mapped_type> std_unordered_map_type;

    typedef Benchmark<data_type, lf_map_type_integral_pair> lf_map_benchmark_type_integral_pair;
    typedef Benchmark<data_type, lf_map_type_integral_key> lf_map_benchmark_type_integral_key;
    typedef Benchmark<data_type, lf_map_type> lf_map_benchmark_type;
    typedef Benchmark<data_type, lf_trie_type> lf_trie_benchmark_type;
    typedef Benchmark<data_type, std_map_type> std_map_benchmark_type;
    typedef Benchmark<data_type, std_unordered_map_type> std_unorderd_benchmark_type;

    const std::size_t num_threads = MAP_TEST_NUM_THREADS;

    lf_map_benchmark_type_integral_pair bm1_integral_pair;
    lf_map_benchmark_type_integral_key bm1_integral_key;
    lf_map_benchmark_type bm1;
    lf_trie_benchmark_type bmt;
    std_map_benchmark_type bm2;
    std_unorderd_benchmark_type bm3;


    std::cout << "benchmark lock free hash trie: ";
    std::cout.flush();
    double rt_generic = bmt(num_threads);
    std::cout << static_cast<int>(rt_generic * 1e9) << " ns per find" << std::endl;

    std::cout << "benchmark lock free hash map (integral key-value pair): ";
    std::cout.flush();
    double r1 = bm1_integral_pair(num_threads);
    std::cout << static_cast<int>(r1 * 1e9) << " ns per find" << std::endl;

    std::cout << "benchmark lock free hash map (integral key): ";
    std::cout.flush();
    double r1_integral_key = bm1_integral_key(num_threads);
    std::cout << static_cast<int>(r1_integral_key * 1e9) << " ns per find" << std::endl;

    std::cout << "benchmark lock free hash map (generic): ";
    std::cout.flush();
    double r1_generic = bm1(num_threads);
    std::cout << static_cast<int>(r1_generic * 1e9) << " ns per find" << std::endl;

    std::cout << "           benchmark std map: ";
    std::cout.flush();
    double r2 = bm2(num_threads);
    std::cout << static_cast<int>(r2 * 1e9) << " ns per find ("
            << static_cast<int>(r2 / r1) << " times slower)" << std::endl;

    std::cout << " benchmark std unordered_map: ";
    std::cout.flush();
    double r3 = bm3(num_threads);
    std::cout << static_cast<int>(r3 * 1e9) << " ns per find ("
            << static_cast<int>(r3 / r1) << " times slower)" << std::endl;
}
